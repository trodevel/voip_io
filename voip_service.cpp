/*

Simple VOIP interface for Skypeio library

Copyright (C) 2014 Sergey Kolevatov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

// $Revision: 1808 $ $Date:: 2015-06-02 #$ $Author: serge $


#include "voip_service.h"           // self

#include "../skype_service/skype_service.h" // SkypeService
#include "../skype_service/event_parser.h"  // EventParser
#include "../skype_service/str_helper.h"    // StrHelper

#include "../utils/dummy_logger.h"      // dummy_log
#include "../utils/mutex_helper.h"      // MUTEX_SCOPE_LOCK
#include "../utils/assert.h"            // ASSERT

#include "object_factory.h"             // create_message_t


#define MODULENAME      "VoipService"

NAMESPACE_VOIP_SERVICE_START

VoipService::VoipService():
        ServerBase( this ),
        sio_( nullptr ),
        callback_( nullptr ),
        state_( UNDEFINED ),
        req_state_( NONE ),
        req_hash_id_( 0 ),
        cs_( skype_service::conn_status_e::NONE ),
        us_( skype_service::user_status_e::NONE ),
        failure_reason_( 0 ),
        pstn_status_( 0 ),
        next_id_( 0 )
{
}

VoipService::~VoipService()
{
}

bool VoipService::init( skype_service::SkypeService * sw )
{
    MUTEX_SCOPE_LOCK( mutex_ );

    if( sw == 0L )
        return false;

    sio_ =   sw;

    return true;
}

// interface IVoipService
void VoipService::consume( const VoipioObject * req )
{
    ServerBase::consume( req );
}

// interface skype_service::ISkypeCallback
void VoipService::consume( const skype_service::Event * e )
{
    VoipioObjectWrap * ew = new VoipioObjectWrap;

    ew->ptr = e;

    ServerBase::consume( ew );
}


void VoipService::handle( const servt::IObject* req )
{
    MUTEX_SCOPE_LOCK( mutex_ );

    if( typeid( *req ) == typeid( VoipioInitiateCall ) )
    {
        handle( dynamic_cast< const VoipioInitiateCall *>( req ) );
    }
    else if( typeid( *req ) == typeid( VoipioPlayFile ) )
    {
        handle( dynamic_cast< const VoipioPlayFile *>( req ) );
    }
    else if( typeid( *req ) == typeid( VoipioRecordFile ) )
    {
        handle( dynamic_cast< const VoipioRecordFile *>( req ) );
    }
    else if( typeid( *req ) == typeid( VoipioDrop ) )
    {
        handle( dynamic_cast< const VoipioDrop *>( req ) );
    }
    else if( typeid( *req ) == typeid( VoipioObjectWrap ) )
    {
        handle( dynamic_cast< const VoipioObjectWrap *>( req ) );
    }
    else
    {
        dummy_log_fatal( MODULENAME, "handle: cannot cast request to known type - %p", (void *) req );

        ASSERT( 0 );
    }

    delete req;
}

void VoipService::handle( const VoipioInitiateCall * req )
{
    // private: no mutex lock

    if( state_ != READY )
    {
        send_reject_response( 0, "not ready" );
        return;
    }

    ASSERT( req_state_ == NONE );

    uint32_t hash_id    = get_next_id();

    bool b = sio_->call( req->party, hash_id );

    if( b == false )
    {
        dummy_log_error( MODULENAME, "failed calling: %s", req->party.c_str() );

        callback_consume( create_error_response( 0, "skype io failed" ) );

        return;
    }

    req_state_      = WAIT_INIT_CALL_RESP;
    req_hash_id_    = hash_id;
}

void VoipService::handle( const VoipioDrop * req )
{
    // private: no mutex lock

    if( state_ != READY )
    {
        send_reject_response( 0, "not ready" );
        return;
    }

    ASSERT( req_state_ == NONE );

    uint32_t hash_id    = get_next_id();

    bool b = sio_->set_call_status( req->call_id, skype_service::call_status_e::FINISHED, hash_id );

    if( b == false )
    {
        callback_consume( create_error_response( 0, "skype io failed" ) );
        return;
    }

    req_state_      = WAIT_DROP_RESP;
    req_hash_id_    = hash_id;
}

void VoipService::handle( const VoipioPlayFile * req )
{
    // private: no mutex lock

    if( state_ != READY )
    {
        send_reject_response( 0, "not ready" );
        return;
    }

    ASSERT( req_state_ == NONE );

    uint32_t hash_id    = get_next_id();

    bool b = sio_->alter_call_set_input_file( req->call_id, req->filename, hash_id );

    if( b == false )
    {
        dummy_log_error( MODULENAME, "failed setting input file: %s", req->filename.c_str() );

        callback_consume( create_call_error_response( req->call_id, 0, "failed setting input file: " + req->filename ) );

        return;
    }

    req_state_      = WAIT_PLAY_RESP;
    req_hash_id_    = hash_id;
}

void VoipService::handle( const VoipioRecordFile * req )
{
    // private: no mutex lock

    if( state_ != READY )
    {
        send_reject_response( 0, "not ready" );
        return;
    }

    ASSERT( req_state_ == NONE );

    uint32_t hash_id    = get_next_id();

    bool b = sio_->alter_call_set_output_file( req->call_id, req->filename, hash_id );

    if( b == false )
    {
        dummy_log_error( MODULENAME, "failed setting output file: %s", req->filename.c_str() );

        callback_consume( create_call_error_response( req->call_id, 0, "failed output input file: " + req->filename ) );

        return;
    }

    req_state_      = WAIT_REC_RESP;
    req_hash_id_    = hash_id;
}

void VoipService::handle( const VoipioObjectWrap * req )
{
    // private: no mutex lock

    const skype_service::Event * ev = static_cast<const skype_service::Event*>( req->ptr );

    ASSERT( ev );

    switch( req_state_ )
    {
    case WAIT_INIT_CALL_RESP:
    case WAIT_DROP_RESP:
    case WAIT_PLAY_RESP:
    case WAIT_REC_RESP:
    {
        if( ev->has_hash_id() == false )
        {
            dummy_log_info( MODULENAME, "ignoring a non-response notification: %s", skype_service::StrHelper::to_string( ev->get_type() ).c_str() );

            delete ev;

            return;
        }

        if( ev->get_hash_id() != req_hash_id_ )
        {
            dummy_log_error( MODULENAME, "unexpected hash id: %u, expected %u, msg %s, ignoring",
                    ev->get_hash_id(), req_hash_id_,
                    skype_service::StrHelper::to_string( ev->get_type() ).c_str() );

            delete ev;

            return;
        }

    }
        break;
    default:
        break;
    }

    switch( req_state_ )
    {
    case WAIT_INIT_CALL_RESP:
        handle_in_state_w_ic( ev );
        break;
    case WAIT_DROP_RESP:
        handle_in_state_w_dr( ev );
        break;
    case WAIT_PLAY_RESP:
        handle_in_state_w_pl( ev );
        break;
    case WAIT_REC_RESP:
        handle_in_state_w_re( ev );
        break;
    default:
        handle_in_state_none( ev );
        break;
    }

    delete ev;
}

void VoipService::handle_in_state_none( const skype_service::Event * ev )
{
    // private: no mutex lock

    skype_service::Event::type_e id = ev->get_type();

    switch( id )
    {
    case skype_service::Event::CONNSTATUS:
        handle( static_cast<const skype_service::ConnStatusEvent*>( ev ) );
        break;

    case skype_service::Event::USERSTATUS:
        handle( static_cast<const skype_service::UserStatusEvent*>( ev ) );
        break;

    case skype_service::Event::CURRENTUSERHANDLE:
        handle( static_cast<const skype_service::CurrentUserHandleEvent*>( ev ) );
        break;

    case skype_service::Event::USER_ONLINE_STATUS:
        break;

    case skype_service::Event::CALL:
        break;
    case skype_service::Event::CALL_DURATION:
        handle( static_cast<const skype_service::CallDurationEvent*>( ev ) );
        break;

    case skype_service::Event::CALL_STATUS:
        handle( static_cast<const skype_service::CallStatusEvent*>( ev ) );
        break;

    case skype_service::Event::CALL_PSTN_STATUS:
        handle( static_cast<const skype_service::CallPstnStatusEvent*>( ev ) );
        break;

    case skype_service::Event::CALL_FAILUREREASON:
        handle( static_cast<const skype_service::CallFailureReasonEvent*>( ev ) );
        break;

    case skype_service::Event::CALL_VAA_INPUT_STATUS:
        handle( static_cast<const skype_service::CallVaaInputStatusEvent*>( ev ) );
        break;

    case skype_service::Event::ERROR:
        handle( static_cast<const skype_service::ErrorEvent*>( ev ) );
        break;

    case skype_service::Event::UNDEF:
        break;

    case skype_service::Event::CHAT:
    case skype_service::Event::CHATMEMBER:
        // simply ignore
        break;

    case skype_service::Event::UNKNOWN:
    default:
        on_unknown( "???" );
        break;
    }
}

void VoipService::handle_in_state_w_ic( const skype_service::Event * ev )
{
    // private: no mutex lock

    req_state_      = NONE;
    req_hash_id_    = 0;

    if( ev->get_type() != skype_service::Event::CALL_STATUS )
    {
        if( ev->get_type() == skype_service::Event::ERROR )
        {
            uint32 errorcode    = static_cast<const skype_service::ErrorEvent*>( ev )->get_par_int();
            std::string descr   = static_cast<const skype_service::ErrorEvent*>( ev )->get_par_str();

            dummy_log_error( MODULENAME, "error occurred: %u '%s'", errorcode, descr.c_str() );

            callback_consume( create_error_response( errorcode, descr ) );
        }
        else
        {
            dummy_log_error( MODULENAME, "unexpected response: %s", skype_service::StrHelper::to_string( ev->get_type() ).c_str() );

            callback_consume( create_error_response( 0, "unexpected response: " + skype_service::StrHelper::to_string( ev->get_type() ) ) );
        }
        return;
    }

    uint32 call_id = static_cast<const skype_service::BasicCallEvent*>( ev )->get_call_id();

    skype_service::call_status_e status_code = static_cast<const skype_service::CallStatusEvent*>( ev )->get_call_s();
    uint32 status  = static_cast<uint32>( status_code );

    dummy_log_debug( MODULENAME, "call initiated: %u, status %s", call_id, skype_service::to_string( status_code ).c_str() );

    failure_reason_ = 0;
    pstn_status_    = 0;

    callback_consume( create_initiate_call_response( call_id, status ) );
}

void VoipService::handle_in_state_w_dr( const skype_service::Event * ev )
{
    // private: no mutex lock

    req_state_      = NONE;
    req_hash_id_    = 0;

    if( ev->get_type() == skype_service::Event::CALL_STATUS )
    {
        const skype_service::CallStatusEvent * cse = static_cast<const skype_service::CallStatusEvent *>( ev );
        if( cse->get_call_s() == skype_service::call_status_e::FINISHED )
        {
            callback_consume( create_message_t<VoipioDropResponse>( cse->get_call_id() ) );
        }
        else
        {
            dummy_log_debug( MODULENAME, "dropping, current state: %s", skype_service::to_string( cse->get_call_s() ).c_str() );
        }
        return;
    }

    dummy_log_error( MODULENAME, "unexpected response: %s", skype_service::StrHelper::to_string( ev->get_type() ).c_str() );

    callback_consume( create_error_response( 0, "unexpected response: " + skype_service::StrHelper::to_string( ev->get_type() ) ) );
}

void VoipService::handle_in_state_w_pl( const skype_service::Event * ev )
{
    // private: no mutex lock

    req_state_      = NONE;
    req_hash_id_    = 0;

    if( ev->get_type() != skype_service::Event::ALTER_CALL_SET_INPUT_FILE )
    {
        dummy_log_error( MODULENAME, "unexpected response: %s", skype_service::StrHelper::to_string( ev->get_type() ).c_str() );

        callback_consume( create_call_error_response(
                static_cast<const skype_service::BasicCallEvent*>( ev )->get_call_id(), 0,
                "unexpected response: " + skype_service::StrHelper::to_string( ev->get_type() ) ) );

        return;
    }
}

void VoipService::handle_in_state_w_re( const skype_service::Event * ev )
{
    // private: no mutex lock

    req_state_      = NONE;
    req_hash_id_    = 0;

    if( ev->get_type() != skype_service::Event::ALTER_CALL_SET_OUTPUT_FILE )
    {
        dummy_log_error( MODULENAME, "unexpected response: %s", skype_service::StrHelper::to_string( ev->get_type() ).c_str() );

        callback_consume( create_call_error_response(
                static_cast<const skype_service::BasicCallEvent*>( ev )->get_call_id(), 0,
                "unexpected response: " +skype_service::StrHelper::to_string( ev->get_type() ) ) );
        return;
    }
}

bool VoipService::shutdown()
{
    dummy_log_debug( MODULENAME, "shutdown()" );

    bool b = ServerBase::shutdown();

    return b;
}

bool VoipService::register_callback( IVoipServiceCallback * callback )
{
    MUTEX_SCOPE_LOCK( mutex_ );

    if( !callback )
        return false;

    if( callback_ )
        return false;

    callback_   = callback;

    return true;
}

void VoipService::handle( const skype_service::ConnStatusEvent * e )
{
    dummy_log_info( MODULENAME, "conn status %u", e->get_conn_s() );

    cs_ = e->get_conn_s();

    switch_to_ready_if_possible();
}

void VoipService::handle( const skype_service::UserStatusEvent * e )
{
    dummy_log_info( MODULENAME, "user status %u", e->get_user_s() );

    us_ = e->get_user_s();

    switch_to_ready_if_possible();
}

void VoipService::switch_to_ready_if_possible()
{
    if( state_ == UNDEFINED )
    {
        if( cs_ == skype_service::conn_status_e::ONLINE  &&
                ( us_ == skype_service::user_status_e::ONLINE
                        || us_ == skype_service::user_status_e::AWAY
                        || us_ == skype_service::user_status_e::DND
                        || us_ == skype_service::user_status_e::INVISIBLE
                        || us_ == skype_service::user_status_e::NA ) )
        {
            state_ = READY;

            dummy_log_info( MODULENAME, "switched to READY" );
        }
    }
    else if( state_ == READY )
    {
        if( cs_ == skype_service::conn_status_e::OFFLINE
                || cs_ == skype_service::conn_status_e::CONNECTING
                || us_ == skype_service::user_status_e::OFFLINE )
        {
            state_ = UNDEFINED;

            dummy_log_info( MODULENAME, "switched to UNDEFINED" );
        }
    }
}

void VoipService::send_reject_response( uint32 errorcode, const std::string & descr )
{
    callback_consume( create_reject_response( errorcode, descr ) );
}

void VoipService::handle( const skype_service::CurrentUserHandleEvent * e )
{
    dummy_log_info( MODULENAME, "current user handle %s", e->get_par_str().c_str() );
}
void VoipService::on_unknown( const std::string & s )
{
    dummy_log_warn( MODULENAME, "unknown response: %s", s.c_str() );
}
void VoipService::handle( const skype_service::ErrorEvent * e )
{
    dummy_log_debug( MODULENAME, "error %u '%s'", e->get_par_int(), e->get_par_str().c_str() );

    callback_consume( create_error_response( e->get_par_int(), e->get_par_str() ) );
}

void VoipService::handle( const skype_service::CallStatusEvent * e )
{
    uint32 n                    = e->get_call_id();
    skype_service::call_status_e s = e->get_call_s();

    dummy_log_debug( MODULENAME, "call %u status %s", n, skype_service::to_string( s ).c_str() );

    switch( s )
    {
    case skype_service::call_status_e::CANCELLED:
        callback_consume( create_call_end( n, VoipioCallEnd::CANCELLED ) );
        break;

    case skype_service::call_status_e::FINISHED:
        if( pstn_status_ != 0 )
            callback_consume( create_call_end( n, VoipioCallEnd::FAILED_PSTN, pstn_status_, pstn_status_msg_ ) );
        else
            callback_consume( create_call_end( n, VoipioCallEnd::FINISHED ) );
        break;

    case skype_service::call_status_e::ROUTING:
        callback_consume( create_message_t<VoipioDial>( n ) );
        break;

    case skype_service::call_status_e::RINGING:
        callback_consume( create_message_t<VoipioRing>( n ) );
        break;

    case skype_service::call_status_e::INPROGRESS:
        callback_consume( create_message_t<VoipioConnect>( n ) );
        break;

    case skype_service::call_status_e::NONE:
        callback_consume( create_call_end( n, VoipioCallEnd::NONE ) );
        break;

    case skype_service::call_status_e::FAILED:
        callback_consume( create_call_end( n, VoipioCallEnd::FAILED, failure_reason_, failure_reason_msg_ ) );
        break;

    case skype_service::call_status_e::REFUSED:
        callback_consume( create_call_end( n, VoipioCallEnd::REFUSED ) );
        break;

    default:
        dummy_log_warn( MODULENAME, "unhandled status %s (%u)", skype_service::to_string( s ).c_str(), s );
        break;
    }
}

void VoipService::handle( const skype_service::CallPstnStatusEvent * ev )
{
    uint32 n    = ev->get_call_id();
    uint32 e    = ev->get_par_int();
    const std::string & descr = ev->get_par_str();

    dummy_log_debug( MODULENAME, "call %u PSTN status %u '%s'", n, e, descr.c_str() );

    pstn_status_        = ev->get_par_int();
    pstn_status_msg_    = ev->get_par_str();
}

void VoipService::handle( const skype_service::CallDurationEvent * e )
{
    dummy_log_debug( MODULENAME, "call %u dur %u", e->get_call_id(), e->get_par_int() );

    callback_consume( create_call_duration( e->get_call_id(), e->get_par_int() ) );
}

void VoipService::handle( const skype_service::CallFailureReasonEvent * e )
{
    dummy_log_info( MODULENAME, "call %u failure %u", e->get_call_id(), e->get_par_int() );

    failure_reason_     = e->get_par_int();
    failure_reason_msg_ = decode_failure_reason( failure_reason_ );
}

const char* VoipService::decode_failure_reason( const uint32 c )
{
    static const char* table[] =
    {
        "",
        "Miscellaneous error",
        "User or phone number does not exist. Check that a prefix is entered for the phone number, either in the form 003725555555 or +3725555555; the form 3725555555 is incorrect.",
        "User is offline",
        "No proxy found",
        "Session terminated.",
        "No common codec found.",
        "Sound I/O error.",
        "Problem with remote sound device.",
        "Call blocked by recipient.",
        "Recipient not a friend.",
        "Current user not authorized by recipient.",
        "Sound recording error.",
        "Failure to call a commercial contact.",
        "Conference call has been dropped by the host. Note that this does not normally indicate abnormal call termination. Call being dropped for all the participants when the conference host leavs the call is expected behaviour."
    };

    if( c > 14 )
        return "";

    return table[ c ];
}

void VoipService::handle( const skype_service::CallVaaInputStatusEvent * e )
{
    uint32 n    = e->get_call_id();
    uint32 s    = e->get_par_int();

    dummy_log_debug( MODULENAME, "call %u vaa_input_status %u", n, s );

    if( s )
        callback_consume( create_message_t<VoipioPlayStarted>( n ) );
    else
        callback_consume( create_message_t<VoipioPlayStopped>( n ) );
}

void VoipService::callback_consume( const VoipioCallbackObject * req )
{
    if( callback_ )
        callback_->consume( req );
}

uint32_t VoipService::get_next_id()
{
    return ++next_id_;
}

NAMESPACE_VOIP_SERVICE_END

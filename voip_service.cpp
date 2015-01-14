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

// $Id: voip_service.cpp 1378 2015-01-13 19:29:50Z serge $


#include "voip_service.h"           // self

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>     // boost::shared_ptr

#include "../skype_io/skype_io.h"       // SkypeIo
#include "../skype_io/event_parser.h"   // EventParser
#include "../skype_io/str_helper.h"     // StrHelper

#include "../utils/dummy_logger.h"      // dummy_log
#include "../utils/wrap_mutex.h"        // SCOPE_LOCK
#include "../utils/assert.h"            // ASSERT

#include "object_factory.h"             // create_message_t


#define MODULENAME      "VoipService"

NAMESPACE_VOIP_SERVICE_START

VoipService::VoipService():
        ServerBase( this ),
        sio_( nullptr ),
        callback_( nullptr ),
        state_( UNDEFINED ),
        cs_( skype_wrap::conn_status_e::NONE ),
        us_( skype_wrap::user_status_e::NONE ),
        failure_reason_( 0 ),
        pstn_status_( 0 )
{
}

VoipService::~VoipService()
{
}

bool VoipService::init( skype_wrap::SkypeIo * sw )
{
    SCOPE_LOCK( mutex_ );

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

// interface skype_wrap::ISkypeCallback
void VoipService::consume( const skype_wrap::Event * e )
{
    VoipioObjectWrap * ew = new VoipioObjectWrap;

    ew->ptr = e;

    ServerBase::consume( ew );
}


void VoipService::handle( const servt::IObject* req )
{
    SCOPE_LOCK( mutex_ );

    if( typeid( *req ) == typeid( VoipioInitiateCall ) )
    {
        handle( dynamic_cast< const VoipioInitiateCall *>( req ) );
    }
    else if( typeid( *req ) == typeid( VoipioPlayFile ) )
    {
        handle( dynamic_cast< const VoipioPlayFile *>( req ) );
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

    bool b = sio_->call( req->party );

    if( b == false )
    {
        dummy_log_error( MODULENAME, "failed calling: %s", req->party.c_str() );

        if( callback_ )
            callback_->consume( create_error_response( 0, "skype io failed" ) );

        return;
    }

    std::string response = sio_->get_response();

    boost::shared_ptr< skype_wrap::Event > ev( skype_wrap::EventParser::to_event( response ) );

    if( ev->get_type() != skype_wrap::Event::CALL_STATUS )
    {
        if( ev->get_type() == skype_wrap::Event::ERROR )
        {
            uint32 errorcode    = static_cast<skype_wrap::ErrorEvent*>( ev.get() )->get_par_int();
            std::string descr   = static_cast<skype_wrap::ErrorEvent*>( ev.get() )->get_par_str();

            dummy_log_error( MODULENAME, "error occurred: %u '%s'", errorcode, descr.c_str() );

            if( callback_ )
                callback_->consume( create_error_response( errorcode, descr ) );
        }
        else
        {
            dummy_log_error( MODULENAME, "unexpected response: %s", response.c_str() );

            if( callback_ )
                callback_->consume( create_error_response( 0, "unexpected response: " + response ) );
        }
        return;
    }

    uint32 call_id = static_cast<skype_wrap::BasicCallEvent*>( ev.get() )->get_call_id();

    skype_wrap::call_status_e status_code = static_cast<skype_wrap::CallStatusEvent*>( ev.get() )->get_call_s();
    uint32 status  = static_cast<uint32>( status_code );

    dummy_log_debug( MODULENAME, "call initiated: %u, status %s", call_id, skype_wrap::to_string( status_code ).c_str() );

    if( callback_ )
        callback_->consume( create_initiate_call_response( call_id, status ) );

}

void VoipService::handle( const VoipioDrop * req )
{
    // private: no mutex lock

    if( state_ != READY )
    {
        send_reject_response( 0, "not ready" );
        return;
    }

    bool b = sio_->set_call_status( req->call_id, skype_wrap::call_status_e::FINISHED );

    if( b == false )
    {
        if( callback_ )
            callback_->consume( create_error_response( 0, "skype io failed" ) );
        return;
    }


    std::string response = sio_->get_response();

    boost::shared_ptr< skype_wrap::Event > ev( skype_wrap::EventParser::to_event( response ) );

    if( ev->get_type() == skype_wrap::Event::CALL_STATUS )
    {
        skype_wrap::CallStatusEvent * cse = static_cast<skype_wrap::CallStatusEvent *>( ev.get() );
        if( cse->get_call_s() == skype_wrap::call_status_e::FINISHED )
        {
            if( callback_ )
                callback_->consume( create_message_t<VoipioDropResponse>( req->call_id ) );
        }
    }

    dummy_log_error( MODULENAME, "unexpected response: %s", response.c_str() );

    if( callback_ )
        callback_->consume( create_error_response( 0, "unexpected response: " + response ) );


}

void VoipService::handle( const VoipioPlayFile * req )
{
    // private: no mutex lock

    if( state_ != READY )
    {
        send_reject_response( 0, "not ready" );
        return;
    }

    bool b = sio_->alter_call_set_input_file( req->call_id, req->filename );

    if( b == false )
    {
        dummy_log_error( MODULENAME, "failed setting input file: %s", req->filename.c_str() );

        if( callback_ )
            callback_->consume( create_call_error_response( req->call_id, 0, "failed setting input file: " + req->filename ) );

        return;
    }

    std::string response = sio_->get_response();

    boost::shared_ptr< skype_wrap::Event > ev( skype_wrap::EventParser::to_event( response ) );

    if( ev->get_type() != skype_wrap::Event::ALTER_CALL_SET_INPUT_FILE )
    {
        dummy_log_error( MODULENAME, "unexpected response: %s", response.c_str() );

        if( callback_ )
            callback_->consume( create_call_error_response( req->call_id, 0, "unexpected response: " + response ) );

        return;
    }
}

void VoipService::handle( const VoipioRecordFile * req )
{
    // private: no mutex lock

    if( state_ != READY )
    {
        send_reject_response( 0, "not ready" );
        return;
    }

    bool b = sio_->alter_call_set_output_file( req->call_id, req->filename );

    if( b == false )
    {
        dummy_log_error( MODULENAME, "failed setting output file: %s", req->filename.c_str() );

        if( callback_ )
            callback_->consume( create_call_error_response( req->call_id, 0, "failed output input file: " + req->filename ) );

    }

    std::string response = sio_->get_response();

    boost::shared_ptr< skype_wrap::Event > ev( skype_wrap::EventParser::to_event( response ) );

    if( ev->get_type() != skype_wrap::Event::ALTER_CALL_SET_OUTPUT_FILE )
    {
        dummy_log_error( MODULENAME, "unexpected response: %s", response.c_str() );

        if( callback_ )
            callback_->consume( create_call_error_response( req->call_id, 0, "unexpected response: " + response ) );

    }
}

void VoipService::handle( const VoipioObjectWrap * req )
{
    // private: no mutex lock

    const skype_wrap::Event * ev = static_cast<const skype_wrap::Event*>( req->ptr );

    ASSERT( ev );

    skype_wrap::Event::type_e id = ev->get_type();

    switch( id )
    {
    case skype_wrap::Event::CONNSTATUS:
        handle( static_cast<const skype_wrap::ConnStatusEvent*>( ev ) );
        break;

    case skype_wrap::Event::USERSTATUS:
        handle( static_cast<const skype_wrap::UserStatusEvent*>( ev ) );
        break;

    case skype_wrap::Event::CURRENTUSERHANDLE:
        handle( static_cast<const skype_wrap::CurrentUserHandleEvent*>( ev ) );
        break;

    case skype_wrap::Event::USER_ONLINE_STATUS:
        break;

    case skype_wrap::Event::CALL:
        break;
    case skype_wrap::Event::CALL_DURATION:
        handle( static_cast<const skype_wrap::CallDurationEvent*>( ev ) );
        break;

    case skype_wrap::Event::CALL_STATUS:
        handle( static_cast<const skype_wrap::CallStatusEvent*>( ev ) );
        break;

    case skype_wrap::Event::CALL_PSTN_STATUS:
        handle( static_cast<const skype_wrap::CallPstnStatusEvent*>( ev ) );
        break;

    case skype_wrap::Event::CALL_FAILUREREASON:
        handle( static_cast<const skype_wrap::CallFailureReasonEvent*>( ev ) );
        break;

    case skype_wrap::Event::CALL_VAA_INPUT_STATUS:
        handle( static_cast<const skype_wrap::CallVaaInputStatusEvent*>( ev ) );
        break;

    case skype_wrap::Event::ERROR:
        handle( static_cast<const skype_wrap::ErrorEvent*>( ev ) );
        break;

    case skype_wrap::Event::UNDEF:
        break;

    case skype_wrap::Event::CHAT:
    case skype_wrap::Event::CHATMEMBER:
        // simply ignore
        break;

    case skype_wrap::Event::UNKNOWN:
    default:
        on_unknown( "???" );
        break;
    }

    delete ev;
}

bool VoipService::shutdown()
{
    ServerBase::shutdown();

    return sio_->shutdown();
}

bool VoipService::register_callback( IVoipServiceCallback * callback )
{
    SCOPE_LOCK( mutex_ );

    if( !callback )
        return false;

    if( callback_ )
        return false;

    callback_   = callback;

    return true;
}

void VoipService::handle( const skype_wrap::ConnStatusEvent * e )
{
    dummy_log_info( MODULENAME, "conn status %u", e->get_conn_s() );

    cs_ = e->get_conn_s();

    switch_to_ready_if_possible();
}

void VoipService::handle( const skype_wrap::UserStatusEvent * e )
{
    dummy_log_info( MODULENAME, "user status %u", e->get_user_s() );

    us_ = e->get_user_s();

    switch_to_ready_if_possible();
}

void VoipService::switch_to_ready_if_possible()
{
    if( cs_ != skype_wrap::conn_status_e::ONLINE )
        return;

    if( us_ != skype_wrap::user_status_e::ONLINE )
        return;

    ASSERT( state_ == UNDEFINED );

    state_ = READY;
}

void VoipService::send_reject_response( uint32 errorcode, const std::string & descr )
{
    if( callback_ )
        callback_->consume( create_reject_response( errorcode, descr ) );
}

void VoipService::handle( const skype_wrap::CurrentUserHandleEvent * e )
{
    dummy_log_info( MODULENAME, "current user handle %s", e->get_par_str().c_str() );
}
void VoipService::on_unknown( const std::string & s )
{
    dummy_log_warn( MODULENAME, "unknown response: %s", s.c_str() );
}
void VoipService::handle( const skype_wrap::ErrorEvent * e )
{
    dummy_log_debug( MODULENAME, "error %u '%s'", e->get_par_int(), e->get_par_str().c_str() );

    if( callback_ )
        callback_->consume( create_error_response( e->get_par_int(), e->get_par_str() ) );
}

void VoipService::handle( const skype_wrap::CallStatusEvent * e )
{
    uint32 n                    = e->get_call_id();
    skype_wrap::call_status_e s = e->get_call_s();

    dummy_log_debug( MODULENAME, "call %u status %s", n, skype_wrap::to_string( s ).c_str() );

    if( callback_ == nullptr )
        return;

    switch( s )
    {
    case skype_wrap::call_status_e::CANCELLED:
        callback_->consume( create_call_end( n, VoipioCallEnd::CANCELLED ) );
        break;

    case skype_wrap::call_status_e::FINISHED:
        if( pstn_status_ != 0 )
            callback_->consume( create_call_end( n, VoipioCallEnd::FAILED_PSTN, pstn_status_, pstn_status_msg_ ) );
        else
            callback_->consume( create_call_end( n, VoipioCallEnd::FINISHED ) );
        break;

    case skype_wrap::call_status_e::ROUTING:
        callback_->consume( create_message_t<VoipioDial>( n ) );
        break;

    case skype_wrap::call_status_e::RINGING:
        callback_->consume( create_message_t<VoipioRing>( n ) );
        break;

    case skype_wrap::call_status_e::INPROGRESS:
        callback_->consume( create_message_t<VoipioConnect>( n ) );
        break;

    case skype_wrap::call_status_e::NONE:
        callback_->consume( create_call_end( n, VoipioCallEnd::NONE ) );
        break;

    case skype_wrap::call_status_e::FAILED:
        callback_->consume( create_call_end( n, VoipioCallEnd::FAILED, failure_reason_, failure_reason_msg_ ) );
        break;

    case skype_wrap::call_status_e::REFUSED:
        callback_->consume( create_call_end( n, VoipioCallEnd::REFUSED ) );
        break;

    default:
        dummy_log_warn( MODULENAME, "unhandled status %s (%u)", skype_wrap::to_string( s ).c_str(), s );
        break;
    }
}

void VoipService::handle( const skype_wrap::CallPstnStatusEvent * ev )
{
    uint32 n    = ev->get_call_id();
    uint32 e    = ev->get_par_int();
    const std::string & descr = ev->get_par_str();

    dummy_log_debug( MODULENAME, "call %u PSTN status %u '%s'", n, e, descr.c_str() );

    pstn_status_        = ev->get_par_int();
    pstn_status_msg_    = ev->get_par_str();
}

void VoipService::handle( const skype_wrap::CallDurationEvent * e )
{
    dummy_log_debug( MODULENAME, "call %u dur %u", e->get_call_id(), e->get_par_int() );

    if( callback_ )
        callback_->consume( create_call_duration( e->get_call_id(), e->get_par_int() ) );
}

void VoipService::handle( const skype_wrap::CallFailureReasonEvent * e )
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

void VoipService::handle( const skype_wrap::CallVaaInputStatusEvent * e )
{
    uint32 n    = e->get_call_id();
    uint32 s    = e->get_par_int();

    dummy_log_debug( MODULENAME, "call %u vaa_input_status %u", n, s );

    if( callback_ == nullptr )
        return;

    if( s )
        callback_->consume( create_message_t<VoipioPlayStarted>( n ) );
    else
        callback_->consume( create_message_t<VoipioPlayStopped>( n ) );
}

NAMESPACE_VOIP_SERVICE_END

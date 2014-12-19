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

// $Id: voip_service.cpp 1273 2014-12-18 18:18:51Z serge $


#include "voip_service.h"           // self

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>     // boost::shared_ptr

#include "../skype_io/skype_io.h"       // SkypeIo
#include "../skype_io/event_parser.h"   // EventParser
#include "../skype_io/events.h"         // BasicCallEvent
#include "../skype_io/str_helper.h"     // StrHelper

#include "../utils/dummy_logger.h"      // dummy_log
#include "../utils/wrap_mutex.h"        // SCOPE_LOCK
#include "../utils/assert.h"            // ASSERT


#define MODULENAME      "VoipService"

NAMESPACE_VOIP_SERVICE_START

VoipService::VoipService() :
        sio_( nullptr ),
        callback_( nullptr ),
        state_( UNDEFINED ),
        cs_( skype_wrap::conn_status_e::NONE ),
        us_( skype_wrap::user_status_e::NONE ),
        errorcode_( errorcode_e::NONE )
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

void VoipService::consume( const VoipioObject * req )
{
    ServerBase::consume( req );
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

    bool b = sio_->alter_call_hangup( req->call_id );

    if( b == false )
    {
        dummy_log_error( MODULENAME, "failed dropping call: %d", req->call_id );

        if( callback_ )
            callback_->consume( create_error( req->call_id, "failed dropping call" ) );

        return;
    }
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
            callback_->consume( create_error( req->call_id, "failed setting input file: " + req->filename ) );

        return;
    }

    std::string response = sio_->get_response();

    boost::shared_ptr< skype_wrap::Event > ev( skype_wrap::EventParser::to_event( response ) );

    if( ev->get_type() != skype_wrap::Event::ALTER_CALL_SET_INPUT_FILE )
    {
        dummy_log_error( MODULENAME, "unexpected response: %s", response.c_str() );

        if( callback_ )
            callback_->consume( create_error( req->call_id, "unexpected response: " + response ) );

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
            callback_->consume( create_error( req->call_id, "failed output input file: " + req->filename ) );

    }

    std::string response = sio_->get_response();

    boost::shared_ptr< skype_wrap::Event > ev( skype_wrap::EventParser::to_event( response ) );

    if( ev->get_type() != skype_wrap::Event::ALTER_CALL_SET_OUTPUT_FILE )
    {
        dummy_log_error( MODULENAME, "unexpected response: %s", response.c_str() );

        if( callback_ )
            callback_->consume( create_error( req->call_id, "unexpected response: " + response ) );

    }
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

void VoipService::on_conn_status( const skype_wrap::conn_status_e s )
{
    dummy_log_info( MODULENAME, "conn status %u", s );

    SCOPE_LOCK( mutex_ );

    cs_ = s;

    switch_to_ready_if_possible();
}

void VoipService::on_user_status( const skype_wrap::user_status_e s )
{
    dummy_log_info( MODULENAME, "user status %u", s );

    SCOPE_LOCK( mutex_ );

    us_ = s;

    switch_to_ready_if_possible();
}

void VoipService::switch_to_ready_if_possible()
{
    if( cs_ != skype_wrap::conn_status_e::ONLINE )
        return;

    if( us_ != skype_wrap::user_status_e::ONLINE )
        return;

    ASSERT( state_ == UNDEFINED );

    state_ == READY;
}

void VoipService::send_reject_response( uint32 errorcode, const std::string & descr )
{
    if( callback_ )
        callback_->consume( create_reject_response( errorcode, descr ) );
}

void VoipService::on_current_user_handle( const std::string & s )
{
    dummy_log_info( MODULENAME, "current user handle %s", s.c_str() );

    SCOPE_LOCK( mutex_ );
}
void VoipService::on_unknown( const std::string & s )
{
    dummy_log_warn( MODULENAME, "unknown response: %s", s.c_str() );

    SCOPE_LOCK( mutex_ );
}
void VoipService::on_error( const uint32 error, const std::string & descr )
{
    dummy_log_debug( MODULENAME, "error %u '%s'", error, descr.c_str() );

    SCOPE_LOCK( mutex_ );

    if( callback_ )
        callback_->consume( create_error_response( error, descr ) );
}

void VoipService::on_call_status( const uint32 n, const skype_wrap::call_status_e s )
{
    dummy_log_debug( MODULENAME, "call %u status %s", n, skype_wrap::to_string( s ).c_str() );

    SCOPE_LOCK( mutex_ );

    if( callback_ == nullptr )
        return;

    switch( s )
    {
    case skype_wrap::call_status_e::CANCELLED:
        callback_->on_call_end( n, static_cast<uint32>( errorcode_ ) );
        break;

    case skype_wrap::call_status_e::FINISHED:
        callback_->on_call_end( n, static_cast<uint32>( errorcode_ ) );
        break;

    case skype_wrap::call_status_e::ROUTING:
        callback_->on_dial( n );
        break;

    case skype_wrap::call_status_e::RINGING:
        callback_->on_ring( n );
        break;

    case skype_wrap::call_status_e::INPROGRESS:
        callback_->on_connect( n );
        break;

    case skype_wrap::call_status_e::NONE:
        callback_->on_call_end( n, static_cast<uint32>( errorcode_ ) );
        break;

    case skype_wrap::call_status_e::FAILED:
    case skype_wrap::call_status_e::REFUSED:
        callback_->on_error( n, static_cast<uint32>( errorcode_ ) );
        break;

    default:
        dummy_log_warn( MODULENAME, "unhandled status %s (%u)", skype_wrap::to_string( s ).c_str(), s );
        break;
    }
}
void VoipService::on_call_pstn_status( const uint32 n, const uint32 e, const std::string & descr )
{
    dummy_log_debug( MODULENAME, "call %u PSTN status %u '%s'", n, e, descr.c_str() );

    SCOPE_LOCK( mutex_ );

    if( !has_callback() )
        return;

    if( e != 0 )
    {
        dummy_log_error( MODULENAME, "call %u - got PSTN error %u '%s'", n, e, descr.c_str() );

        callback_->on_fatal_error( n, static_cast<uint32>( errorcode_ ) );
    }
}
void VoipService::on_call_duration( const uint32 n, const uint32 t )
{
    dummy_log_debug( MODULENAME, "call %u dur %u", n, t );

    SCOPE_LOCK( mutex_ );

    if( !has_callback() )
        return;

    callback_->on_call_duration( n, t );
}

void VoipService::on_call_failure_reason( const uint32 n, const uint32 c )
{
    dummy_log_info( MODULENAME, "call %u failure %u", n, c );

    SCOPE_LOCK( mutex_ );

    errorcode_  = decode_failure_reason( c );
}

errorcode_e VoipService::decode_failure_reason( const uint32 c )
{
    static const errorcode_e table[] = {
            errorcode_e::NONE,
            errorcode_e::UNKNOWN,        //    1 CALL 181 FAILUREREASON 1 Miscellaneous error
            errorcode_e::WRONG_NUMBER,   //    2 CALL 181 FAILUREREASON 2 User or phone number does not exist. Check that a prefix is entered for the phone number, either in the form 003725555555 or +3725555555; the form 3725555555 is incorrect.
            errorcode_e::SUBSCRIBER_OFFLINE,   //    3 CALL 181 FAILUREREASON 3 User is offline
            errorcode_e::VOIP_SPECIFIC,  //    4 CALL 181 FAILUREREASON 4 No proxy found
            errorcode_e::VOIP_SPECIFIC,  //    5 CALL 181 FAILUREREASON 5 Session terminated.
            errorcode_e::VOIP_SPECIFIC,  //    6 CALL 181 FAILUREREASON 6 No common codec found.
            errorcode_e::HW_ERROR,       //    7 CALL 181 FAILUREREASON 7 Sound I/O error.
            errorcode_e::HW_ERROR,       //    8 CALL 181 FAILUREREASON 8 Problem with remote sound device.
            errorcode_e::REJECTED,       //    9 CALL 181 FAILUREREASON 9 Call blocked by recipient.
            errorcode_e::REJECTED,       //    10 CALL 181 FAILUREREASON 10 Recipient not a friend.
            errorcode_e::REJECTED,       //    11 CALL 181 FAILUREREASON 11 Current user not authorized by recipient.
            errorcode_e::VOIP_SPECIFIC,  //    12 CALL 181 FAILUREREASON 12 Sound recording error.
            errorcode_e::VOIP_SPECIFIC,  //    13 CALL 181 FAILUREREASON 13 Failure to call a commercial contact.
            errorcode_e::NONE,           //    14 CALL 181 FAILUREREASON 14 Conference call has been dropped by the host. Note that this does not normally indicate abnormal call termination. Call being dropped for all the participants when the conference host leavs the call is expected behaviour.
    };

    if( c <= 14 )
        return table[c];

    return errorcode_e::UNKNOWN;
}

void VoipService::on_call_vaa_input_status( const uint32 n, const uint32 s )
{
    dummy_log_debug( MODULENAME, "call %u vaa_input_status %u", n, s );

    SCOPE_LOCK( mutex_ );

    if( callback_ == nullptr )
        return;

    if( s )
        callback_->on_play_start( n );
    else
        callback_->on_play_stop( n );
}

NAMESPACE_VOIP_SERVICE_END

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

// $Id: voip_service.cpp 1099 2014-10-01 19:02:11Z serge $


#include "voip_service.h"           // self

#include <boost/bind.hpp>

#include "../skype_io/skype_io.h"       // SkypeIo
#include "../skype_io/event_parser.h"   // EventParser

#include "../utils/dummy_logger.h"      // dummy_log
#include "../utils/wrap_mutex.h"        // SCOPE_LOCK

#include "namespace_voipservice.h"  // NAMESPACE_VOIP_SERVICE_START

#define MODULENAME      "VoipService"

NAMESPACE_VOIP_SERVICE_START

VoipService::VoipService() :
        sio_( 0L )
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

bool VoipService::is_ready() const
{
    SCOPE_LOCK( mutex_ );

    if( sio_ )
        return true;

    return false;
}
bool VoipService::initiate_call( const std::string & party, uint32 & call_id, uint32 & status )
{
    SCOPE_LOCK( mutex_ );

    bool b = sio_->call( party );

    if( !b )
    {
        dummy_log_error( MODULENAME, "failed calling: %s", party.c_str() );
        return false;
    }

    std::string response = sio_->get_response();

    skype_wrap::Event ev = skype_wrap::EventParser::to_event( response );

    if( ev.get_id() != skype_wrap::Event::CALL_STATUS )
    {
        dummy_log_error( MODULENAME, "unexpected response: %s", response.c_str() );
        return false;
    }

    call_id = ev.get_call_id();
    status  = static_cast<uint32>( ev.get_call_s() );

    return true;
}

bool VoipService::drop_call( uint32 call_id )
{
    SCOPE_LOCK( mutex_ );

    bool b = sio_->alter_call_hangup( call_id );

    if( !b )
    {
        dummy_log_error( MODULENAME, "failed dropping call: %d", call_id );
        return false;
    }

    return true;
}

bool VoipService::set_input_file( uint32 call_id, const std::string & filename )
{
    SCOPE_LOCK( mutex_ );

    bool b = sio_->alter_call_set_input_file( call_id, filename );

    if( !b )
    {
        dummy_log_error( MODULENAME, "failed setting input file: %s", filename.c_str() );
        return false;
    }

    std::string response = sio_->get_response();

    skype_wrap::Event ev = skype_wrap::EventParser::to_event( response );

    if( ev.get_id() != skype_wrap::Event::ALTER_CALL_SET_INPUT_FILE )
    {
        dummy_log_error( MODULENAME, "unexpected response: %s", response.c_str() );
        return false;
    }

    call_id = ev.get_call_id();

    return true;
}

bool VoipService::set_output_file( uint32 call_id, const std::string & filename )
{
    SCOPE_LOCK( mutex_ );

    bool b = sio_->alter_call_set_output_file( call_id, filename );

    if( !b )
    {
        dummy_log_error( MODULENAME, "failed setting output file: %s", filename.c_str() );
        return false;
    }

    std::string response = sio_->get_response();

    skype_wrap::Event ev = skype_wrap::EventParser::to_event( response );

    if( ev.get_id() != skype_wrap::Event::ALTER_CALL_SET_OUTPUT_FILE )
    {
        dummy_log_error( MODULENAME, "unexpected response: %s", response.c_str() );
        return false;
    }

    call_id = ev.get_call_id();

    return true;
}


bool VoipService::shutdown()
{
    return sio_->shutdown();
}

skype_wrap::ISkypeCallback* VoipService::get_event_handler()
{
    SCOPE_LOCK( mutex_ );

    return &dio_;
}

bool VoipService::register_callback( IVoipServiceCallback * callback )
{
    SCOPE_LOCK( mutex_ );

    return dio_.register_callback( callback );
}


VoipService::DialerIO::DialerIO():
        cs_( skype_wrap::conn_status_e::NONE ), us_( skype_wrap::user_status_e::NONE ), callback_( 0L ), errorcode_( ERR_NONE )
{
}

void VoipService::DialerIO::on_conn_status( const skype_wrap::conn_status_e s )
{
    dummy_log_info( MODULENAME, "conn status %u", s );

    SCOPE_LOCK( mutex_ );

    cs_ = s;

    send_ready_if_possible();
}
void VoipService::DialerIO::on_user_status( const skype_wrap::user_status_e s )
{
    dummy_log_info( MODULENAME, "user status %u", s );

    SCOPE_LOCK( mutex_ );

    us_ = s;

    send_ready_if_possible();
}

void VoipService::DialerIO::send_ready_if_possible()
{
    if( cs_ != skype_wrap::conn_status_e::ONLINE )
        return;

    if( us_ != skype_wrap::user_status_e::ONLINE )
        return;

    if( !has_callback() )
        return;

    callback_->on_ready( 0 );
}

void VoipService::DialerIO::on_current_user_handle( const std::string & s )
{
    dummy_log_info( MODULENAME, "current user handle %s", s.c_str() );

    SCOPE_LOCK( mutex_ );
}
void VoipService::DialerIO::on_unknown( const std::string & s )
{
    dummy_log_warn( MODULENAME, "unknown response: %s", s.c_str() );

    SCOPE_LOCK( mutex_ );
}
void VoipService::DialerIO::on_call_status( const uint32 n, const skype_wrap::call_status_e s )
{
    dummy_log_debug( MODULENAME, "call %u status %u", n, s );

    SCOPE_LOCK( mutex_ );

    if( !has_callback() )
        return;

    switch( s )
    {
    case skype_wrap::call_status_e::FINISHED:
        callback_->on_call_end( n, errorcode_ );
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
        callback_->on_call_end( n, errorcode_ );
        break;

    default:
        dummy_log_warn( MODULENAME, "unhandled status %u", s );
        break;
    }
}
void VoipService::DialerIO::on_call_duration( const uint32 n, const uint32 t )
{
    dummy_log_debug( MODULENAME, "call %u dur %u", n, t );

    SCOPE_LOCK( mutex_ );

    if( !has_callback() )
        return;

    callback_->on_call_duration( n, t );
}

void VoipService::DialerIO::on_call_failure_reason( const uint32 n, const uint32 c )
{
    dummy_log_info( MODULENAME, "call %u failure %u", n, c );

    SCOPE_LOCK( mutex_ );

    errorcode_  = decode_failure_reason( c );
}

errorcode_e VoipService::DialerIO::decode_failure_reason( const uint32 c )
{
    static const errorcode_e table[] = {
            ERR_NONE,
            ERR_UNKNOWN,        //    1 CALL 181 FAILUREREASON 1 Miscellaneous error
            ERR_WRONG_NUMBER,   //    2 CALL 181 FAILUREREASON 2 User or phone number does not exist. Check that a prefix is entered for the phone number, either in the form 003725555555 or +3725555555; the form 3725555555 is incorrect.
            ERR_SUBSCRIBER_OFFLINE,   //    3 CALL 181 FAILUREREASON 3 User is offline
            ERR_VOIP_SPECIFIC,  //    4 CALL 181 FAILUREREASON 4 No proxy found
            ERR_VOIP_SPECIFIC,  //    5 CALL 181 FAILUREREASON 5 Session terminated.
            ERR_VOIP_SPECIFIC,  //    6 CALL 181 FAILUREREASON 6 No common codec found.
            ERR_HW_ERROR,       //    7 CALL 181 FAILUREREASON 7 Sound I/O error.
            ERR_HW_ERROR,       //    8 CALL 181 FAILUREREASON 8 Problem with remote sound device.
            ERR_REJECTED,       //    9 CALL 181 FAILUREREASON 9 Call blocked by recipient.
            ERR_REJECTED,       //    10 CALL 181 FAILUREREASON 10 Recipient not a friend.
            ERR_REJECTED,       //    11 CALL 181 FAILUREREASON 11 Current user not authorized by recipient.
            ERR_VOIP_SPECIFIC,  //    12 CALL 181 FAILUREREASON 12 Sound recording error.
            ERR_VOIP_SPECIFIC,  //    13 CALL 181 FAILUREREASON 13 Failure to call a commercial contact.
            ERR_NONE,           //    14 CALL 181 FAILUREREASON 14 Conference call has been dropped by the host. Note that this does not normally indicate abnormal call termination. Call being dropped for all the participants when the conference host leavs the call is expected behaviour.
    };

    if( c <= 14 )
        return table[c];

    return ERR_UNKNOWN;
}

void VoipService::DialerIO::on_call_vaa_input_status( const uint32 n, const uint32 s )
{
    dummy_log_debug( MODULENAME, "call %u vaa_input_status %u", n, s );

    SCOPE_LOCK( mutex_ );

    if( !has_callback() )
        return;

    if( s )
        callback_->on_play_start( n );
    else
        callback_->on_play_stop( n );
}

bool VoipService::DialerIO::has_callback() const
{
    return callback_ != 0L;
}

bool VoipService::DialerIO::register_callback( IVoipServiceCallback * callback )
{
    SCOPE_LOCK( mutex_ );

    if( !callback )
        return false;

    if( callback_ )
        return false;

    callback_   = callback;

    return true;
}


NAMESPACE_VOIP_SERVICE_END

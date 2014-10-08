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

// $Id: voip_service.h 1114 2014-10-08 17:33:43Z serge $

#ifndef VOIP_SERVICE_H
#define VOIP_SERVICE_H

#include <string>                   // std::string
#include <boost/thread.hpp>         // boost::mutex

#include "i_voip_service.h"             // IVoipService
#include "i_voip_service_callback.h"    // IVoipServiceCallback
#include "../skype_io/i_skype_callback.h"   // ISkypeCallback
#include "voip_types.h"             // errorcode_e

#include "namespace_lib.h"          // NAMESPACE_VOIP_SERVICE_START

namespace skype_wrap
{
class SkypeIo;
}

NAMESPACE_VOIP_SERVICE_START

class VoipService: virtual public IVoipService
{
private:

    class DialerIO: virtual public skype_wrap::ISkypeCallback
    {
    public:
        DialerIO();

        bool register_callback( IVoipServiceCallback * callback );

        // callback interface
        virtual void on_conn_status( const skype_wrap::conn_status_e s );
        virtual void on_user_status( const skype_wrap::user_status_e s );
        virtual void on_current_user_handle( const std::string & s );
        virtual void on_unknown( const std::string & s );
        virtual void on_call_status( const uint32 n, const skype_wrap::call_status_e s );
        virtual void on_call_duration( const uint32 n, const uint32 t );
        virtual void on_call_failure_reason( const uint32 n, const uint32 c );
        virtual void on_call_vaa_input_status( const uint32 n, const uint32 s );

    private:

        void send_ready_if_possible();

        static errorcode_e decode_failure_reason( uint32 c );

        bool has_callback() const;

    private:
        mutable boost::mutex        mutex_;

        skype_wrap::conn_status_e   cs_;
        skype_wrap::user_status_e   us_;

        IVoipServiceCallback        * callback_;

        errorcode_e                 errorcode_;
    };

public:

public:
    VoipService();

    ~VoipService();

    virtual bool is_ready() const;
    virtual bool initiate_call( const std::string & party, uint32 & call_id, uint32 & status );
    virtual bool drop_call( uint32 call_id );
    virtual bool set_input_file( uint32 call_id, const std::string & filename );
    virtual bool set_output_file( uint32 call_id, const std::string & filename );

    virtual bool shutdown();

    bool init( skype_wrap::SkypeIo * sw );

    skype_wrap::ISkypeCallback* get_event_handler();

    bool register_callback( IVoipServiceCallback * callback );

private:

public:


private:
    mutable boost::mutex        mutex_;

    skype_wrap::SkypeIo         * sio_;

    DialerIO                    dio_;
};

NAMESPACE_VOIP_SERVICE_END

#endif  // VOIP_SERVICE_H

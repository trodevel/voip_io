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

// $Id: voip_service.h 1273 2014-12-18 18:18:51Z serge $

#ifndef VOIP_SERVICE_H
#define VOIP_SERVICE_H

#include <string>                   // std::string
#include <boost/thread.hpp>         // boost::mutex

#include "i_voip_service.h"             // IVoipService
#include "i_voip_service_callback.h"    // IVoipServiceCallback
#include "../skype_io/i_skype_callback.h"   // ISkypeCallback
#include "../servt/server_t.h"          // ServerT
#include "voip_types.h"             // errorcode_e

#include "objects.h"                    // VoipioObject
#include "namespace_lib.h"          // NAMESPACE_VOIP_SERVICE_START

namespace skype_wrap
{
class SkypeIo;
}

NAMESPACE_VOIP_SERVICE_START

class VoipService;

typedef servt::ServerT< const servt::IObject*, VoipService> ServerBase;

class VoipService:
        public ServerBase,
        virtual public IVoipService,
        virtual public skype_wrap::ISkypeCallback
{
    friend ServerBase;

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
        virtual void on_error( const uint32 error, const std::string & descr );
        virtual void on_call_status( const uint32 n, const skype_wrap::call_status_e s );
        virtual void on_call_pstn_status( const uint32 n, const uint32 e, const std::string & descr );
        virtual void on_call_duration( const uint32 n, const uint32 t );
        virtual void on_call_failure_reason( const uint32 n, const uint32 c );
        virtual void on_call_vaa_input_status( const uint32 n, const uint32 s );

    private:

        void switch_to_ready_if_possible();

        static errorcode_e decode_failure_reason( uint32 c );



    private:
        mutable boost::mutex        mutex_;

        skype_wrap::conn_status_e   cs_;
        skype_wrap::user_status_e   us_;

        IVoipServiceCallback        * callback_;

        errorcode_e                 errorcode_;
    };

public:
    VoipService();

    ~VoipService();

    bool init( skype_wrap::SkypeIo * sw );

    virtual bool shutdown();

    bool register_callback( IVoipServiceCallback * callback );

    // interface IVoipService
    void consume( const VoipioObject * req );

    // interface skype_wrap::ISkypeCallback
    virtual void on_conn_status( const skype_wrap::conn_status_e s );
    virtual void on_user_status( const skype_wrap::user_status_e s );
    virtual void on_current_user_handle( const std::string & s );
    virtual void on_unknown( const std::string & s );
    virtual void on_error( const uint32 error, const std::string & descr );
    virtual void on_call_status( const uint32 n, const skype_wrap::call_status_e s );
    virtual void on_call_pstn_status( const uint32 n, const uint32 e, const std::string & descr );
    virtual void on_call_duration( const uint32 n, const uint32 t );
    virtual void on_call_failure_reason( const uint32 n, const uint32 c );
    virtual void on_call_vaa_input_status( const uint32 n, const uint32 s );

private:

    // IVoipService interface
    void handle( const servt::IObject* req );

    void handle( const VoipioInitiateCall * req );
    void handle( const VoipioDrop * req );
    void handle( const VoipioPlayFile * req );
    void handle( const VoipioRecordFile * req );

    static errorcode_e decode_failure_reason( uint32 c );
    void switch_to_ready_if_possible();
    void send_reject_response( uint32 errorcode, const std::string & descr );

private:

    enum state_e
    {
        UNDEFINED,
        READY
    };

private:
    mutable boost::mutex        mutex_;

    skype_wrap::SkypeIo         * sio_;
    IVoipServiceCallback        * callback_;

    state_e                     state_;

    skype_wrap::conn_status_e   cs_;
    skype_wrap::user_status_e   us_;
    errorcode_e                 errorcode_;
};

NAMESPACE_VOIP_SERVICE_END

#endif  // VOIP_SERVICE_H

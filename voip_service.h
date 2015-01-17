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

// $Revision: 1404 $ $Date:: 2015-01-16 #$ $Author: serge $

#ifndef VOIP_SERVICE_H
#define VOIP_SERVICE_H

#include <string>                   // std::string
#include <boost/thread.hpp>         // boost::mutex

#include "i_voip_service.h"             // IVoipService
#include "i_voip_service_callback.h"    // IVoipServiceCallback
#include "../skype_io/i_skype_callback.h"   // ISkypeCallback
#include "../skype_io/events.h"             // ConnStatusEvent, ...
#include "../threcon/i_controllable.h"      // IControllable
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
        virtual public skype_wrap::ISkypeCallback,
        virtual public threcon::IControllable
{
    friend ServerBase;

public:
    VoipService();

    ~VoipService();

    bool init( skype_wrap::SkypeIo * sw );

    bool register_callback( IVoipServiceCallback * callback );

    // interface threcon::IControllable
    virtual bool shutdown();

    // interface IVoipService
    void consume( const VoipioObject * req );

    // interface skype_wrap::ISkypeCallback
    virtual void consume( const skype_wrap::Event * e );

private:

    // ServerT interface
    void handle( const servt::IObject* req );

    // IVoipService interface
    void handle( const VoipioInitiateCall * req );
    void handle( const VoipioDrop * req );
    void handle( const VoipioPlayFile * req );
    void handle( const VoipioRecordFile * req );
    void handle( const VoipioObjectWrap * req );

    // interface skype_wrap::ISkypeCallback
    void handle( const skype_wrap::ConnStatusEvent * e );
    void handle( const skype_wrap::UserStatusEvent * e );
    void handle( const skype_wrap::CurrentUserHandleEvent * e );
    void handle( const skype_wrap::ErrorEvent * e );
    void handle( const skype_wrap::CallStatusEvent * e );
    void handle( const skype_wrap::CallPstnStatusEvent * e );
    void handle( const skype_wrap::CallDurationEvent * e );
    void handle( const skype_wrap::CallFailureReasonEvent * e );
    void handle( const skype_wrap::CallVaaInputStatusEvent * e );

    void on_unknown( const std::string & s );


    static const char* decode_failure_reason( uint32 c );
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
    uint32                      failure_reason_;
    std::string                 failure_reason_msg_;
    uint32                      pstn_status_;
    std::string                 pstn_status_msg_;
};

NAMESPACE_VOIP_SERVICE_END

#endif  // VOIP_SERVICE_H

/*

VOIP service callback interface.

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

// $Id: i_voip_service_callback.h 549 2014-05-22 17:19:33Z serge $

#ifndef I_VOIP_SERVICE_CALLBACK_H
#define I_VOIP_SERVICE_CALLBACK_H

#include <string>                   // std::string
#include "../utils/types.h"         // uint32

#include "namespace_voipservice.h"  // NAMESPACE_VOIP_SERVICE_START

NAMESPACE_VOIP_SERVICE_START


class IVoipServiceCallback
{
public:

public:
    virtual ~IVoipServiceCallback() {};

    virtual void on_ready( uint32 errorcode )                                               = 0;
    virtual void on_error( uint32 call_id, uint32 errorcode )                               = 0;
    virtual void on_call_end( uint32 call_id, uint32 errorcode )                            = 0;
    virtual void on_dial( uint32 call_id )                                                  = 0;
    virtual void on_ring( uint32 call_id )                                                  = 0;
    virtual void on_connect( uint32 call_id )                                               = 0;
    virtual void on_call_duration( uint32 call_id, uint32 t )                               = 0;
    virtual void on_play_start( uint32 call_id )                                            = 0;
    virtual void on_play_stop( uint32 call_id )                                             = 0;
};

NAMESPACE_VOIP_SERVICE_END

#endif  // I_VOIP_SERVICE_CALLBACK_H

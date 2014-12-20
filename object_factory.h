/*

Voip IO objects.

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


// $Id: object_factory.h 1276 2014-12-19 18:10:07Z serge $

#ifndef VOIP_SERVICE_OBJECT_FACTORY_H
#define VOIP_SERVICE_OBJECT_FACTORY_H

#include "objects.h"    // VoipioObject...

NAMESPACE_VOIP_SERVICE_START

inline void init_call_id( VoipioCallObject * obj, uint32 call_id )
{
    obj->call_id = call_id;
}

inline void init_call_id( VoipioCallbackCallObject * obj, uint32 call_id )
{
    obj->call_id = call_id;
}

template <class _T>
_T *create_message_t( uint32 call_id )
{
    _T *res = new _T;

    init_call_id( res, call_id );

    return res;
}

inline VoipioErrorResponse *create_error_response( uint32 errorcode, const std::string & descr )
{
    VoipioErrorResponse *res = new VoipioErrorResponse;

    res->errorcode  = errorcode;
    res->descr      = descr;

    return res;
}

inline VoipioRejectResponse *create_reject_response( uint32 errorcode, const std::string & descr )
{
    VoipioRejectResponse *res = new VoipioRejectResponse;

    res->errorcode  = errorcode;
    res->descr      = descr;

    return res;
}

inline VoipioInitiateCallResponse *create_initiate_call_response( uint32 call_id, uint32 status )
{
    VoipioInitiateCallResponse *res = new VoipioInitiateCallResponse;

    res->call_id    = call_id;
    res->status     = status;

    return res;
}

inline VoipioCallDuration *create_call_duration( uint32 call_id, uint32 t )
{
    VoipioCallDuration *res = create_message_t<VoipioCallDuration>( call_id );

    res->t  = t;

    return res;
}

inline VoipioCallEnd *create_call_end( uint32 call_id, uint32 errorcode )
{
    VoipioCallEnd *res = create_message_t<VoipioCallEnd>( call_id );

    res->errorcode  = errorcode;

    return res;
}

inline VoipioError *create_error( uint32 call_id, const std::string & error )
{
    VoipioError *res = create_message_t<VoipioError>( call_id );

    res->error = error;

    return res;
}

inline VoipioFatalError *create_fatal_error( uint32 call_id, const std::string & error )
{
    VoipioFatalError *res = create_message_t<VoipioFatalError>( call_id );

    res->error = error;

    return res;
}

NAMESPACE_VOIP_SERVICE_END

#endif  // VOIP_SERVICE_OBJECT_FACTORY_H

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


// $Id: object_factory.h 1272 2014-12-17 18:25:08Z serge $

#ifndef VOIP_SERVICE_OBJECT_FACTORY_H
#define VOIP_SERVICE_OBJECT_FACTORY_H

#include "objects.h"    // VoipioObject...

NAMESPACE_VOIP_SERVICE_START

inline void init_call_id( VoipioCallObject * obj, uint32 call_id )
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

inline VoipioCallDuration *create_call_duration( uint32 call_id, uint32 t )
{
    VoipioCallDuration *res = create_message_t<VoipioCallDuration>( call_id );

    res->t  = t;

    return res;
}

inline VoipioError *create_error( uint32 call_id, const std::string & error )
{
    VoipioError *res = create_message_t<VoipioError>( call_id );

    res->error = error;

    return res;
}

NAMESPACE_VOIP_SERVICE_END

#endif  // VOIP_SERVICE_OBJECT_FACTORY_H

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


// $Revision: 3032 $ $Date:: 2015-12-23 #$ $Author: serge $

#ifndef VOIP_SERVICE_OBJECT_FACTORY_H
#define VOIP_SERVICE_OBJECT_FACTORY_H

#include "objects.h"    // Object...

NAMESPACE_VOIP_SERVICE_START

template <class _T>
void init_job_id( _T * obj, uint32_t job_id )
{
    obj->job_id = job_id;
}

template <class _T>
void init_call_id( _T * obj, uint32_t call_id )
{
    obj->call_id = call_id;
}

template <class _T>
_T *create_message_t( uint32_t call_id )
{
    _T *res = new _T;

    init_call_id( res, call_id );

    return res;
}

inline InitiateCallRequest *create_initiate_call_request( uint32_t job_id, const std::string & party )
{
    InitiateCallRequest *res = new InitiateCallRequest;

    init_job_id( res, job_id );

    res->party      = party;

    return res;
}

inline InitiateCallResponse *create_initiate_call_response( uint32_t job_id, uint32_t call_id, uint32_t status )
{
    InitiateCallResponse *res = new InitiateCallResponse;

    init_job_id( res, job_id );

    res->call_id    = call_id;
    res->status     = status;

    return res;
}

inline DropRequest *create_drop_request( uint32_t job_id, uint32_t call_id )
{
    DropRequest *res = new DropRequest;

    init_job_id( res, job_id );

    res->call_id    = call_id;

    return res;
}

inline DropResponse *create_drop_response( uint32_t job_id )
{
    DropResponse *res = new DropResponse;

    init_job_id( res, job_id );

    return res;
}

inline PlayFileRequest *create_play_file_request( uint32_t job_id, uint32_t call_id, const std::string & filename )
{
    PlayFileRequest *res = new PlayFileRequest;

    init_job_id( res, job_id );

    res->call_id    = call_id;
    res->filename   = filename;

    return res;
}

inline PlayFileResponse *create_play_file_response( uint32_t job_id )
{
    PlayFileResponse *res = new PlayFileResponse;

    init_job_id( res, job_id );

    return res;
}

inline RecordFileRequest *create_record_file_request( uint32_t job_id, uint32_t call_id, const std::string & filename )
{
    RecordFileRequest *res = new RecordFileRequest;

    init_job_id( res, job_id );

    res->call_id    = call_id;
    res->filename   = filename;

    return res;
}


inline ErrorResponse *create_error_response( uint32_t job_id, uint32_t errorcode, const std::string & descr )
{
    ErrorResponse *res = new ErrorResponse;

    init_job_id( res, job_id );

    res->errorcode  = errorcode;
    res->descr      = descr;

    return res;
}

inline RejectResponse *create_reject_response( uint32_t job_id, uint32_t errorcode, const std::string & descr )
{
    RejectResponse *res = new RejectResponse;

    init_job_id( res, job_id );

    res->errorcode  = errorcode;
    res->descr      = descr;

    return res;
}

inline CallDuration *create_call_duration( uint32_t call_id, uint32_t t )
{
    CallDuration *res = create_message_t<CallDuration>( call_id );

    res->t  = t;

    return res;
}

inline Failed *create_failed( uint32_t call_id, Failed::type_e type, uint32_t errorcode = 0, const std::string & descr = std::string() )
{
    Failed *res = create_message_t<Failed>( call_id );

    res->type       = type;
    res->errorcode  = errorcode;
    res->descr      = descr;

    return res;
}

inline ConnectionLost *create_connection_lost( uint32_t call_id, ConnectionLost::type_e type, uint32_t errorcode, const std::string & descr )
{
    ConnectionLost *res = create_message_t<ConnectionLost>( call_id );

    res->type       = type;
    res->errorcode  = errorcode;
    res->descr      = descr;

    return res;
}

NAMESPACE_VOIP_SERVICE_END

#endif  // VOIP_SERVICE_OBJECT_FACTORY_H

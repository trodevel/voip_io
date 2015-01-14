/*

Voip Service objects.

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


// $Id: objects.h 1378 2015-01-13 19:29:50Z serge $

#ifndef VOIP_SERVICE_OBJECTS_H
#define VOIP_SERVICE_OBJECTS_H

#include <string>                   // std::string
#include "../utils/types.h"         // uint32

#include "../servt/i_object.h"      // IObject

#include "namespace_lib.h"          // NAMESPACE_VOIP_SERVICE_START

NAMESPACE_VOIP_SERVICE_START

struct VoipioObject: public servt::IObject
{
};

struct VoipioCallObject: public VoipioObject
{
    uint32          call_id;
};

struct VoipioInitiateCall: public VoipioObject
{
    std::string     party;
};

struct VoipioPlayFile: public VoipioCallObject
{
    std::string     filename;
};

struct VoipioRecordFile: public VoipioCallObject
{
    std::string     filename;
};

struct VoipioDrop: public VoipioCallObject
{
};

// ******************* CALLBACKS *******************

struct VoipioCallbackObject: public VoipioObject
{
};

struct VoipioErrorResponse: public VoipioCallbackObject
{
    uint32 errorcode;
    std::string descr;
};

struct VoipioRejectResponse: public VoipioCallbackObject
{
    uint32 errorcode;
    std::string descr;
};

struct VoipioInitiateCallResponse: public VoipioCallbackObject
{
    uint32 call_id;
    uint32 status;
};


struct VoipioCallbackCallObject: public VoipioCallbackObject
{
    uint32          call_id;
};

struct VoipioDropResponse: public VoipioCallbackCallObject
{
};

struct VoipioDial: public VoipioCallbackCallObject
{
};

struct VoipioRing: public VoipioCallbackCallObject
{
};

struct VoipioConnect: public VoipioCallbackCallObject
{
};

struct VoipioCallDuration: public VoipioCallbackCallObject
{
    uint32 t;
};

struct VoipioCallEnd: public VoipioCallbackCallObject
{
    enum type_e
    {
        NONE,
        CANCELLED,
        FINISHED,
        FAILED_PSTN,
        FAILED,
        REFUSED
    };

    type_e          type;
    uint32          errorcode;
    std::string     descr;
};

struct VoipioPlayStarted: public VoipioCallbackCallObject
{
};

struct VoipioPlayStopped: public VoipioCallbackCallObject
{
};

struct VoipioCallErrorResponse: public VoipioCallbackCallObject
{
    uint32          errorcode;
    std::string     descr;
};

// ******************* WRAPPER for objects not derived from servt::IObject *******************

struct VoipioObjectWrap: public VoipioObject
{
    const void  *ptr;
};


NAMESPACE_VOIP_SERVICE_END

#endif  // VOIP_SERVICE_OBJECTS_H

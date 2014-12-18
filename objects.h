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


// $Id: objects.h 1272 2014-12-17 18:25:08Z serge $

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

struct VoipioInitiateCallResponse: public VoipioCallbackObject
{
    uint32 call_id;
    uint32 status;
};

struct VoipioReady: public VoipioCallbackObject
{
    uint32 errorcode;
};


struct VoipioCallbackCallObject: public VoipioCallbackObject
{
    uint32          call_id;
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
    uint32 errorcode;
};

struct VoipioPlayStarted: public VoipioCallbackCallObject
{
};

struct VoipioPlayStopped: public VoipioCallbackCallObject
{
};

struct VoipioError: public VoipioCallbackCallObject
{
    std::string     error;
};

struct VoipioFatalError: public VoipioCallbackCallObject
{
    std::string     error;
};

NAMESPACE_VOIP_SERVICE_END

#endif  // VOIP_SERVICE_OBJECTS_H

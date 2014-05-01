/*

VOIP service interface.

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

// $Id: i_voip_service.h 473 2014-04-29 17:48:53Z serge $

#ifndef I_VOIP_SERVICE_H
#define I_VOIP_SERVICE_H

#include <string>                   // std::string
#include "../utils/types.h"         // uint32

#include "namespace_voipservice.h"  // NAMESPACE_VOIP_SERVICE_START

NAMESPACE_VOIP_SERVICE_START


class IVoipService
{
public:

public:
    virtual ~IVoipService() {};

    virtual bool is_ready() const                                                               = 0;
    virtual bool initiate_call( const std::string & party, uint32 & call_id, uint32 & status )  = 0;
    virtual bool drop_call( uint32 call_id )                                                    = 0;
    virtual bool set_input_file( uint32 call_id, const std::string & filename )                 = 0;
    virtual bool set_output_file( uint32 call_id, const std::string & filename )                = 0;

    virtual bool shutdown()                                                                     = 0;
};

NAMESPACE_VOIP_SERVICE_END

#endif  // I_VOIP_SERVICE_H

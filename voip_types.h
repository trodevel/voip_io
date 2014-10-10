/*

Constants.

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

// $Id: voip_types.h 1115 2014-10-09 17:19:50Z serge $

#ifndef VOIP_TYPES_H
#define VOIP_TYPES_H

#include "namespace_lib.h"          // NAMESPACE_VOIP_SERVICE_START

NAMESPACE_VOIP_SERVICE_START

enum class errorcode_e
{
    NONE         = 0,
    UNKNOWN,
    WRONG_NUMBER,
    SUBSCRIBER_OFFLINE,
    REJECTED,
    HW_ERROR,
    VOIP_SPECIFIC,
};

NAMESPACE_VOIP_SERVICE_END

#endif  // VOIP_TYPES_H

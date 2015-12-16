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

// $Revision: 2990 $ $Date:: 2015-12-15 #$ $Author: serge $

#ifndef I_VOIP_SERVICE_CALLBACK_H
#define I_VOIP_SERVICE_CALLBACK_H

#include "namespace_lib.h"          // NAMESPACE_VOIP_SERVICE_START

NAMESPACE_VOIP_SERVICE_START

class CallbackObject;

class IVoipServiceCallback
{
public:
    virtual ~IVoipServiceCallback() {};

    virtual void consume( const CallbackObject * req )    = 0;
};

NAMESPACE_VOIP_SERVICE_END

#endif  // I_VOIP_SERVICE_CALLBACK_H

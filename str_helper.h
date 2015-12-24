/*

Voip Service to_string().

Copyright (C) 2015 Sergey Kolevatov

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


// $Revision: 3046 $ $Date:: 2015-12-24 #$ $Author: serge $

#ifndef VOIP_SERVICE_STR_HELPER_H
#define VOIP_SERVICE_STR_HELPER_H

#include <sstream>              // std::ostringstream
#include "objects.h"            // objects

NAMESPACE_VOIP_SERVICE_START

// ******************* GENERICS *******************

inline std::string to_string( const RequestObject & r )
{
    std::ostringstream os;

    os << r.job_id;

    return os.str();
}

inline std::string to_string( const ResponseObject & r )
{
    std::ostringstream os;

    os << r.job_id;

    return os.str();
}

// ******************* ERRORS *******************

inline std::string to_string( const ErrorResponse & r )
{
    std::ostringstream os;

    os << r.job_id << " " << r.errorcode << " '" << r.descr <<"'";

    return os.str();
}

inline std::string to_string( const RejectResponse & r )
{
    std::ostringstream os;

    os << r.job_id << " " << r.errorcode << " '" << r.descr <<"'";

    return os.str();
}

// ******************* CALL *******************

inline std::string to_string( const InitiateCallRequest & r )
{
    std::ostringstream os;

    os << r.job_id << " " << r.party;

    return os.str();
}

inline std::string to_string( const InitiateCallResponse & r )
{
    std::ostringstream os;

    os << r.job_id << " " << r.call_id << " " << r.status;

    return os.str();
}

inline std::string to_string( const DropRequest & r )
{
    std::ostringstream os;

    os << r.job_id << " " << r.call_id;

    return os.str();
}

inline std::string to_string( const DropResponse & r )
{
    std::ostringstream os;

    os << r.job_id;

    return os.str();
}

// ******************* IN-CALL REQUESTS *******************

inline std::string to_string( const PlayFileRequest & r )
{
    std::ostringstream os;

    os << r.job_id << " " << r.call_id << " '" << r.filename <<"'";

    return os.str();
}

inline std::string to_string( const PlayFileResponse & r )
{
    std::ostringstream os;

    os << r.job_id;

    return os.str();
}

inline std::string to_string( const RecordFileRequest & r )
{
    std::ostringstream os;

    os << r.job_id << " " << r.call_id << " '" << r.filename <<"'";

    return os.str();
}

inline std::string to_string( const RecordFileResponse & r )
{
    std::ostringstream os;

    os << r.job_id;

    return os.str();
}

NAMESPACE_VOIP_SERVICE_END

#endif  // VOIP_SERVICE_STR_HELPER_H

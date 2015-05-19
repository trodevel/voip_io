/*

Dummy example.

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

// $Revision: 1754 $ $Date:: 2015-05-19 #$ $Author: serge $

#include <iostream>         // cout
#include <typeinfo>

#include "i_voip_service_callback.h"    // IVoipServiceCallback
#include "i_voip_service.h"             // IVoipService
#include "voip_service.h"               // VoipService
#include "object_factory.h"             // voip_service::create_message_t

#include "../skype_io/skype_io.h"       // SkypeIo
#include "../utils/dummy_logger.h"      // dummy_log_set_log_level

class Callback: virtual public voip_service::IVoipServiceCallback
{
public:
    Callback():
        call_id_( 0 )
    {
    }

    // interface IVoipServiceCallback
    void consume( const voip_service::VoipioCallbackObject * req )
    {
        if( typeid( *req ) == typeid( voip_service::VoipioInitiateCallResponse ) )
        {
            // handle( dynamic_cast< const voip_service::VoipioInitiateCallResponse *>( req ) );

            call_id_    = dynamic_cast< const voip_service::VoipioInitiateCallResponse *>( req )->call_id;
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioErrorResponse ) )
        {
            // handle( dynamic_cast< const voip_service::VoipioErrorResponse *>( req ) );
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioDropResponse ) )
        {
            // handle( dynamic_cast< const voip_service::VoipioDropResponse *>( req ) );
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioCallErrorResponse ) )
        {
            // handle( dynamic_cast< const voip_service::VoipioCallErrorResponse *>( req ) );
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioCallEnd ) )
        {
            // handle( dynamic_cast< const voip_service::VoipioCallEnd *>( req ) );
            call_id_    = 0;
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioDial ) )
        {
            // handle( dynamic_cast< const voip_service::VoipioDial *>( req ) );
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioRing ) )
        {
            // handle( dynamic_cast< const voip_service::VoipioRing *>( req ) );
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioConnect ) )
        {
            // handle( dynamic_cast< const voip_service::VoipioConnect *>( req ) );
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioCallDuration ) )
        {
            // handle( dynamic_cast< const voip_service::VoipioCallDuration *>( req ) );
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioPlayStarted ) )
        {
            // handle( dynamic_cast< const voip_service::VoipioPlayStarted *>( req ) );
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioPlayStopped ) )
        {
            // handle( dynamic_cast< const voip_service::VoipioPlayStopped *>( req ) );
        }
        else
        {
            std::cout << "got unknown event" << std::endl;
        }

        delete req;
    }

private:

    uint32_t    call_id_;

};

void control_thread( voip_service::VoipService * voips )
{
    std::cout << "type exit or quit to quit: " << std::endl;

    std::string input;

    while( true )
    {
        std::cout << "your command: ";

        std::getline( std::cin, input );

        std::cout << "command: " << input << std::endl;

        if( input == "exit" || input == "quit" )
            break;

        /*
        bool b = voips->consume( input );

        if( b == false )
        {
            std::cout << "ERROR: cannot process command '" << input << "'" << std::endl;
        }
        */

    };

    std::cout << "exiting ..." << std::endl;

    //voips->shutdown();
}



int main( int argc, char **argv )
{
    dummy_logger::set_log_level( log_levels_log4j::TRACE );

    skype_wrap::SkypeIo sio;
    voip_service::VoipService  voips;

    bool b = sio.init();

    if( !b )
    {
        std::cout << "cannot initialize SkypeIo - " << sio.get_error_msg() << std::endl;
        return 0;
    }

    b = voips.init( & sio );
    {
        std::cout << "cannot initialize VoipService" << std::endl;
        return 0;
    }

    Callback test;

    voips.register_callback( &test );

    std::thread t( std::bind( &control_thread, &voips ) );

    t.join();

    std::cout << "Done! =)" << std::endl;

    return 0;
}

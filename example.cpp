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

// $Revision: 1766 $ $Date:: 2015-05-20 #$ $Author: serge $

#include <iostream>         // cout
#include <typeinfo>
#include <sstream>          // stringstream

#include "i_voip_service_callback.h"    // IVoipServiceCallback
#include "i_voip_service.h"             // IVoipService
#include "voip_service.h"               // VoipService
#include "object_factory.h"             // voip_service::create_message_t

#include "../skype_io/skype_io.h"       // SkypeIo
#include "../utils/dummy_logger.h"      // dummy_log_set_log_level

class Callback: virtual public voip_service::IVoipServiceCallback
{
public:
    Callback()
    {
    }

    // interface IVoipServiceCallback
    void consume( const voip_service::VoipioCallbackObject * req )
    {
        if( typeid( *req ) == typeid( voip_service::VoipioInitiateCallResponse ) )
        {
            std::cout << "got VoipioInitiateCallResponse "
                    << dynamic_cast< const voip_service::VoipioInitiateCallResponse *>( req )->call_id
                    << std::endl;
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioErrorResponse ) )
        {
            std::cout << "got VoipioErrorResponse "
                    << dynamic_cast< const voip_service::VoipioErrorResponse *>( req )->descr
                    << std::endl;
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioDropResponse ) )
        {
            std::cout << "got VoipioDropResponse "
                    << dynamic_cast< const voip_service::VoipioDropResponse *>( req )->call_id
                    << std::endl;
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioCallErrorResponse ) )
        {
            std::cout << "got VoipioCallErrorResponse "
                    << dynamic_cast< const voip_service::VoipioCallErrorResponse *>( req )->call_id
                    << std::endl;
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioCallEnd ) )
        {
            std::cout << "got VoipioCallEnd "
                    << dynamic_cast< const voip_service::VoipioCallEnd *>( req )->call_id
                    << std::endl;
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioDial ) )
        {
            std::cout << "got VoipioDial "
                    << dynamic_cast< const voip_service::VoipioDial *>( req )->call_id
                    << std::endl;
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioRing ) )
        {
            std::cout << "got VoipioRing "
                    << dynamic_cast< const voip_service::VoipioRing *>( req )->call_id
                    << std::endl;
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioConnect ) )
        {
            std::cout << "got VoipioConnect "
                    << dynamic_cast< const voip_service::VoipioConnect *>( req )->call_id
                    << std::endl;
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioCallDuration ) )
        {
            std::cout << "got VoipioCallDuration "
                    << dynamic_cast< const voip_service::VoipioCallDuration *>( req )->call_id
                    << std::endl;
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioPlayStarted ) )
        {
            std::cout << "got VoipioPlayStarted "
                    << dynamic_cast< const voip_service::VoipioPlayStarted *>( req )->call_id
                    << std::endl;
        }
        else if( typeid( *req ) == typeid( voip_service::VoipioPlayStopped ) )
        {
            std::cout << "got VoipioPlayStopped "
                    << dynamic_cast< const voip_service::VoipioPlayStopped *>( req )->call_id
                    << std::endl;
        }
        else
        {
            std::cout << "got unknown event" << std::endl;
        }

        delete req;
    }

};

void control_thread( voip_service::VoipService * voips )
{
    std::cout << "type exit or quit to quit: " << std::endl;
    std::cout << "call <party>" << std::endl;
    std::cout << "drop <call_id>" << std::endl;
    std::cout << "play <call_id> <file>" << std::endl;
    std::cout << "rec  <call_id> <file>" << std::endl;

    std::string input;

    while( true )
    {
        std::cout << "your command: ";

        std::getline( std::cin, input );

        std::cout << "your input: " << input << std::endl;

        std::string cmd;

        std::stringstream stream( input );
        if( stream >> cmd )
        {
            if( cmd == "exit" || cmd == "quit" )
            {
                break;
            }
            else if( cmd == "call" )
            {
                std::string s;
                stream >> s;

                voips->consume( voip_service::create_initiate_call_request( s ) );
            }
            else if( cmd == "drop" )
            {
                int call_id;
                stream >> call_id;

                voips->consume( voip_service::create_message_t<voip_service::VoipioDrop>( call_id ) );
            }
            else if( cmd == "play" )
            {
                int call_id;
                std::string filename;
                stream >> call_id >> filename;

                voips->consume( voip_service::create_play_file( call_id, filename ) );
            }
            else if( cmd == "rec" )
            {
                int call_id;
                std::string filename;
                stream >> call_id >> filename;

                voips->consume( voip_service::create_record_file( call_id, filename ) );
            }
            else
                std::cout << "ERROR: unknown command '" << cmd << "'" << std::endl;
        }
        else
        {
            std::cout << "ERROR: cannot read command" << std::endl;
        }

    };

    std::cout << "exiting ..." << std::endl;
}



int main( int argc, char **argv )
{
    dummy_logger::set_log_level( log_levels_log4j::TRACE );

    skype_wrap::SkypeIo sio;
    voip_service::VoipService  voips;

    {
        bool b = voips.init( & sio );
        if( !b )
        {
            std::cout << "cannot initialize VoipService" << std::endl;
            return 0;
        }
    }

    {
        bool b = sio.init();

        if( !b )
        {
            std::cout << "cannot initialize SkypeIo - " << sio.get_error_msg() << std::endl;
            return 0;
        }

        sio.register_callback( & voips );
    }

    Callback test;

    voips.register_callback( &test );
    voips.start();

    std::thread t( std::bind( &control_thread, &voips ) );

    t.join();


    sio.shutdown();

    voips.VoipService::shutdown();

    std::cout << "Done! =)" << std::endl;

    return 0;
}

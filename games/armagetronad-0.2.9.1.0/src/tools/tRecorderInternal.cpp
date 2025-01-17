/*

*************************************************************************

ArmageTron -- Just another Tron Lightcycle Game in 3D.
Copyright (C) 2000  Manuel Moos (manuel@moosnet.de)

**************************************************************************

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

***************************************************************************

*/

// declaration
#ifndef     TRECORDERINTERNAL_H_INCLUDED
#include    "tRecorderInternal.h"
#endif

#include    <ctype.h>

#ifdef      HAVE_STDLIB
#include    <stdlib.h>
#endif

#include    <fstream>

#include    "tCommandLine.h"
#include    "tConsole.h"
#include    "tDirectories.h"

#undef  INLINE_DEF
#define INLINE_DEF

// static pointer to currently running recording
// !todo: make this a context item
tRecording * tRecording::currentRecording_ = 0;
tPlayback * tPlayback::currentPlayback_ = 0;

// ******************************************************************************************
// *
// * ~tRecording
// *
// ******************************************************************************************
//!
//!
// ******************************************************************************************

tRecording::~tRecording()
{
    // clear current recording
    currentRecording_ = 0;
}

// ******************************************************************************************
// *
// *    tRecording
// *
// ******************************************************************************************
//!
//!
// ******************************************************************************************

tRecording::tRecording()
{
    tASSERT( !currentRecording_ );

    std::cout << "Recording!\n";

    // set current recording
    currentRecording_ = this;
}

// ******************************************************************************************
// *
// *    Stop
// *
// ******************************************************************************************
//!
//!
// ******************************************************************************************

void tRecording::Stop()
{
    currentRecording_ = NULL;
}

// ******************************************************************************************
// *
// * tRecording
// *
// ******************************************************************************************
//!
//!       @param  other   the source to copy from
//!
// ******************************************************************************************

tRecording::tRecording( tRecording const & other )
{
    tASSERT(false);
}

// ******************************************************************************************
// *
// *  operator =
// *
// ******************************************************************************************
//!
//!       @param  other   the source to copy from
//!     @return         a reference to this
//!
// ******************************************************************************************

tRecording & tRecording::operator= ( tRecording const & other )
{
    tASSERT(false);

    return *this;
}

// ******************************************************************************************
// *
// *    BeginSection
// *
// ******************************************************************************************
//!
//!     @param  name    the name of the new section
//!
// ******************************************************************************************

void tRecording::BeginSection( char const * name )
{
    DoGetStream() << "\n" << name;
}

// ******************************************************************************************
// *
// *    DoGetStream
// *
// ******************************************************************************************
//!
//!     @return     the stream to write to
//!
// ******************************************************************************************

/*
std::ostream & tRecording::DoGetStream( void ) const
{
    tASSERT(0); // pure virtual

    return std::ostream();
}
*/

// ******************************************************************************************
// *
// * ~tPlayback
// *
// ******************************************************************************************
//!
//!
// ******************************************************************************************

tPlayback::~tPlayback()
{
    // clear current Playback
    currentPlayback_ = 0;
}

// ******************************************************************************************
// *
// *    tPlayback
// *
// ******************************************************************************************
//!
//!
// ******************************************************************************************

tPlayback::tPlayback()
{
    tASSERT( !currentPlayback_ );

    // set current Playback
    currentPlayback_ = this;
}

// ******************************************************************************************
// *
// * tPlayback
// *
// ******************************************************************************************
//!
//!       @param  other   the source to copy from
//!
// ******************************************************************************************

tPlayback::tPlayback( tPlayback const & other )
{
    tASSERT(false);
}

// ******************************************************************************************
// *
// *  operator =
// *
// ******************************************************************************************
//!
//!       @param  other   the source to copy from
//!     @return         a reference to this
//!
// ******************************************************************************************

tPlayback & tPlayback::operator= ( tPlayback const & other )
{
    tASSERT(false);

    return *this;
}
// ******************************************************************************************
// *
// *    GetNextSection
// *
// ******************************************************************************************
//!
//!     @return     the name of the next section
//!
// ******************************************************************************************

const std::string & tPlayback::GetNextSection( void ) const
{
    return nextSection_;
}

/*
void EatWhite( std::istream & stream )
{
    std::ws( stream );
    return;

    // eat whitespace (read to end of line)
    //int c= ' ';
    //while ( isblank(c) )
    //    c = stream.get();
    //if ( c != '\n' )
    //    stream.unget();
}
*/

// ******************************************************************************************
// *
// *    AdvanceSection
// *
// ******************************************************************************************
//!
//!
// ******************************************************************************************

void tPlayback::AdvanceSection( bool skipLines )
{
    std::istream& stream = DoGetStream();

    if(skipLines)
    {
        std::string line;
        std::getline(stream, line);
    }
    std::ws( stream );
    stream >> nextSection_;
    std::ws( stream );

    // memorize if end marking was seen
    static bool end = false;
    if ( nextSection_ == "END" )
        end = true;

    if ( !stream.good() )
    {
        if ( !end )
            con << "Recording ends abruptly here, prepare for a crash!\n";

        // stop playing back
        nextSection_ = "EOF";
        currentPlayback_ = NULL;
    }
}

class tRecordingCommandLineAnalyzer: public tCommandLineAnalyzer
{
private:
    bool DoAnalyze( tCommandLineParser & parser, int pass ) override
    {
        switch(pass)
        {
            // pass 0: only accept strict options
            case 0:
            {
                tString filename;
                if ( parser.GetOption( filename, "--record" ) )
                {
                    // start a recorder
                    static tRecorderImp< tRecording, std::ofstream > recorder( static_cast< char const * >( filename ) );
                    return true;
                }

                if ( parser.GetOption( filename, "--playback" ) )
                {
                    // start a playback
                    static tRecorderImp< tPlayback, std::ifstream > recorder( static_cast< char const * >( filename ) );
                    recorder.InitPlayback();
                    return true;
                }
            }
            break;
            // pass 1: anything may be a recording to play back
            case 1:
            {
                auto *filename = parser.Current();
                if(tDirectories::FileMatchesWildcard(filename, "*.aarec"))
                {
                    static tRecorderImp< tPlayback, std::ifstream > recorder( parser.Current() );
                    recorder.InitPlayback();
                    parser.Advance();
                    return true;
                }
            }
            break;
        }


        return false;
    }

    void DoHelp( std::ostream & s ) override
    {                                      //
        s << "--record <filename>          : creates a DEBUG recording while running\n";
        s << "--playback <filename>        : plays back a DEBUG recording\n";
        s << "<filename.aarec>             : short form, plays back a DEBUG recording, recording must end in .aarec\n";
    }
};

static tRecordingCommandLineAnalyzer analyzer;

// Copyright (c) 2002 - 2018, Kit10 Studios LLC
// All Rights Reserved

#include <mewos/OutputDebugStringLogListener.h>

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#ifdef GetCommandLine
#undef GetCommandLine
#endif // GetCommandLine
#ifdef GetObject
#undef GetObject
#endif
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

using namespace mewos;

OutputDebugStringLogListener::OutputDebugStringLogListener()
{
}

OutputDebugStringLogListener::~OutputDebugStringLogListener()
{
}


void OutputDebugStringLogListener::LogEvent( const kit::LogEvent* event )
{
	OutputDebugStringA( (event->text + "\n").c_str() );
}

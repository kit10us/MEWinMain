// Copyright (c) 2002 - 2018, Kit10 Studios LLC
// All Rights Reserved

#include <mewos/OutputDebugStringLogListener.h>

#include <port/win/Windows.h>

using namespace mewos;

OutputDebugStringLogListener::OutputDebugStringLogListener()
{
}

OutputDebugStringLogListener::~OutputDebugStringLogListener()
{
}


bool OutputDebugStringLogListener::LogEvent( const kit::debug::LogEvent* event )
{
	OutputDebugStringA( (event->text + "\n").c_str() );
	return true;
}

unify::Path OutputDebugStringLogListener::GetPath() const
{
	return {};
}

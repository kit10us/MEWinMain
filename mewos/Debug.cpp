// Copyright (c) 2003 - 2011, Kit10 Studios LLC
// All Rights Reserved

#include <mewos/Debug.h>
#include <mewos/ErrorHandler.h>

using namespace mewos;


Debug::Debug( WindowsOS * os )
	: m_os{ os }
{
	SetErrorHandler( me::debug::IErrorHandler::ptr{ new ErrorHandler( os ) } );
}

Debug::~Debug()	
{
}

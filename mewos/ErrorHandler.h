// Copyright (c) 2003 - 2011, Kit10 Studios LLC
// All Rights Reserved

#pragma once

#include <mewos/WindowsOS.h>
#include <me/debug/IErrorHandler.h>

namespace mewos
{
	class ErrorHandler : public me::debug::IErrorHandler
	{
		mewos::WindowsOS* m_os;

	public:
		ErrorHandler( mewos::WindowsOS* os );

		me::debug::ReportErrorResult ReportError( me::debug::ErrorLevel level, std::string source, std::string error, bool canContinue, bool canRetry ) override;
	};
}
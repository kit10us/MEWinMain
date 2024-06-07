// Copyright (c) 2002 - 2018, Kit10 Studios LLC
// All Rights Reserved

#include <kit/debug/ILogger.h>
#include <kit/debug/ILogListener.h>

namespace mewos
{
	/// <summary>
	/// Customer logger that logs to the Visual Studio Output window.
	/// </summary>
	class OutputDebugStringLogListener : public kit::debug::ILogListener
	{
	public:
		OutputDebugStringLogListener();
		~OutputDebugStringLogListener();

		bool LogEvent( const kit::debug::LogEvent* event ) override;
		unify::Path GetPath() const override;
	};
}
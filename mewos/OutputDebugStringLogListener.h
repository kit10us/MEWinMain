// Copyright (c) 2002 - 2018, Kit10 Studios LLC
// All Rights Reserved

#include <kit/ILogger.h>
#include <kit/ILogListener.h>

namespace mewos
{
	/// <summary>
	/// Customer logger that logs to the Visual Studio Output window.
	/// </summary>
	class OutputDebugStringLogListener : public kit::ILogListener
	{
	public:
		OutputDebugStringLogListener();
		~OutputDebugStringLogListener();

		void LogEvent( const kit::LogEvent* event ) override;
	};
}
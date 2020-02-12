// Copyright (c) 2003 - 2011, Kit10 Studios LLC
// All Rights Reserved

#pragma once

#include <mewos/WindowsOS.h>
#include <me/debug/DefaultDebug.h>
#include <unify/Path.h>
#include <list>
#include <string>
#include <chrono>
#include <map>

namespace mewos
{
	class Debug : public me::debug::DefaultDebug
	{
		WindowsOS * m_os;
		//size_t m_flushedLogLines;
	public:
		Debug( WindowsOS * os );
		virtual ~Debug();

		//void LogSectionLine( std::string section, std::string line ) override;

		/// <summary>
		/// Flush log lines by outputting lines logged, and ensuring our logged lines are consumed.
		/// </summary>
		void Flush();
	};
}
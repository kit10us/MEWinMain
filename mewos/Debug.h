// Copyright (c) 2003 - 2011, Kit10 Studios LLC
// All Rights Reserved

#pragma once

#include <mewos/WindowsOS.h>
#include <me/debug/DefaultDebug.h>

namespace mewos
{
	/// <summary>
	/// Specialize the default debug class to support Windows features.
	/// </summary>
	class Debug : public me::debug::DefaultDebug
	{
		WindowsOS * m_os;
	public:
		Debug( WindowsOS * os );
		~Debug() override;
	};
}
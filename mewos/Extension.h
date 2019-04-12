// Copyright (c) 2002 - 2018, Kit10 Studios LLC
// All Rights Reserved

#pragma once

#include <me/game/IGame.h>
#include <me/os/IExtension.h>
#include <qxml/Element.h>
#include <unify/Path.h>

namespace mewos
{
	class Extension : public me::os::IExtension
	{
	public:
		Extension( me::game::IGame * gameInstance, unify::Path source, const qxml::Element * element );
		virtual ~Extension();

	public: // me::os::IExtension

	private:
		unify::Path m_source;
		void* m_moduleHandle;
	};
}
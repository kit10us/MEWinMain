// Copyright (c) 2002 - 2018, Kit10 Studios LLC
// All Rights Reserved

#pragma once

#include <me/game/IGame.h>
#include <me/os/IExtension.h>
#include <qxml/Element.h>
#include <unify/Path.h>
#include <unify/Result.h>

namespace mewos
{
	class Extension : public me::os::IExtension
	{
		unify::Path m_source;
		void* m_moduleHandle;

	public:
<<<<<<< HEAD
		Extension(me::game::IGame* gameInstance, unify::Path source, const qxml::Element* element);
=======
		Extension();
>>>>>>> temp
		virtual ~Extension();

		unify::Result<> Load(me::game::IGame* gameInstance, unify::Path source, const qxml::Element* element);

		void Unload();

	public: // me::os::IExtension

	};
}
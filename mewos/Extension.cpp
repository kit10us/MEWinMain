// Copyright (c) 2002 - 2018, Kit10 Studios LLC
// All Rights Reserved

#include <mewos/Extension.h>
#include <me/exception/FileNotFound.h>

#include <port/win/general.h>

using namespace mewos;

typedef bool( __cdecl *LoaderFunction )(me::game::IGame *, const qxml::Element * element);

Extension::Extension()
	: m_moduleHandle{}
{
}

unify::Result<> Extension::Load(me::game::IGame* gameInstance, unify::Path source, const qxml::Element* element)
{
	using namespace me;
	auto block{ gameInstance->Debug()->GetLogger()->CreateBlock( "Extension \"" + source.Filename() + "\" loading" ) };

	auto debug = gameInstance->Debug();

	m_source = source;
	if( !m_source.Exists() )
	{
		return unify::Failure("File not found " + m_source.ToString() );
	}

	block->Log( "Loading library module." );
	m_moduleHandle = LoadLibraryA( m_source.ToString().c_str() );
	if( !m_moduleHandle )
	{
		uint32_t errorCode = GetLastError();
		if( errorCode == ERROR_MOD_NOT_FOUND )
		{
			return unify::Failure("Extension \"" + m_source.ToString() + "\" loaded, however, a failure occured due to likely missing dependency (missing another DLL)!" );
		}
		else
		{
			return unify::Failure("Extension \"" + m_source.ToString() + "\" loaded, however, a failure occured (error code: " + *unify::ToString( errorCode ) + ")!" );
		}
	}

	LoaderFunction loader{};
	{
		block->Log( "Getting MELoader." );
		loader = (LoaderFunction)GetProcAddress( (HMODULE)m_moduleHandle, "MELoader" );

		if( !loader )
		{
			FreeLibrary( (HMODULE)m_moduleHandle );
			m_moduleHandle = 0;
			debug->ReportError(me::debug::ErrorLevel::Failure, "Extension, \"" + m_source.ToString() + "\" loaded, however MELoader not found!" );
		}

		block->Log( "Executing loader" );
		loader( gameInstance, element );
	}
	return {};
}

void Extension::Unload()
{
	if (m_moduleHandle)
	{
		FreeLibrary( (HMODULE)m_moduleHandle );
		m_moduleHandle = 0;
	}
}

Extension::~Extension()
{
	Unload();
}
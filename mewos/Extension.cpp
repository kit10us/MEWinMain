// Copyright (c) 2002 - 2018, Kit10 Studios LLC
// All Rights Reserved

#include <mewos/Extension.h>
#include <me/exception/FileNotFound.h>

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

#ifdef GetObject
#undef GetObject
#endif
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

using namespace mewos;

typedef bool( __cdecl *LoaderFunction )(me::game::IGame *, const qxml::Element * element);

Extension::Extension( me::game::IGame* gameInstance, unify::Path source, const qxml::Element* element )
	: m_moduleHandle{}
	, m_block{ gameInstance->Debug()->GetLogger()->CreateBlock( "Extension \"" + source.Filename() + "\"" ) }
{
	using namespace me;
	auto debug = gameInstance->Debug();
	auto block{ m_block->SubBlock( "ctor" ) };

	m_source = source;
	debug->Try( [&]
	{
		if( !m_source.Exists() )
		{
			throw exception::FileNotFound( m_source );
		}
	}, debug::ErrorLevel::Extension, false, true );

	debug->Try( [&]
		{
			block->Log( "Loading library module." );
			m_moduleHandle = LoadLibraryA( m_source.ToString().c_str() );
			if( !m_moduleHandle )
			{
				DWORD errorCode = GetLastError();
				if( errorCode == ERROR_MOD_NOT_FOUND )
				{
					throw unify::Exception( "Extension \"" + m_source.ToString() + "\" loaded, however, a failure occured due to likely missing dependency (missing another DLL)!" );
				}
				else
				{
					throw unify::Exception( "Extension \"" + m_source.ToString() + "\" loaded, however, a failure occured (error code: " + unify::Cast< std::string >( errorCode ) + ")!" );
				}
			}
		}, debug::ErrorLevel::Extension, false, true );

	LoaderFunction loader{};
	{
		block->Log( "Getting MELoader." );
		loader = (LoaderFunction)GetProcAddress( (HMODULE)m_moduleHandle, "MELoader" );

		debug->Try( [&]
		{
			if( !loader )
			{
				FreeLibrary( (HMODULE)m_moduleHandle );
				m_moduleHandle = 0;
				throw unify::Exception( "Extension, \"" + m_source.ToString() + "\" loaded, however MELoader not found!" );
			}
		}, debug::ErrorLevel::Extension, false, false );

		block->Log( "Executing loader" );
		debug->Try( [&]
		{
			loader( gameInstance, element );
		}, debug::ErrorLevel::Extension, false, false );
	}
}

Extension::~Extension()
{
	FreeLibrary( (HMODULE)m_moduleHandle );
	m_moduleHandle = 0;
}
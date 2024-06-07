// Copyright (c) 2002 - 2018, Kit10 Studios LLC
// All Rights Reserved

#include <mewos/Extension.h>
#include <me/exception/FileNotFound.h>

#include <port/win/Windows.h>

using namespace mewos;

typedef bool( __cdecl *LoaderFunction )(me::game::IGame *, const qxml::Element * element);

Extension::Extension( me::game::IGame* gameInstance, unify::Path source, const qxml::Element* element, unify::Result::ptr result)
	: m_moduleHandle{}
	, m_block{ gameInstance->Debug()->GetLogger()->CreateBlock( "Extension \"" + source.Filename() + "\"" ) }
{
	using namespace me;
	using Result = unify::Result;


	auto debug = gameInstance->Debug();
	auto block{ m_block->SubBlock( "ctor" ) };

	m_source = source;
	if( !m_source.Exists() )
	{
		result = Result::FAILED("File not found " + m_source.ToString() );
	}

	block->Log( "Loading library module." );
	m_moduleHandle = LoadLibraryA( m_source.ToString().c_str() );
	if( !m_moduleHandle )
	{
		DWORD errorCode = GetLastError();
		if( errorCode == ERROR_MOD_NOT_FOUND )
		{
			result = Result::FAILED("Extension \"" + m_source.ToString() + "\" loaded, however, a failure occured due to likely missing dependency (missing another DLL)!" );
		}
		else
		{
			result = Result::FAILED("Extension \"" + m_source.ToString() + "\" loaded, however, a failure occured (error code: " + unify::Cast< std::string >( errorCode ) + ")!" );
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
}

Extension::~Extension()
{
	FreeLibrary( (HMODULE)m_moduleHandle );
	m_moduleHandle = 0;
}
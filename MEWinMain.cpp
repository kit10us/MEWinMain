// Copyright (c) 2002 - 2018, Kit10 Studios LLC
// All Rights Reserved

#include <mewos/WindowsOS.h>
#include <mewos/OutputDebugStringLogListener.h>
#include <me/game/Game.h>
#include <me/debug/IDebug.h>
#include <me/exception/Handled.h>

#include <WndProc.h>
#include <shellapi.h>

extern "C" LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

void Deleter( mewos::WindowsOS * factory )
{
	delete factory;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow )
{
	using namespace mewos;
	MSG msg;

	me::game::IGame * gameInstance;
	me::os::IOS::ptr os;
	std::shared_ptr< OutputDebugStringLogListener > outputDebugStringLogListener;

	try
	{

		unify::Path runPath;
		{
			char buffer[MAX_PATH];
			GetCurrentDirectoryA( MAX_PATH, buffer );
			runPath = unify::Path( std::string( buffer ) + "/" );
		}

		unify::Path programPath;
		{
			using namespace std;
			char buffer[MAX_PATH];
			GetModuleFileNameA( NULL, buffer, MAX_PATH );
			programPath = unify::Path( buffer );
			programPath.Normalize();
		}


		std::vector< std::string > arguments;
		{
			std::string cmdLine = lpszCmdLine;
			size_t l = 0;
			size_t r = 0;
			bool inQuote = false;
			std::string working;

			for( size_t l = 0, r = 0; r <= cmdLine.length(); ++r )
			{
				if( !inQuote && (r == cmdLine.length() || cmdLine.at( r ) == ' ') )
				{
					if( l != r )
					{
						working += cmdLine.substr( l, r - l );
					}
					if( working.empty() == false )
					{
						arguments.push_back( working );
						working.clear();
					}
					l = r + 1;
				}
				else if( cmdLine.at( r ) == '\"' )
				{
					// Include partial string...
					working += cmdLine.substr( l, r - l );
					l = r + 1; // One past the double quote.
					inQuote = !inQuote;
				}
			}
		}

		me::os::OSParameters osParameters( runPath, programPath, arguments );
		osParameters.hInstance = hInstance;
		osParameters.hPrevInstance = hPrevInstance;
		osParameters.nCmdShow = nCmdShow;
		osParameters.wndProc = WndProc;

		gameInstance = GetGameInstance();
		os.reset( new mewos::WindowsOS( gameInstance, osParameters ) );
		os->Debug()->GetLogger()->AttachListener( kit::debug::ILogListener::ptr( new OutputDebugStringLogListener ) );
	}
	catch( std::exception exception )
	{
		// NOTE: Our goal is to never hit hear in release.
        OutputDebugStringA( "[" );
        OutputDebugStringA( "Mercury Failure: " );
        OutputDebugStringA( exception.what() );
        OutputDebugStringA( "]\n" );
		MessageBoxA( 0, exception.what(), "Mercury Failure", MB_ICONEXCLAMATION );
		return -1;
	}

	enum class Stage {
		Initializing,
		Running,
		Exiting
	} stage{ Stage::Initializing };

	while( stage != Stage::Exiting )
	{
		bool allowRetry{ false };
		//try
		{
			if( stage == Stage::Initializing )
			{
				allowRetry = true;
				gameInstance->Initialize( os );
				gameInstance->Debug()->GetLogger()->Log( "After exiting init." );
				stage = Stage::Running; 
			}

			if( stage == Stage::Running )
			{
				allowRetry = false;
				while( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) == 1 )
				{
					if( msg.message == WM_QUIT )
					{
						break;
					}
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}

				gameInstance->Tick();
				if( gameInstance->IsQuitting() )
				{
					break;
				}

				gameInstance->Draw();
			}
		}
		/*
		catch( me::exception::Handled & )
		{
			return -1;
		}
		catch( std::exception exception )
		{
			gameInstance->Debug()->ReportError( me::debug::ErrorLevel::Engine, exception.what(), false, false );
			return -1;
		}
		catch( ... )
		{
			gameInstance->Debug()->ReportError( me::debug::ErrorLevel::Engine, "Unknown exception", false, false );
			return -1;
		}
		*/
	}

 
    return (int)msg.wParam; 
} 
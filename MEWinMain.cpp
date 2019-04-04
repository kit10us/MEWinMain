// Copyright (c) 2002 - 2018, Kit10 Studios LLC
// All Rights Reserved

#include <mewos/WindowsOS.h>
#include <me/game/Game.h>
#include <me/debug/IDebug.h>
#include <me/exception/Handled.h>
#include <WndProc.h>

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#ifdef GetCommandLine
#undef GetCommandLine
#endif // GetCommandLine
#ifdef GetObject
#undef GetObject
#endif
#ifdef max
#undef max
#endif
#ifdef min
#undef min	
#endif

#include <shellapi.h>

extern "C" LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow )
{
	MSG msg;

	static me::game::IGame * gameInstance;

	try
	{
		gameInstance = GetGameInstance();
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
	//osParameters.hWnd = hWnd;
	osParameters.hInstance = hInstance;
	osParameters.hPrevInstance = hPrevInstance;
	osParameters.nCmdShow = nCmdShow;
	osParameters.wndProc = WndProc;
	

	enum class Stage {
		Initializing,
		Running,
		Exiting
	} stage{ Stage::Initializing };

	while( stage != Stage::Exiting )
	{
		bool allowRetry{ false };
		try
		{
			if( stage == Stage::Initializing )
			{
				allowRetry = true;
				gameInstance->Initialize( osParameters );
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
		catch( me::exception::Handled & )
		{
			return -1;
		}
		catch( std::exception exception )
		{
			gameInstance->Debug()->ReportError( me::ErrorLevel::Engine, "MEWinMain", exception.what(), false, false );
			return -1;
		}
		catch( ... )
		{
			gameInstance->Debug()->ReportError( me::ErrorLevel::Engine, "MEWinMain", "Unknown exception", false, false );
			return -1;
		}
	}

 
    return msg.wParam; 
} 
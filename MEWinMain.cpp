// Copyright (c) 2002 - 2018, Evil Quail LLC
// All Rights Reserved

#include <mewos/WindowsOS.h>
#include <me/game/Game.h>
#include <me/debug/IDebug.h>
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

	me::os::OSParameters osParameters;
	osParameters.hInstance = hInstance;
	osParameters.hPrevInstance = hPrevInstance;
	osParameters.cmdLine = lpszCmdLine;
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
		catch (std::exception exception)
		{
			return -1;
		}
	}

 
    return msg.wParam; 
} 
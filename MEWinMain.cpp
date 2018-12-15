// Copyright (c) 2002 - 2018, Evil Quail LLC
// All Rights Reserved

#include <mewos/WindowsOS.h>
#include <me/game/Game.h>
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


#define ID_EDIT_MESSAGE		100
#define ID_BUTTON_ABORT		101
#define ID_BUTTON_RETRY		102
#define ID_BUTTON_IGNORE	103

int s_failureWindowResult {};

extern "C"
LRESULT CALLBACK WndProcFailureWindow( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message)
	{
	case WM_CREATE:
	{
		HINSTANCE hInstace = (HINSTANCE)GetWindowLong( hWnd, GWL_HINSTANCE );

		const int padding = 5;
		RECT parentRect {};
		GetClientRect( hWnd, &parentRect );

		int childWidth = parentRect.right - padding * 2;
		int top = padding;
		int buttonHeight = 25;
		int buttonWidth = (childWidth / 3) - padding * 2;
		int editHeight = parentRect.bottom - parentRect.top - padding * 3 - buttonHeight;

		CreateWindowW(
			L"Edit",
			0,
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER,
			padding, top, childWidth, editHeight,
			hWnd,
			(HMENU)ID_EDIT_MESSAGE,
			hInstace,
			0 );
		top += editHeight + padding;
		
		int buttonLeft = padding;
		CreateWindowW(
			L"Button",
			L"Abort",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			buttonLeft, top, buttonWidth, buttonHeight,
			hWnd,
			(HMENU)ID_BUTTON_ABORT,
			hInstace,
			0 );
		buttonLeft += buttonWidth + padding;

		CreateWindowW(
			L"Button",
			L"Retry",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			buttonLeft, top, buttonWidth, buttonHeight,
			hWnd,
			(HMENU)ID_BUTTON_RETRY,
			hInstace,
			0 );
		buttonLeft += buttonWidth + padding;

		CreateWindowW(
			L"Button",
			L"Ignore",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			buttonLeft, top, buttonWidth, buttonHeight,
			hWnd,
			(HMENU)ID_BUTTON_IGNORE,
			hInstace,
			0 );
		buttonLeft += buttonWidth + padding;
		top += buttonHeight;
		return 0;
	}

	case WM_KEYDOWN:
	if (wParam == VK_ESCAPE)
	{
		//TesterBase::s_tester->RequestQuit();
		return 0;
	}

	case WM_COMMAND:
	{
		if( lParam )
		{
			int controlId = (int)LOWORD( wParam );
			int controlMessage = (int)HIWORD( wParam );
			switch( controlId )
			{
			case ID_BUTTON_ABORT:
			{
				s_failureWindowResult = controlId;
				PostQuitMessage( 0 );	
				break;
			}

			case ID_BUTTON_RETRY:
			{
				s_failureWindowResult = controlId;
				PostQuitMessage( 0 );
				break;
			}

			case ID_BUTTON_IGNORE:
			{
				s_failureWindowResult = controlId;
				PostQuitMessage( 0 );
				break;
			}

			default:
				break;
			}
		}
		break;
	}

	case WM_CLOSE: // Fall through to WM_DESTROY...
	{
		PostQuitMessage( 0 );
		return 0;
	}

	case WM_DESTROY:
	{
		return 0;
	}

	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
	}
	return 0;
}

int ShowFailureWindow( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow, std::string failure, bool abort, bool retry, bool ignore )
{
	HWND activeWindow = GetActiveWindow();
	EnableWindow( activeWindow, false );

	const wchar_t * CLASS_NAME{ L"MercuryFailureWindowClass" };

	WNDCLASS wc{};
	wc.style = CS_HREDRAW | CS_VREDRAW;

	wc.lpfnWndProc = (WNDPROC)WndProcFailureWindow;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"MercuryFailureWindowClass";
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;

	if (!RegisterClass( &wc ))
	{
		throw;
	}

	HWND hWnd = CreateWindowW(
		CLASS_NAME,
		L"Mercury failure",
		WS_CAPTION | WS_MINIMIZEBOX | WS_BORDER | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT,
		430, 300,
		0,
		0,
		hInstance,
		0 );
	if (!hWnd)
	{
		throw;
	}

	ShowWindow( hWnd, nCmdShow );
	UpdateWindow( hWnd );

	HWND editMessage = GetDlgItem( hWnd, ID_EDIT_MESSAGE );
	SetWindowTextA( editMessage, failure.c_str() );

	HWND buttonAbort = GetDlgItem( hWnd, ID_BUTTON_ABORT );
	EnableWindow( buttonAbort, retry );

	HWND buttonRetry = GetDlgItem( hWnd, ID_BUTTON_RETRY );
	EnableWindow( buttonRetry, retry );

	HWND buttonIgnore = GetDlgItem( hWnd, ID_BUTTON_IGNORE );
	EnableWindow( buttonIgnore, retry );

	MSG msg{};
	while (GetMessage( &msg, NULL, 0, 0 ))
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	DestroyWindow( hWnd );

	EnableWindow( activeWindow, true );
	SetActiveWindow( activeWindow );

	return s_failureWindowResult;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow )
{
	MSG msg;

	static me::game::Game * gameInstance;

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

	try
	{
		me::os::OSParameters osParameters;
		osParameters.hInstance = hInstance;
		osParameters.hPrevInstance = hPrevInstance;
		osParameters.lpszCmdLine = lpszCmdLine;
		osParameters.nCmdShow = nCmdShow;
		osParameters.wndProc = WndProc; 

		bool retry = true;
		while( retry )
		{
			retry = false;
			try
			{
				gameInstance->Initialize( osParameters );
			}
			catch( std::exception exception )
			{
				gameInstance->Debug()->LogLine( "Failure", "Mercury Failure:" );
				gameInstance->Debug()->LogLine( "Failure", exception.what() );

				int result = ShowFailureWindow( hInstance, hPrevInstance, lpszCmdLine, nCmdShow, exception.what(), true, true, true );
				switch( result )
				{
				case ID_BUTTON_ABORT:
					GetGameInstance()->Debug()->LogLine( "Failure", "   Abort" );
					return 0;

				case ID_BUTTON_RETRY:
					GetGameInstance()->Debug()->LogLine( "Failure", "   Retry" );
					retry = true;
					break;

				case ID_BUTTON_IGNORE:
					// Do nothing.
					GetGameInstance()->Debug()->LogLine( "Failure", "   Ignore" );
					break;
				}
			}
		}

		bool running = true;
		while( running )
		{ 
			try
			{
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
			catch( std::exception exception )
			{
				gameInstance->Debug()->LogLine( "Failure", "Mercury Failure: " );
				gameInstance->Debug()->LogLine( "Failure", exception.what() );

				int result = ShowFailureWindow( hInstance, hPrevInstance, lpszCmdLine, nCmdShow, exception.what(), true, false, true );
				switch (result)
				{
				case ID_BUTTON_ABORT:
					GetGameInstance()->Debug()->LogLine( "Failure", "   Abort" );
					return -1;

				case ID_BUTTON_RETRY:
					// TODO:
					break;

				case ID_BUTTON_IGNORE:
					// Do nothing.
					GetGameInstance()->Debug()->LogLine( "Failure", "   Ignore" );
					break;
				}
			}
		}
	}
	catch( std::exception exception )
	{
		gameInstance->Debug()->LogLine( "Failure", "Mercury Failure:" );
		gameInstance->Debug()->LogLine( "Failure", exception.what() );
		MessageBoxA( 0, exception.what(), "Mercury Failure", MB_ICONEXCLAMATION );
		return -1;
	}
 
    return msg.wParam; 
} 
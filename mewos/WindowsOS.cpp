// Copyright (c) 2002 - 2018, Kit10 Studios LLC
// All Rights Reserved

#pragma once

#include <mewos/WindowsOS.h>
#include <mewos/Environment.h>
#include <mewos/Extension.h>
#include <mewos/ErrorHandler.h>
#include <me/debug/DefaultDebug.h>
#include <me/exception/FailedToCreate.h>
#include <me/exception/FileNotFound.h>
#include <unify/Exception.h>
#include <unify/Path.h>
#include <shellapi.h>
#include <WinUser.h>

using namespace me;
using namespace mewos;

WindowsOS::WindowsOS( me::game::IGame * game, me::os::OSParameters osParameters )
	: m_environment{ std::make_shared< Environment >() }
	, m_game{ game }
	, m_debug{ new me::debug::DefaultDebug{osParameters.GetRunPath(), unify::Path{"default"} } }
	, m_keyboard{}
	, m_hasFocus{}
	, m_mouse{}
	, m_osParameters{ osParameters }
	, m_assetPaths{ new rm::AssetPaths }
	, m_block{}
{
	//m_debug->SetErrorHandler( me::debug::IErrorHandler::ptr{ new ErrorHandler( this ) } );
	m_block = m_debug->GetLogger()->CreateBlock( "WindowsOS" );
}

WindowsOS::~WindowsOS()
{
	m_renderers.clear();
}

void WindowsOS::CreatePendingDisplays( std::string title )
{
	if( m_pendingDisplays.empty() )
	{
		return;
	}

	for( auto && display : m_pendingDisplays )
	{
		CreateDisplay( display, title );
	}

	m_pendingDisplays.clear();
}

void WindowsOS::CreateDisplay( render::Display display, std::string title )
{
	bool isPrimary = m_renderers.empty() ? true : false; // Note that this is VERY explicit - we are actual spelling out our intention (even though it looks redundant).

														 // If we have no handle, create a window...
	if( m_osParameters.handle == 0 )
	{
		// Regardless of windowed or not, we need a window...
		WNDCLASS wc;
		memset( &wc, 0, sizeof( wc ) );
		wc.style = 0;
		wc.lpfnWndProc = (WNDPROC)m_osParameters.wndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetHInstance();
		wc.hIcon = LoadIcon( (HINSTANCE)NULL, IDI_APPLICATION );
		wc.hCursor = LoadCursor( (HINSTANCE)NULL, IDC_ARROW );

		if( isPrimary )
		{
			wc.lpszMenuName = L"MainMenu";
			wc.lpszClassName = L"MainWndClass";
		}
		else		    // TODO: This worked, yet is dodgy...
		{
			wc.lpszMenuName = 0;
			wc.lpszClassName = L"SecondWndClass";
		}

		if( !RegisterClass( &wc ) )
		{
			throw std::exception( "Failed to register window class!" );
		}

		int x = static_cast< int >(display.GetPosition().x);
		int y = static_cast< int >(display.GetPosition().y);
		int width = static_cast< int >(display.GetSize().width);
		int height = static_cast< int >(display.GetSize().height);
		HWND parentHandle = (HWND)m_osParameters.parentHandle;
		HINSTANCE hInstance = GetHInstance();
		HWND handle = CreateWindowA( "MainWndClass", title.c_str(), WS_OVERLAPPEDWINDOW, x, y, width, height,
			parentHandle, (HMENU)NULL, hInstance, (LPVOID)NULL );

		if( !handle )
		{
			auto last_error = GetLastError();
			throw exception::FailedToCreate( "Failed to create window!" );
		}

		m_osParameters.handle = handle;

		{ // Resize window to ensure exact pixel match...
			RECT windowRect;
			GetWindowRect( handle, &windowRect );

			RECT clientRect;
			GetClientRect( handle, &clientRect );

			long newWindowWidth = (windowRect.right - windowRect.left) + width - clientRect.right;
			long newWindowHeight = (windowRect.bottom - windowRect.top) + height - clientRect.bottom;
			MoveWindow( handle, windowRect.left, windowRect.top, newWindowWidth, newWindowHeight, false );
		}

		ShowWindow( handle, m_osParameters.nCmdShow );
		UpdateWindow( handle );
	}

	m_renderers.push_back( me::render::IRenderer::ptr{ m_rendererFactory->Produce( display, m_renderers.size() ) } );
}

HINSTANCE WindowsOS::GetHInstance() const
{
	return (HINSTANCE)m_osParameters.hInstance;
}

HWND WindowsOS::GetHandle() const
{
	return (HWND)m_osParameters.handle;
}


/////////////////////////////////////////////////
// me::os::IOS
/////////////////////////////////////////////////

me::os::IEnvironment::weak_ptr WindowsOS::GetEnvironment() const
{
	return m_environment;
}

me::game::IGame * WindowsOS::GetGame()
{
	return m_game;
}

me::debug::IDebug * WindowsOS::Debug()
{
	return m_debug.get();
}

const me::debug::IDebug * WindowsOS::Debug() const
{
	return m_debug.get();
}

void * WindowsOS::Feed( std::string target, void * data )
{
	os::win::OSFood * food = ( os::win::OSFood* )data;

	LRESULT lresult = WndProc((HWND)food->handle, (UINT)food->message, (WPARAM)food->wParam, (LPARAM)food->lParam);
	return (void*)lresult;
}

void WindowsOS::AddDisplay( render::Display display )
{
	m_pendingDisplays.push_back( display );
}

size_t WindowsOS::RendererCount() const
{
	return m_renderers.size();
}

me::render::IRenderer * WindowsOS::GetRenderer( size_t index ) const
{
	return m_renderers[index].get();
}

void WindowsOS::SetHasFocus( bool hasFocus )
{
	m_hasFocus = hasFocus;
}

bool WindowsOS::GetHasFocus() const
{
	return m_hasFocus;
}

void WindowsOS::SetRenderFactory( me::render::IRendererFactory::ptr renderFactory )
{
	m_rendererFactory = renderFactory;
}


void WindowsOS::BuildRenderers( std::string title )
{
	CreatePendingDisplays( title );

	// TODO: DragAcceptFiles( this->GetHWnd(), true );
}

void WindowsOS::Startup()
{
	auto block{ m_block->SubBlock( "Startup" ) };

	auto keyboardItr = m_game->GetInputManager()->FindSource( "Keyboard" );
	if( keyboardItr )
	{
		m_keyboard = keyboardItr.get();
	}

	auto mouseItr = m_game->GetInputManager()->FindSource( "Mouse" );
	if( mouseItr )
	{
		m_mouse = mouseItr.get();
	}
}

void WindowsOS::Shutdown()
{
}

LRESULT WindowsOS::WndProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam )
{
	game::IGame & gameInstance = *m_game;
	static bool trackingMouse = false;

	switch( message )
	{
	case WM_NCCREATE:
		{
			// Extra code is for testing.
			CREATESTRUCT* createstruct = (CREATESTRUCT*)(lParam);
			auto ret = DefWindowProc(handle, message, wParam, lParam);
			auto last_error = GetLastError();
			return ret;
		}
		break;

	case WM_CLOSE: // Fall through to WM_DESTROY...
	case WM_DESTROY:
		gameInstance.Quit();
		return 0;

	case WM_MOUSELEAVE:
	{
		if( m_mouse == nullptr ) break;

		for( size_t renderer = 0; renderer < RendererCount(); ++renderer )
		{
			if( m_osParameters.handle == handle )
			{
				trackingMouse = false;
				//m_mouse->SetState( renderer, "MouseAvailable", "Available", false );
				break;
			}
		}
		break;
	}

	case WM_LBUTTONDOWN:
	{
		if( m_mouse == nullptr ) break;

		for( size_t renderer = 0; renderer < RendererCount(); ++renderer )
		{
			if( GetRenderer( renderer )->GetDisplay().GetHandle() == handle )
			{
				trackingMouse = false;
				input::ButtonData * data = new input::ButtonData();
				data->down = true;
				data->pressed = false;
				size_t inputIndex = m_mouse->GetInputIndex( renderer, "LeftButton" );
				m_mouse->SetInputData( renderer, inputIndex, input::IData::ptr( data ) );
				break;
			}
		}
		break;
	}

	case WM_LBUTTONUP:
	{
		if( m_mouse == nullptr ) break;

		for( size_t renderer = 0; renderer < RendererCount(); ++renderer )
		{
			if( GetRenderer( renderer )->GetDisplay().GetHandle() == handle )
			{
				trackingMouse = false;
				input::ButtonData * data = new input::ButtonData();
				data->down = false;
				data->pressed = false;
				size_t inputIndex = m_mouse->GetInputIndex( renderer, "LeftButton" );
				m_mouse->SetInputData( renderer, inputIndex, input::IData::ptr( data ) );
				break;
			}
		}
		break;
	}

	case WM_RBUTTONDOWN:
	{
		if( m_mouse == nullptr ) break;

		for( size_t renderer = 0; renderer < RendererCount(); ++renderer )
		{
			if( GetRenderer( renderer )->GetDisplay().GetHandle() == handle )
			{
				trackingMouse = false;
				input::ButtonData * data = new input::ButtonData();
				data->down = true;
				data->pressed = false;
				size_t inputIndex = m_mouse->GetInputIndex( renderer, "RightButton" );
				m_mouse->SetInputData( renderer, inputIndex, input::IData::ptr( data ) );
				break;
			}
		}
		break;
	}

	case WM_RBUTTONUP:
	{
		if( m_mouse == nullptr ) break;

		for( size_t renderer = 0; renderer < RendererCount(); ++renderer )
		{
			if( GetRenderer( renderer )->GetDisplay().GetHandle() == handle )
			{
				trackingMouse = false;
				input::ButtonData * data = new input::ButtonData();
				data->down = false;
				data->pressed = false;
				size_t inputIndex = m_mouse->GetInputIndex( renderer, "RightButton" );
				m_mouse->SetInputData( renderer, inputIndex, input::IData::ptr( data ) );
				break;
			}
		}
		break;
	}

	case WM_MBUTTONDOWN:
	{
		if( m_mouse == nullptr ) break;

		for( size_t renderer = 0; renderer < RendererCount(); ++renderer )
		{
			if( GetRenderer( renderer )->GetDisplay().GetHandle() == handle )
			{
				trackingMouse = false;
				input::ButtonData * data = new input::ButtonData();
				data->down = true;
				data->pressed = false;
				size_t inputIndex = m_mouse->GetInputIndex( renderer, "MiddleButton" );
				m_mouse->SetInputData( renderer, inputIndex, input::IData::ptr( data ) );
				break;
			}
		}
		break;
	}

	case WM_MBUTTONUP:
	{
		if( m_mouse == nullptr ) break;

		for( size_t renderer = 0; renderer < RendererCount(); ++renderer )
		{
			if( GetRenderer( renderer )->GetDisplay().GetHandle() == handle )
			{
				trackingMouse = false;
				input::ButtonData * data = new input::ButtonData();
				data->down = false;
				data->pressed = false;
				size_t inputIndex = m_mouse->GetInputIndex( renderer, "MiddleButton" );
				m_mouse->SetInputData( renderer, inputIndex, input::IData::ptr( data ) );
				break;
			}
		}
		break;
	}

	case WM_MOUSEWHEEL:
	{
		if( m_mouse == nullptr ) break;

		short zDelta = GET_WHEEL_DELTA_WPARAM( wParam );
		for( size_t renderer = 0; renderer < RendererCount(); ++renderer )
		{
			if( GetRenderer( renderer )->GetDisplay().GetHandle() == handle )
			{
				trackingMouse = false;
				input::TrackerData * data = new input::TrackerData();
				size_t inputIndex = m_mouse->GetInputIndex( renderer, "Tracker" );
				data->set.z = true;
				data->change.z = zDelta / (float)WHEEL_DELTA;
				m_mouse->SetInputData( renderer, inputIndex, input::IData::ptr( data ) );
				break;
			}
		}
		break;
	}

	case WM_MOUSEMOVE:
	{
		if( m_mouse == nullptr ) break;

		// TODO:
		// Enable tracking when the mouse leaves the client area...
		if( !trackingMouse )
		{
			TRACKMOUSEEVENT trackMouseEvent = TRACKMOUSEEVENT();
			trackMouseEvent.cbSize = sizeof TRACKMOUSEEVENT;
			trackMouseEvent.dwFlags = TME_LEAVE;
			trackMouseEvent.hwndTrack = handle;
			trackMouseEvent.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent( &trackMouseEvent );
		}


		RECT clientRect;
		GetClientRect( handle, &clientRect );

		for( size_t renderer = 0; renderer < RendererCount(); ++renderer )
		{
			if( GetRenderer( renderer )->GetDisplay().GetHandle() == handle )
			{
				trackingMouse = false;

				float width = static_cast<float>( GetRenderer( renderer )->GetViewport().GetSize().width );
				float height = static_cast< float >( GetRenderer( renderer )->GetViewport().GetSize().height );
				float clientWidth = static_cast< float >( clientRect.right );
				float clientHeight = static_cast< float >( clientRect.bottom );

				unify::V2< int > mousePosition( static_cast< int >( LOWORD( lParam ) ), static_cast< int >( HIWORD( lParam ) ) );
				mousePosition.x *= static_cast< int >( width / clientWidth );
				mousePosition.y *= static_cast< int >( height / clientHeight );

				input::TrackerData * data = new input::TrackerData();
				size_t inputIndex = m_mouse->GetInputIndex( renderer, "Tracker" );
				data->set.x = true;
				data->position.x = (float)mousePosition.x;
				data->set.y = true;
				data->position.y = (float)mousePosition.y;
				m_mouse->SetInputData( renderer, inputIndex, input::IData::ptr( data ) );
				break;
			}
		}
		break;
	}

	case WM_ACTIVATE:
	{
		WORD lowOrder = wParam & 0x0000FFFF;
		bool minimized = ( wParam & 0xFFFF0000 ) != 0;
		switch( lowOrder )
		{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			SetHasFocus( true );
			break;
		case WA_INACTIVE:
			SetHasFocus( false );
			break;

		default:
			assert( 0 && "Invalid activity state!" );
		}
	}
	break;
	/*

	case WM_DROPFILES:
	{
	HDROP drop = reinterpret_cast< HDROP >(wParam);

	std::vector< unify::Path > files;
	unify::V2< float > point;

	size_t numberOfFiles = DragQueryFile( drop, 0xFFFFFFFF, 0, 0 );
	for ( size_t file = 0; file < numberOfFiles; ++file )
	{
	char filePath[260];
	DragQueryFileA( drop, file, filePath, 260 );
	files.push_back( filePath );
	}

	POINT pt;
	DragQueryPoint( drop, &pt );
	point.x = static_cast< float >(pt.x);
	point.y = static_cast< float >(pt.y);

	game.OnDragDrop( files, point );
	}
	break;
	*/

	}

	return DefWindowProc( handle, message, wParam, lParam );
}

rm::AssetPaths::ptr WindowsOS::GetAssetPaths()
{
	return m_assetPaths;
}

const me::os::OSParameters * WindowsOS::GetOSParameters() const
{
	return &m_osParameters;
}

me::os::IExtension::ptr WindowsOS::CreateExtension( unify::Path source, const qxml::Element * element )
{
	m_block->SubBlock( "CreateExtention \"" + source.ToString() + "\"" );
	auto extension = me::os::IExtension::ptr{ new Extension( GetGame(), source, element ) };
	return extension;
}
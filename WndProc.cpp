// Copyright (c) 2002 - 2018, Kit10 Studios LLC
// All Rights Reserved

#include <me/game/Game.h>

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

#include <shellapi.h>

extern "C" me::game::Game * GetGameInstance();

extern "C"
LRESULT CALLBACK WndProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam )
{
	auto osFood = me::os::win::OSFood((me::os::win::HWnd)handle, message, (me::os::win::WParam)wParam, (me::os::win::LParam)lParam);
	auto result = GetGameInstance()->GetOS()->Feed( "OS", (void*)&osFood );
	return (LRESULT)result;
}

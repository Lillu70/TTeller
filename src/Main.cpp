
#pragma once


#include <Windows.h>
#include <Xinput.h>

#include "Utility\Primitives.h"
#include "Utility\Intrinsics.h"
#include "Utility\Instrumentation.h"
#include "Utility\Assert.h"
#include "Utility\Utility.h"
#include "Platform\Input.h"
#include "Platform\Platform_Interface.h"

#include "Platform\Win32.cpp"

#include "App\Main_App.h"

static constexpr u32 APP_MEMORY_SIZE = MiB;
static const wchar_t* APP_TITLE = L"N\xE4lk\xE4peli";

Declare_Timing_Tables();

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	Win32_Init(620 * 2, 480 * 2, CW_USEDEFAULT, CW_USEDEFAULT, APP_TITLE, APP_MEMORY_SIZE);
	
	Platform_Calltable calltable = Win32_Get_Calltable();
	Init_App(calltable, s_app.game_state_memory, APP_MEMORY_SIZE);
	
	f64 frame_time = 0;
	
	while(Is_Flag_Set(s_app.flags, (u32)App_Flags::is_running))
    {
		Begin_Timing_Block(run_time);
		
		Win32_Flush_Events();
		
		bool update_surface = false;
		
		Update_App(frame_time, &update_surface);
		
		Win32_Update_Surface(update_surface);
		
		frame_time = Win32_Update_Frame_Time();
		
		End_Timing_Block(run_time);
		
		Instrumentation_Call(Win32_Report_Timing_Results());
    }
	
	return 0;
}
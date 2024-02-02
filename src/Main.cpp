
#pragma once

#include <Windows.h>
#include <Xinput.h>

#include "Utility/Primitives.h"
#include "Utility/Assert.h"
#include "Utility/Utility.h"
#include "Platform/Input.h"
#include "Platform/Platform_Interface.h"

#include "Platform/Win32.cpp"

#include "App/Main_App.h"

static constexpr u32 APP_MEMORY_SIZE = MiB;
static const char* APP_TITLE = "Nalkapeli";


int WINAPI WinMain(HINSTANCE instance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{    
	Win32_Init(instance, 620, 480, CW_USEDEFAULT, CW_USEDEFAULT, APP_TITLE, APP_MEMORY_SIZE);
		
	Platform_Calltable calltable = Win32_Get_Calltable();
	Init_App(calltable, s_app.game_state_memory, APP_MEMORY_SIZE);
	
	f64 frame_time = 0;
	    
	while(Is_Flag_Set(s_app.flags, (u32)App_Flags::is_running))
	{
		Win32_Flush_Events();
		
		bool update_surface = false;
		
		Update_App(frame_time, &update_surface);
		
		Win32_Update_Surface(update_surface);
		
		frame_time = Win32_Update_Frame_Time();
	}
	
	return 0;
}
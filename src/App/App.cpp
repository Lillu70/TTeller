
#pragma once

#include "App.h"
#include "Main_App.h"

#include "GUI_Factory.cpp"


void Init_App(Platform_Calltable platform_calltable)
{
	Init_Shell_From_Paged_General_Allocator(&s_allocator, &s_mem);
	
	s_platform = platform_calltable;
	Generate_Folder_Hierarchy(&s_platform);
	{
		serialization_lalloc.init(&s_platform, MiB);
		s_mem.init(&s_platform, 1);
	}
	
	Init_GUI();
}


void Update_App(f64 delta_time, bool* update_surface)
{
	*update_surface = true;
	Begin_Timing_Block(internal_run_time);
	
	u32 app_flags = s_platform.Get_Flags();
	
	if(Is_Flag_Set(app_flags, (u32)App_Flags::window_has_resized))
	{

		i32 window_width = s_platform.Get_Window_Width();
		i32 window_height = s_platform.Get_Window_Height();
		if(window_width > 0 && window_height > 0)
		{
			u32* pixel_buffer = s_platform.Resize_Pixel_Buffer(window_width, window_height);
			Init_Canvas(&s_canvas, pixel_buffer, v2u{u32(window_width), u32(window_height)});					
		}
		else
		{
			s_canvas.buffer = 0;
		}
	}
	
	if(!s_canvas.buffer)
		return;
	 
	bool wants_to_exit = Is_Flag_Set(app_flags, (u32)App_Flags::wants_to_exit);
	
	#if 0
	
	if(wants_to_exit)
		s_platform.Set_Flag(App_Flags::is_running, false);

	#endif

	Run_Active_Menu(wants_to_exit);
	
	End_Timing_Block(internal_run_time);
}
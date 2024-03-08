
#pragma once

#include "App.h"
#include "Main_App.h"

#include "GUI_Factory.cpp"


void Init_App(Platform_Calltable platform_calltable, void* app_memory, u32 app_memory_size)
{
	Init_Shell_From_General_Allocator(&s_allocator, &s_mem);
	
	s_platform = platform_calltable;
	
	Assert(app_memory_size > INTERIM_MEM_SIZE);
	
	{
		s_interim_mem.init(app_memory, INTERIM_MEM_SIZE);	
		s_mem.init(s_interim_mem.memory + s_interim_mem.capacity, app_memory_size - INTERIM_MEM_SIZE);
	}
	
	Init_GUI();
}


void Update_App(f64 delta_time, bool* update_surface)
{
	__m128 vA = _mm_set1_ps(1.0f);
	__m128 vB = _mm_set1_ps(2.0f);
	__m128 vS = _mm_add_ps(vA, vB);
	
	Begin_Timing_Block(internal_run_time);
	
	u32 app_flags = s_platform.Get_Flags();
	
	if(Is_Flag_Set(app_flags, (u32)App_Flags::window_has_resized))
	{

		i32 window_width = s_platform.Get_Window_Width();
		i32 window_height = s_platform.Get_Window_Height();
		
		u32* pixel_buffer = s_platform.Resize_Pixel_Buffer(window_width, window_height);
		Init_Canvas(&s_canvas, pixel_buffer, v2u{u32(window_width), u32(window_height)});		
	}
	
	if(!s_canvas.buffer)
		return;
	 
	s_interim_mem.clear();
	
	if(Is_Flag_Set(app_flags, (u32)App_Flags::wants_to_exit))
		s_platform.Set_Flag(App_Flags::is_running, false);
	
	
	Run_Active_Menu();
	
	End_Timing_Block(internal_run_time);
}

#pragma once

#include "App.h"
#include "Main_App.h"

#include "GUI_Factory.cpp"


void Init_App(Platform_Calltable platform_calltable, void* app_memory, u32 app_memory_size)
{
	s_platform = platform_calltable;
	
	Assert(app_memory_size > INTERIM_MEM_SIZE);
	
	{
		s_interim_mem.init(app_memory, INTERIM_MEM_SIZE);	
		s_mem.init(s_interim_mem.memory + s_interim_mem.capacity, app_memory_size - INTERIM_MEM_SIZE);
	}
	
	u32* pixel_buffer = s_platform.Get_Pixel_Buffer();
	v2i pixel_buffer_dim = v2i{s_platform.Get_Pixel_Buffer_Width(), s_platform.Get_Pixel_Buffer_Height()};
	
	Init_Canvas(&s_canvas, pixel_buffer, v2i::Cast<u32>(pixel_buffer_dim));
	Init_GUI();

	GUI_Create_New_Event_Frame();
}


void Update_App(f64 delta_time, bool* update_surface)
{
	s_interim_mem.clear();
	
	if(Is_Flag_Set(s_platform.Get_Flags(), (u32)App_Flags::wants_to_exit))
		s_platform.Set_Flag(App_Flags::is_running, false);
	
	
	*update_surface = true;
	Clear_Canvas(&s_canvas, Put_Color(20, 20, 20));
	
	GUI_Handle_Input(&s_gui, &s_platform);
	GUI_Draw_Widgets(&s_gui, &s_canvas, &s_interim_mem);
}
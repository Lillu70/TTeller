
#pragma once

#include "App.h"
#include "Main_App.h"


static void Run_Active_Menu(u32 app_flags)
{
	bool app_wants_to_exit = Is_Flag_Set(app_flags, (u32)App_Flags::wants_to_exit);
	
	Menus current_menu = s_global_data.active_menu;
	
	Update_Actions(&s_platform, s_hotkeys, s_hotkey_count, &s_global_data.action_context);
	
	if(s_hotkeys[Global_Hotkeys::toggle_fullscreen].Is_Pressed())
	{
		bool fullsceen = Is_Flag_Set(app_flags, (u32)App_Flags::is_fullscreen) > 0;
		s_platform.Set_Flag(App_Flags::is_fullscreen, !fullsceen);
	}
	
	if(app_wants_to_exit ||
		s_global_data.force_quit_popup || 
		(s_hotkeys[Global_Hotkeys::open_quit_popup].Is_Pressed() && 
		!s_popup_func))
	{
		s_global_data.force_quit_popup = false;
		s_platform.Set_Flag(App_Flags::wants_to_exit, false);
		
		switch(current_menu)
		{
			case Menus::main_menu:
			case Menus::campaigns_menu:
			case Menus::GM_players:
			{	
				Set_Popup_Function(Do_Main_Menu_Quit_Popup);
			}break;
			
			default:
			{
				if(u32(current_menu) > u32(Menus::EVENT_EDITOR_BEGIN) && 
					u32(current_menu) < u32(Menus::EVENT_EDITOR_END))
				{
					Set_Popup_Function(Do_Event_Editor_Quit_Popup);
				}
				else
				{
					s_platform.Set_Flag(App_Flags::is_running, false);
				}
			}
		}
	}
	else if(s_hotkeys[Global_Hotkeys::open_quit_popup].Is_Pressed() && s_popup_func)
		Close_Popup();
	
	if(s_popup_func)
	{
		s_gui.flags |= GUI_Context_Flags::hard_ignore_selection;
		s_gui_banner.flags |= GUI_Context_Flags::hard_ignore_selection;
	}
	else
	{
		Inverse_Bit_Mask(&s_gui.flags, GUI_Context_Flags::hard_ignore_selection);
		Inverse_Bit_Mask(&s_gui_banner.flags, GUI_Context_Flags::hard_ignore_selection);
	}
	
	GUI_Update_Actions();
	
	switch(current_menu)
	{		
		case Menus::main_menu:
		{
			Do_Main_Menu_Frame();
		}break;
		
		case Menus::campaigns_menu:
		{
			Do_Event_Editor_Campaigns_Menu_Frame();
		}break;
		
		case Menus::EE_all_events:
		{
			Do_Event_Editor_All_Events_Frame();
		}break;
		
		case Menus::EE_participants:
		{
			Do_Event_Editor_Participants_Frame();
		}break;
		
		case Menus::EE_text:
		{
			Do_Event_Editor_Text_Frame();
		}break;
		
		case Menus::EE_consequences:
		{
			Do_Event_Editor_Consequences_Frame();
		}break;
		
		case Menus::select_campaign_to_play_menu:
		{
			s_global_data.active_menu = Menus::GM_players;
			
			#if 1
			
			String game_name = Create_String(&s_allocator, "Kampanjatesti");
			
			Events_Container editor_format_campagin;
			
			if(Load_Campaign(
				&editor_format_campagin,
				&game_name,
				&s_allocator,
				&s_platform))
			{
				Game_State game_state;
				
				if(Convert_Editor_Campaign_Into_Game_Format(
					&game_state,
					&editor_format_campagin,
					&s_allocator))
				{
					if(s_game_state.memory)
						Delete_Game(&s_game_state, &s_allocator);
					
					s_game_state = game_state;
				}

				Delete_All_Events(&editor_format_campagin, &s_allocator);
			}
			else
			{
				Terminate;
			}
			
			game_name.free();
			
			#endif
			
		}break;
		
		case Menus::GM_players:
		{
			Do_New_Game_Players();
		}break;
		
		case Menus::GM_event_display:
		{
			Do_Event_Display_Frame();
		}break;
		
		default:
		{
			Assert(false);
		}
	}
	
	if(s_popup_func)
	{
		s_popup_func();
	}
	else	
	{
		if(u32(current_menu) > u32(Menus::EVENT_EDITOR_BEGIN) &&
			u32(current_menu) < u32(Menus::EVENT_EDITOR_END))
		{
			if(s_hotkeys[Editor_Hotkeys::save].Is_Released())
			{
				Serialize_Campaign(s_editor_state.event_container, &s_platform);
			}
			
			if(s_global_data.active_menu != Menus::EE_all_events)
			{
				if(s_hotkeys[Editor_Hotkeys::jump_to_participants].Is_Released())
				{
					s_global_data.active_menu = Menus::EE_participants;
				}
				
				if(s_hotkeys[Editor_Hotkeys::jump_to_event_text].Is_Released())
				{
					s_global_data.active_menu = Menus::EE_text;
				}
				
				if(s_hotkeys[Editor_Hotkeys::jump_to_consequences].Is_Released())
				{
					s_global_data.active_menu = Menus::EE_consequences;
				}
				
				if(s_hotkeys[Editor_Hotkeys::jump_to_all_events].Is_Released())
				{
					s_global_data.active_menu = Menus::EE_all_events;
				}
			}
		}	
	}
	
	// TODO: sub menu context reseting!
	if(s_global_data.active_menu != current_menu)
	{
		GUI_Reset_Context(&s_gui);
		GUI_Reset_Context(&s_gui_banner);
		
		GUI_Activate_Context(&s_gui_banner);
		
		// -- Cleanup on exiting a menu.
		switch(current_menu)
		{
			case Menus::campaigns_menu:
			{
				if(s_global_data.on_disk_campaign_names)
				{
					Dynamic_Array<String>* on_disk_names = s_global_data.on_disk_campaign_names;
					for(String* name = Begin(on_disk_names); name < End(on_disk_names); ++name)
						name->free();
					
					s_allocator.free(s_global_data.on_disk_campaign_names);
					s_global_data.on_disk_campaign_names = 0;
				}
			}break;
		}
		
		// -- Initialization on entering a menu.
		switch(s_global_data.active_menu)
		{
			case Menus::campaigns_menu:
			{
				Delete_All_Events(&s_editor_state.event_container, &s_allocator);
				
				char exe_path[260];
				u32 exe_path_lenght = s_platform.Get_Executable_Path(exe_path, Array_Lenght(exe_path));
				
				String campaign_directory 
					= Create_String(&s_allocator, exe_path, campaign_folder_wildcard_path);
				
				s_global_data.on_disk_campaign_names = s_platform.Search_Directory_For_Maching_Names(
					campaign_directory.buffer, 
					&s_allocator);
				
				campaign_directory.free();
				
				Dynamic_Array<String>* on_disk_names = s_global_data.on_disk_campaign_names;
				if(on_disk_names)
				{
					for(String* name = Begin(on_disk_names); name < End(on_disk_names); ++name)
					{
						for(u32 i = name->lenght - 1; i < name->lenght; --i)
						{
							if(name->buffer[i] == '.')
							{
								name->lenght = i;
								name->buffer[i] = 0;
								break;
							}
						}
					}
				}
				
			}break;
		}
	}
	
	if(s_hotkeys[Global_Hotkeys::display_memory].Is_Down())
	{
		Canvas canvas = Create_Sub_Canvas(&s_canvas, v2u{s_canvas.dim.x, s_canvas.dim.y / 2});
		
		Draw_Memory_Display(&s_mem, &canvas);
	}
	
	#if 0
	{
		static Image subpixel_img = {};
		if(!subpixel_img.buffer)
		{
			char* path = "Allahu.png";
			Load_Image(&subpixel_img, path, &s_platform);
		}
		f64 time = s_platform.Get_Time_Stamp();
		
		v2f pos, dim;
		pos = Get_Middle(&s_canvas);
		v2f base = v2f{100, 100};
		
		#if 1
			pos.x += base.x * 7 * f32(sin(time / 5));
			pos.y += base.y * 5 * f32(cos(time / 5));
			dim = v2f{base.x * f32(cos(time) + 2.0), base.y * f32(sin(time) + 2.0)};
		
		#else
			static f64 first_time = s_platform.Get_Time_Stamp();
		
			pos += f32(time - first_time);
			dim = base;
		#endif
		
		Rect rect = Create_Rect_Center(pos, dim);
		
		#if 1
			Draw_Image(&s_canvas, &subpixel_img, rect);
		
		#else
			Draw_Image_Badly(&s_canvas, &subpixel_img, rect);
		
		#endif		
	}
	#endif
}


void Init_App(Platform_Calltable platform_calltable)
{	
	Init_Shell_From_Paged_General_Allocator(&s_allocator, &s_mem);
	
	s_platform = platform_calltable;
	Generate_Folder_Hierarchy(&s_platform);
	{
		s_serialization_lalloc.init(&s_platform, MiB);
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
		s_global_data.popup_panel_rect = Create_Rect_Min_Max_HZ(v2f{0,0}, v2f{0,0});
		
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
	 
	Run_Active_Menu(app_flags);
	
	End_Timing_Block(internal_run_time);
}

#pragma once


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
		s_hotkeys[Global_Hotkeys::open_quit_popup].Is_Pressed())
	{
		s_global_data.force_quit_popup = false;
		s_platform.Set_Flag(App_Flags::wants_to_exit, false);
		
		switch(current_menu)
		{
			case Menus::main_menu:
			case Menus::campaigns_menu:
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
				Serialize_Campaign(s_global_data.event_container, &s_platform);
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
				Delete_All_Events(&s_global_data.event_container, &s_allocator);
				
				s_global_data.on_disk_campaign_names = s_platform.Search_Directory_For_Maching_Names(
					campaign_folder_wildcard_path, 
					&s_allocator);
				
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
}
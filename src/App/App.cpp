

// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#pragma once

#include "App.h"
#include "Main_App.h"


void Init_App(Platform_Calltable platform_calltable)
{
    Init_Shell_From_Paged_General_Allocator(&s_allocator, &s_mem);
    
    s_platform = platform_calltable;
    Generate_Folder_Hierarchy(&s_platform);
    {
        s_scrach_buffer.init(&s_platform, MiB);
        s_mem.init(&s_platform, 1);
    }
    Init_GUI();
}


void Update_App()
{
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
        !s_global_data.popup_func))
    {
        s_global_data.force_quit_popup = false;
        s_platform.Set_Flag(App_Flags::wants_to_exit, false);
        
        if(u32(current_menu) > u32(Menus::EVENT_EDITOR_BEGIN) && 
            u32(current_menu) < u32(Menus::EVENT_EDITOR_END))
        {
            Set_Popup_Function(Do_Event_Editor_Quit_Popup);
        }
        else
        {
            Set_Popup_Function(Do_Default_Quit_Popup);
        }
    }
    else if(s_hotkeys[Global_Hotkeys::open_quit_popup].Is_Pressed() && s_global_data.popup_func)
        Close_Popup();
    
    if(s_global_data.popup_func)
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
            Do_Select_Campagin_To_Play_Frame();
        }break;
        
        case Menus::GM_players:
        {
            Do_New_Game_Players();
        }break;
        
        case Menus::GM_event_display:
        {
            Do_Event_Display_Frame();
        }break;
        
        case Menus::settings_menu:
        {
            Do_Settings_Menu_Frame();
        }break;
        
        case Menus::GM_let_the_games_begin:
        {
            Do_Let_The_Games_Begin_Frame();
        }break;
        
        case Menus::GM_day_counter:
        {
            Do_Day_Counter_Display_Frame();
        }break;
        
        case Menus::GM_night_falls:
        {
            Do_Night_Falls_Frame();
        }break;
        
        case Menus::GM_everyone_is_dead:
        {
            Do_All_Players_Are_Dead_Frame();
        }break;
        
        case Menus::GM_we_have_a_winner:
        {
            Do_We_Have_A_Winner_Frame();
        }break;
        
        case Menus::GM_event_assignement_failed:
        {
            Do_Event_Assignement_Failed_Frame();
        }break;
        
        default:
        {
            Terminate;
        }
    }
    
    if(s_global_data.popup_func)
    {
        Run_Popup_GUI_Frame();
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
    
    
    if(s_global_data.active_menu != current_menu)
    {
        GUI_Reset_Context(&s_gui);
        GUI_Reset_Context(&s_gui_banner);
        GUI_Activate_Context(&s_gui_banner);
        
        // Handle leaving a menu.
        switch(current_menu)
        {
            case Menus::EE_text:
            {
                Editor_Event* event = Active_Event(&s_editor_state);
                Update_Editor_Event_Text_Issues(event);
            }break;
            
            case Menus::EE_participants:
            {
                Editor_Event* event = Active_Event(&s_editor_state);
                Update_Editor_Event_Participant_Issues(event);
                
                if(event->issues.errors || event->issues.warnings)
                    Update_Editor_Event_Text_Issues(event);
            }break;
            
            case Menus::EE_consequences:
            {
                Editor_Event* event = Active_Event(&s_editor_state);
                Update_Editor_Event_Consequence_Issues(event);
            }break;
            
            case Menus::select_campaign_to_play_menu:
            case Menus::campaigns_menu:
            {
                Clear_Editor_Format_Campaigns();
            }break;
        }
        
        // Handle entering a menu.
        switch(s_global_data.active_menu)
        {            
            case Menus::select_campaign_to_play_menu:
            case Menus::campaigns_menu:
            {
                Gather_Editor_Format_Campaigns();
            }break;
        }
    }
    
    #if 1
    if(s_hotkeys[Global_Hotkeys::display_memory].Is_Down())
    {
        Canvas canvas = Create_Sub_Canvas(&s_canvas, v2u{s_canvas.dim.x, s_canvas.dim.y / 2});
        
        Draw_Memory_Display(&s_mem, &canvas);
    }
    
    #endif
    
    s_scrach_buffer.clear();
    
    s_global_data.pooled_contexes_in_use_count = 0;
    
    End_Timing_Block(internal_run_time);
}
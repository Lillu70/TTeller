

// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#pragma once


static void Do_Main_Menu_Name_New_Campaign_Popup(GUI_Context*);
static void Do_Event_Editor_On_Exit_Editor(GUI_Context*);
static void Do_Event_Editor_Delete_Event_Popup(GUI_Context*);
static void Do_Event_Editor_Display_Active_Event_Errors_Popup(GUI_Context*);
static void Do_Name_New_Campaign_Popup(GUI_Context* context);
static void Do_Are_You_Sure_You_Want_To_Delete_Campaing_Popup(GUI_Context* context);
static void Do_Rename_Campaing_Popup(GUI_Context* context);


static void Free_New_Campaing_Name()
{
    s_global_data.new_campaign_name.free();
}


static void Do_Event_Editor_All_Events_Frame()
{
    Assert(s_editor_state.source == Menus::campaigns_menu || s_editor_state.source == Menus::select_campaign_to_play_menu);
    
    void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        constexpr f32 s = 2.f;
        if(GUI_Do_Button(context, &GUI_AUTO_TOP_LEFT, &GUI_AUTO_FIT, "<", GUI_Scale_Default(s)))
        {
            bool diff = Is_Event_Container_Different_From_On_Disk_File(
                &s_editor_state.event_container, 
                &s_platform,
                &s_allocator);
            
            if(diff)
            {
                Set_Popup_Function(Do_Event_Editor_On_Exit_Editor);
            }
            else
            {
                Delete_Event_Container(&s_editor_state.event_container, &s_allocator);
                s_global_data.active_menu = s_editor_state.source;
                return;
            }
        }
        
        GUI_Push_Layout(context);
        
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        char* title_text = s_editor_state.event_container.campaign_name.buffer;
        GUI_Do_Title_Text(context, AUTO, title_text, GUI_Scale_Default(s));
        
        v2f scaler = GUI_Scale_Default(s);
        v2f img_dim = v2f{GUI_Character_Height(context, scaler.x), GUI_Character_Height(context, scaler.y)};
        if(GUI_Do_Image_Button(context, AUTO, &img_dim, &s_global_data.edit_image, v2f{0.9f, 0.9f}))
        {
            Set_Popup_Function(Do_Rename_Campaing_Popup, Free_New_Campaing_Name);
        }
        f32 rename_button_pos_y = context->layout.last_element.pos.y;
        
        f32 title_height = context->layout.last_element.dim.y;
        f32 title_max_x = context->layout.last_element.rect.max.x - context->anchor_base.x;
        
        GUI_Pop_Layout(context);            
    
        static bool jump_into_new_event = true;
        
        GUI_Do_Spacing(context, v2f{0, s_post_title_y_spacing});
        
        f32 char_height = GUI_Character_Height(context) + f32(context->theme->padding);
        v2f checkbox_dim = v2f{char_height, char_height};
        GUI_Do_Checkbox(context, AUTO, &checkbox_dim, &jump_into_new_event);
        
        GUI_Push_Layout(context);
        
        context->layout.build_direction = GUI_Build_Direction::right_center;
        GUI_Do_Text(context, AUTO, L1(jump_to_event), GUI_Highlight_Prev(context));
        
        GUI_Pop_Layout(context);
        
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(create_new_day_event)))
        {
            if(jump_into_new_event)
            {
                s_global_data.active_menu = Menus::EE_participants;
                s_editor_state.active_event_index = s_editor_state.event_container.day_event_count;
            }
            
            String unique_name = Generate_Unique_Name(
                Begin(s_editor_state.event_container.events), 
                s_editor_state.event_container.day_event_count,
                &s_allocator);
            
            u32 temp_count = s_editor_state.event_container.events->count;
            
            // NOTE: Just to grow the array if needed. 
            // the actual memory contents is irrelevant at this point in time.
            Push(&s_editor_state.event_container.events, &s_allocator);
            
            Editor_Event* buffer = Begin(s_editor_state.event_container.events);
            
            // Checks if there are any night events.
            if(temp_count - s_editor_state.event_container.day_event_count > 0)
            {
                Insert_Element_Into_Packed_Array(
                    buffer,
                    buffer + temp_count,
                    &temp_count,
                    sizeof(*buffer),
                    s_editor_state.event_container.day_event_count);
            }
            
            Editor_Event* event = buffer + s_editor_state.event_container.day_event_count;
            Init_Event_Takes_Name_Ownership(event, &s_allocator, &unique_name);
            Update_Editor_Event_Issues(event);
            s_editor_state.event_container.day_event_count += 1;
        }
        
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(create_new_night_event)))
        {
            if(jump_into_new_event)
            {
                s_global_data.active_menu = Menus::EE_participants;
                s_editor_state.active_event_index = s_editor_state.event_container.events->count;
            }
            
            String unique_name = Generate_Unique_Name(
                Begin(s_editor_state.event_container.events) + s_editor_state.event_container.day_event_count, 
                s_editor_state.event_container.events->count - s_editor_state.event_container.day_event_count,
                &s_allocator);
            
            Editor_Event* event = Push(&s_editor_state.event_container.events, &s_allocator);
            Init_Event_Takes_Name_Ownership(event, &s_allocator, &unique_name);
            Update_Editor_Event_Issues(event);
        }
        
        // -- title bar buttons --    
        f32 padding = context->theme->padding;
        
        char* save_text = L1(save);
        
        f32 w1 = GUI_Tight_Fit_Text(save_text, &context->theme->font).x + padding;
        
        f32 ctrl_buttons_width = w1 + context->dynamic_slider_girth + padding * 2;
        v2f title_row_pos 
            = Get_Title_Bar_Row_Placement(context, title_max_x, padding, ctrl_buttons_width);
        
        context->layout.anchor = GUI_Anchor::left;
        
        title_row_pos.y = rename_button_pos_y;
        
        v2f dim = v2f{w1, title_height};
    
        // Save button.
        if(GUI_Do_Button(context, &title_row_pos, &dim, save_text))
        {
            Serialize_Campaign(&s_editor_state.event_container, &s_platform);
        }
        
        if(GUI_Is_Context_Active(&s_gui_banner) &&
            s_hotkeys[Editor_Hotkeys::active_pannel_toggle].Is_Released() && 
            s_editor_state.event_container.events->count)
        {
            GUI_Activate_Context(&s_gui);
        }
    }; // ----------------------------------------------------------------------------------------
    
    void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        if(!s_editor_state.event_container.events)
            return;
      
        GUI_Context* gui_event_list_day     = Get_GUI_Context_From_Pool();
        GUI_Context* gui_event_list_night   = Get_GUI_Context_From_Pool();
        
        u32 day_event_count = s_editor_state.event_container.day_event_count;
        u32 night_event_count = s_editor_state.event_container.events->count - day_event_count;
        
        // --- Active context and hotkey ------------------------------------------------------------
        {
            if(s_hotkeys[Editor_Hotkeys::active_pannel_toggle].Is_Released() &&
                Bit_Not_Set(s_gui_banner.flags, GUI_Context_Flags::hard_ignore_selection))
            {
                if(GUI_Is_Context_Active(gui_event_list_day))
                {
                    if(night_event_count > 0)
                        GUI_Activate_Context(gui_event_list_night);
                    else
                        GUI_Activate_Context(&s_gui_banner);
                }
                else if(GUI_Is_Context_Active(gui_event_list_night))
                    GUI_Activate_Context(&s_gui_banner);
            }
            
            if(!s_editor_state.event_container.events->count && (
                GUI_Is_Context_Active(gui_event_list_day) || 
                GUI_Is_Context_Active(gui_event_list_night)))
            {
                GUI_Activate_Context(&s_gui_banner);
            }
            
            else if(GUI_Is_Context_Active(gui_event_list_day) && !day_event_count)
            {
                GUI_Activate_Context(gui_event_list_night);
            }
            
            else if(GUI_Is_Context_Active(gui_event_list_night) && !night_event_count)
            {
                GUI_Activate_Context(gui_event_list_day);
            }

            else if(GUI_Is_Context_Active(&s_gui))            
            {
                if(day_event_count == 0)
                    GUI_Activate_Context(gui_event_list_night);
                
                else
                    GUI_Activate_Context(gui_event_list_day);
            }            
        }
        // ------------------------------------------------------------------------------------------


        u32 border_width = 30;
        u32 border_width_x2 = border_width * 2;
        u32 canvas_half_width = context->canvas->dim.x / 2;
        u32 sub_x = canvas_half_width - border_width_x2;
        
        char* day_list_text = L1(day_events);

        f32 y_text_pos = f32(context->canvas->dim.y) - f32(context->theme->padding);
        v2f day_list_text_pos = v2f{f32(border_width), y_text_pos};
        u32 day_list_text_lenght = Null_Terminated_Buffer_Lenght(day_list_text);
        Font* font = &context->theme->font;
        
        f32 left_edge 
            = day_list_text_pos.x + f32(font->char_width * day_list_text_lenght * GUI_DEFAULT_TEXT_SCALE.x);
        
        v2f night_list_text_pos = v2f{f32(canvas_half_width) + border_width, y_text_pos};
        u32 bounds;
        if(left_edge < night_list_text_pos.x)
        {
            GUI_Do_Text(context, &day_list_text_pos, day_list_text);
            GUI_Do_Text(context, &night_list_text_pos, L1(night_events));
            bounds = u32(GUI_Get_Bounds_In_Pixel_Space(context).min.y);
        }
        else
        {
            bounds = context->canvas->dim.y;
        }
        
        u32 top_offset = border_width_x2;
        top_offset += context->canvas->dim.y - bounds;
        v2u sub_dim = v2u{sub_x, context->canvas->dim.y - top_offset};
        
        // Region too small.
        if(context->canvas->dim.y <= top_offset || canvas_half_width <= border_width_x2)
            return;
        
        void(*do_event_list)(GUI_Context*, Editor_Event*, v2f**, u32) 
            = [](GUI_Context* context, Editor_Event* events, v2f** pos, u32 i)
        {
            Editor_Event* event = events + i;
            
            GUI_Theme* theme = context->theme;
            
            if(event->issues.errors)
                context->theme = &s_settings.error_theme;                
            else if(event->issues.warnings)
                context->theme = &s_settings.warning_theme;
            
            // Destroy event
            v2f del_button_dim = v2f{} + GUI_Character_Height(context) + theme->padding;
            if(GUI_Do_Image_Button(context, *pos, &del_button_dim, &s_global_data.delete_image, v2f{} + 0.9f))
            {
                s_editor_state.event_idx_to_delete = i;
                Set_Popup_Function(Do_Event_Editor_Delete_Event_Popup);
            }
            *pos = 0;
            
            GUI_Push_Layout(context);
            
            context->layout.build_direction = GUI_Build_Direction::right_center;
            
            if((event->issues.errors || event->issues.warnings) && 
                GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "!"))
            {
                
                s_editor_state.active_event_index = i;
                Set_Popup_Function(Do_Event_Editor_Display_Active_Event_Errors_Popup);
            }
            
            if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, event->name.buffer))
            {
                s_global_data.active_menu = Menus::EE_participants;
                s_editor_state.active_event_index = i;
            }
            
            context->theme = theme;
            
            #if 0
            
            char index_text_buffer[12] = {0};
            GUI_Do_Text(
                context, 
                AUTO, 
                U32_To_Char_Buffer((u8*)&index_text_buffer, i));
            
            #endif
            
            GUI_Pop_Layout(context);
        };
        
        // Day list sub gui
        {
            v2u day_list_buffer_offset = v2u{border_width, border_width};
            Canvas event_list_day_canvas = Create_Sub_Canvas(&s_canvas, sub_dim, day_list_buffer_offset);
            Clear_Sub_Canvas(&event_list_day_canvas, s_settings.list_bg_color);
            
            Inverse_Bit_Mask(&gui_event_list_day->flags,  GUI_Context_Flags::hard_ignore_selection);
            u32 set_mask = GUI_Context_Flags::enable_dynamic_sliders;
            set_mask |= (context->flags &  GUI_Context_Flags::hard_ignore_selection);
            gui_event_list_day->flags |= set_mask;
            
            GUI_Begin_Context(
                gui_event_list_day,
                &event_list_day_canvas,
                &s_global_data.action_context,
                &s_settings.theme,
                day_list_buffer_offset.As<i32>());
            {
                Editor_Event* events = Begin(s_editor_state.event_container.events);
                
                v2f* pos = &GUI_AUTO_TOP_LEFT;
                for(u32 i = 0; i < s_editor_state.event_container.day_event_count; ++i)
                {
                    do_event_list(gui_event_list_day, events, &pos, i);
                }
                
            }
            GUI_End_Context(gui_event_list_day);
        }
        
        // Night list sub gui
        {
            v2u night_list_buffer_offset = v2u{border_width * 3 + sub_x, border_width};
            Canvas event_list_night_canvas = Create_Sub_Canvas(&s_canvas, sub_dim, night_list_buffer_offset);
            Clear_Sub_Canvas(&event_list_night_canvas, s_settings.list_bg_color);
            
            Inverse_Bit_Mask(&gui_event_list_night->flags,  GUI_Context_Flags::hard_ignore_selection);
            u32 set_mask = GUI_Context_Flags::enable_dynamic_sliders;
            set_mask |= (context->flags &  GUI_Context_Flags::hard_ignore_selection);
            gui_event_list_night->flags |= set_mask;
            
            GUI_Begin_Context(
                gui_event_list_night,
                &event_list_night_canvas,
                &s_global_data.action_context,
                &s_settings.theme,
                night_list_buffer_offset.As<i32>());
            {
                Editor_Event* events = Begin(s_editor_state.event_container.events);
                
                v2f* pos = &GUI_AUTO_TOP_LEFT;
                for(u32 i = s_editor_state.event_container.day_event_count; 
                    i < s_editor_state.event_container.events->count; ++i)
                {
                    do_event_list(gui_event_list_night, events, &pos, i);
                }
            }
            
            GUI_End_Context(gui_event_list_night);
        }
        
    }; // ----------------------------------------------------------------------------------------
    
    Do_GUI_Frame_With_Banner(banner_func, menu_func, false);
}


static void Do_Event_Editor_Participants_Frame()
{
    void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        constexpr f32 s = 2.f;
        
        if(GUI_Do_Button(context, &GUI_AUTO_TOP_LEFT, &GUI_AUTO_FIT, "<", GUI_Scale_Default(s)))
        {
            s_global_data.active_menu = Menus::EE_all_events;
        }
        f32 title_height = context->layout.last_element.dim.y;
        
        GUI_Push_Layout(context);
        
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        GUI_Do_Title_Text(context, AUTO, L1(parti), GUI_Scale_Default(s));
        f32 title_max_x = context->layout.last_element.rect.max.x - context->anchor_base.x;
        
        GUI_Pop_Layout(context);
        
        GUI_Do_Spacing(context, v2f{0, s_post_title_y_spacing});
        
        Editor_Event* event = Active_Event(&s_editor_state);
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(add_new_parti)))
        {
            s_gui.flags |= GUI_Context_Flags::maxout_horizontal_slider;
            if(event->participents->count < event->max_participent_count)
            {
                Create_Participent(&event->participents, &s_allocator);
            }
        }
        
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        if(event->participents->count > 0)
        {    
            if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(delete_parti)))
                Delete_All_Participants_From_Event(event, &s_allocator);
        }    
        
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(add_global_req)))
        {
            Global_Mark_Requirement* gmr = Push(&event->global_mark_reqs, &s_allocator);
            gmr->mark = Create_String(&s_allocator, 12);
        }
        
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        // -- title bar buttons --    
        f32 padding = context->theme->padding;
        char* go_to_event_text_text = L1(event_text);
        char* go_to_consequences_text = L1(cons);
        
        Font* font = &context->theme->font;
        f32 w1 = GUI_Tight_Fit_Text(go_to_event_text_text, font).x + padding;
        f32 w2 = GUI_Tight_Fit_Text(go_to_consequences_text, font).x + padding;
        
        f32 ctrl_buttons_width = w1 + w2 + context->dynamic_slider_girth + padding * 2;
        v2f title_row_pos 
            = Get_Title_Bar_Row_Placement(context, title_max_x, padding, ctrl_buttons_width);
        
        v2f dim = v2f{w1, title_height};
        if(GUI_Do_Button(context, &title_row_pos, &dim, go_to_event_text_text))
        {
            s_global_data.active_menu = Menus::EE_text;
        }
        
        dim = v2f{w2, title_height};
        if(GUI_Do_Button(context, AUTO, &dim, go_to_consequences_text))
        {
            s_global_data.active_menu = Menus::EE_consequences;
        }
        
    }; // ----------------------------------------------------------------------------------------
    
    
    void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        Editor_Event* event = Active_Event(&s_editor_state);
    
        f32 collumn_min_width = 150.f * GUI_DEFAULT_TEXT_SCALE.x;
        f32 padding = context->theme->padding;
        
        v2f* pos = &GUI_AUTO_TOP_LEFT;
        if(event->global_mark_reqs->count)
        {
            
            GUI_Do_Text(context, pos, L1(global_reqs));
            pos = 0;
            
            GUI_Push_Layout(context);
            
            f32 collumn_start = GUI_Get_Collumn_Start(context, X_AXIS);
            
            Global_Mark_Requirement* global_mark_reqs = Begin(event->global_mark_reqs);
            for(u32 i = 0; i < event->global_mark_reqs->count; ++i)
            {
                GUI_Placement rcp = context->layout.last_element;
                
                if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "X"))
                {
                    context->layout.last_element = rcp;
                    
                    Delete_Global_Mark_Requirement(event->global_mark_reqs, i--);
                    continue;
                }
                
                f32 left_size 
                    = context->layout.last_element.pos.x - context->layout.last_element.dim.x / 2; 
                
                GUI_Push_Layout(context);
                context->layout.build_direction = GUI_Build_Direction::right_center;
                
                Global_Mark_Requirement* gmr = global_mark_reqs + i;
                
                GUI_Do_Text(context, AUTO, L1(global_mark), GUI_Highlight_Prev(context));
                
                f32 right_side 
                    = context->layout.last_element.pos.x + context->layout.last_element.dim.x / 2; 
                
                GUI_Pop_Layout(context);
                
                f32 width = right_side - left_size;
                GUI_Do_SL_Input_Field(context, AUTO, &width, &gmr->mark);
                
                if(u32 s = GUI_Do_Dropdown_Button(
                    context, AUTO, AUTO, 
                    u32(gmr->mark_exists),
                    LN(exists_statement_names)))
                {
                    gmr->mark_exists = Exists_Statement(s - 1);
                }
                
                if(gmr->mark_exists == Exists_Statement::does_have)
                {
                    GUI_Do_Text(context, AUTO, L1(duration_remaining), GUI_Highlight_Next(context, 2));
                    
                    // Numerical_Relation
                    if(u32 s = GUI_Do_Dropdown_Button(
                        context, 
                        AUTO, 
                        &GUI_AUTO_FIT, 
                        (char*)s_numerical_relation_names[u32(gmr->numerical_relation)],
                        Array_Lenght(s_numerical_relation_names),
                        (char**)s_numerical_relation_names))
                    {
                        gmr->numerical_relation = Numerical_Relation(s - 1);
                    }
                    
                    GUI_Push_Layout(context);
                    
                    context->layout.build_direction = GUI_Build_Direction::right_center;
                    
                    CSTR_List duration_names = LN(duration_names);
                    
                    // Target value
                    if(u32 s = GUI_Do_Dropdown_Button(
                        context, 
                        AUTO, 
                        &GUI_AUTO_FIT, 
                        duration_names.list[gmr->relation_target],
                        duration_names.count - 1,
                        duration_names.list + 1))
                    {
                        gmr->relation_target = i8(s);
                    }
                    
                    GUI_Pop_Layout(context);
                }
                
                GUI_Do_Spacing(context, AUTO);
            }
            
            GUI_Pop_Layout(context);
        
            GUI_End_Collumn(context, collumn_min_width, collumn_start, X_AXIS);
            
            context->layout.build_direction = GUI_Build_Direction::right_top;
            
            Rect bounds = GUI_Get_Bounds_In_Pixel_Space(context);
            v2f pannel_dim = v2f{10, bounds.max.y - bounds.min.y};
            GUI_Do_Panel(context, AUTO, &pannel_dim);
        }        
        
        // -- participants --
        
        GUI_Do_Text(context, pos, L1(parti));
        
        context->layout.build_direction = GUI_Build_Direction::down_left;
        
        for(u32 p = 0; p < event->participents->count; ++p)
        {
            Participent* parti = Begin(event->participents) + p;
            
            bool defer_del = false;
            
            if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "X"))
            {
                defer_del = true;
            }
            
            f32 collumn_start = GUI_Get_Collumn_Start(context, X_AXIS);
            
            context->layout.build_direction = GUI_Build_Direction::right_top;
            
            GUI_Push_Layout(context);
            context->layout.build_direction = GUI_Build_Direction::down_left;
            
            GUI_Push_Layout(context);
            
            context->layout.build_direction = GUI_Build_Direction::right_center;
            
            u8 typing_marker_buffer[12 + 2] = {0};
            char* typing_marker;
            {
                typing_marker = U32_To_Char_Buffer((u8*)&typing_marker_buffer[2], p + 1);
                typing_marker -= 1;
                *typing_marker = 'k';
                typing_marker -= 1;
                *typing_marker = '/';
            }
            
            GUI_Do_Text(context, AUTO, typing_marker, GUI_Highlight_Prev(context));
            
            GUI_Pop_Layout(context);
            
            if(u32 s = GUI_Do_Dropdown_Button(
                context,
                AUTO,
                &GUI_AUTO_FIT, 
                L1(add_req), 
                LN(req_names)))
            {
                if(parti->reqs->count < parti->max_requirements)
                {
                    Participation_Requirement_Type new_type = (Participation_Requirement_Type)(s - 1);
                    Init_Participation_Requirement(Push(&parti->reqs, &s_allocator), new_type, &s_allocator);
                }
            }
            
            f32 new_req_width = context->layout.last_element.dim.x;
            
            GUI_Do_Spacing(context, AUTO);
            
            Participation_Requirement* requirement_array = Begin(parti->reqs);
            for(u32 r = 0; r < parti->reqs->count; ++r)
            {
                Participation_Requirement* req = requirement_array + r;
                
                GUI_Placement rcp = context->layout.last_element;
                
                if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "X"))
                {
                    Make_Requirement_Hollow(req);
                    
                    u32 size = sizeof(Participation_Requirement);
                    Remove_Element_From_Packed_Array(requirement_array, &parti->reqs->count, size, r--);
                    
                    context->layout.last_element = rcp;
                    continue;
                }
                
                GUI_Push_Layout(context);
                
                context->layout.build_direction = GUI_Build_Direction::right_center;
                
                GUI_Do_Text(context, AUTO, LN1(req_names, (u32)req->type), GUI_Highlight_Prev(context));
                
                GUI_Pop_Layout(context);
                
                switch(req->type)
                {
                    case Participation_Requirement_Type::mark_item:
                    case Participation_Requirement_Type::mark_personal:
                    {
                        GUI_Do_SL_Input_Field(context, AUTO, &new_req_width, &req->mark);
                        
                        if(u32 s = GUI_Do_Dropdown_Button(
                            context, 
                            AUTO, 
                            AUTO, 
                            u32(req->mark_exists),
                            LN(exists_statement_names)))
                        {
                            req->mark_exists = Exists_Statement(s - 1);
                        }
                        
                        if(req->mark_exists == Exists_Statement::does_have)
                        {
                            GUI_Do_Text(context, AUTO, L1(duration_remaining), GUI_Highlight_Next(context, 2));
                            
                            // Numerical_Relation
                            if(u32 s = GUI_Do_Dropdown_Button(
                                context, 
                                AUTO, 
                                &GUI_AUTO_FIT, 
                                (char*)s_numerical_relation_names[u32(req->numerical_relation)],
                                Array_Lenght(s_numerical_relation_names),
                                (char**)s_numerical_relation_names))
                            {
                                req->numerical_relation = Numerical_Relation(s - 1);
                            }
                            
                            GUI_Push_Layout(context);
                            
                            context->layout.build_direction = GUI_Build_Direction::right_center;
                            
                            CSTR_List duration_names = LN(duration_names);
                            
                            // Target value
                            if(u32 s = GUI_Do_Dropdown_Button(
                                context, 
                                AUTO, 
                                &GUI_AUTO_FIT, 
                                duration_names.list[req->relation_target],
                                duration_names.count - 1,
                                duration_names.list + 1))
                            {
                                req->relation_target = u16(s);
                            }
                            
                            GUI_Pop_Layout(context);
                        }
                    }break;
                    
                    case Participation_Requirement_Type::character_stat:
                    {
                        // Stat
                        if(u32 s = GUI_Do_Dropdown_Button(
                            context, 
                            AUTO, 
                            &GUI_AUTO_FIT, 
                            u32(req->stat_type),
                            LN(stat_names)))
                        {
                            req->stat_type = Character_Stats(s - 1);
                        }
                        
                        GUI_Push_Layout(context);
                        
                        context->layout.build_direction = GUI_Build_Direction::right_center;
                        
                        // Numerical_Relation
                        constexpr char* value_option_names[] = {"0", "1", "2", "3"};
                        if(u32 s = GUI_Do_Dropdown_Button(
                            context, 
                            AUTO, 
                            &GUI_AUTO_FIT, 
                            (char*)s_numerical_relation_names[u32(req->numerical_relation)],
                            Array_Lenght(s_numerical_relation_names),
                            (char**)s_numerical_relation_names))
                        {
                            req->numerical_relation = Numerical_Relation(s - 1);
                        }
                        
                        // Target value
                        if(u32 s = GUI_Do_Dropdown_Button(
                            context, 
                            AUTO, 
                            AUTO, 
                            (char*)value_option_names[req->relation_target],
                            Array_Lenght(value_option_names),
                            (char**)value_option_names))
                        {
                            req->relation_target = u16(s - 1);
                        }
                        
                        GUI_Pop_Layout(context);
                        
                        if((req->numerical_relation == Numerical_Relation::greater_than &&
                            req->relation_target == Array_Lenght(value_option_names) - 1) ||
                            (req->numerical_relation == Numerical_Relation::less_than &&
                            req->relation_target == 0))
                        {
                            GUI_Do_Title_Text(context, AUTO, L1(impossible), GUI_Scale_Default(v2f{} + 0.5f));
                        }
                        else if((req->numerical_relation == Numerical_Relation::less_than_equals &&
                            req->relation_target == Array_Lenght(value_option_names) - 1) || 
                            (req->numerical_relation == Numerical_Relation::greater_than_equals &&
                            req->relation_target == 0))
                        {
                            GUI_Do_Title_Text(context, AUTO, L1(always_true), GUI_Scale_Default(v2f{} + 0.5f));
                        }
                        
                    }break;
                }
                
                v2f spacing = v2f{context->layout.last_element.dim.x, context->theme->padding * 2};
                GUI_Do_Spacing(context, &spacing);    
            }
            
            GUI_Pop_Layout(context);
            
            GUI_End_Collumn(context, collumn_min_width, collumn_start, X_AXIS);
            
            if(defer_del)
            {
                Delete_Participent(p--, event->participents, &s_allocator);
            }
        }
    }; // ----------------------------------------------------------------------------------------
    
    Do_GUI_Frame_With_Banner(banner_func, menu_func);
    
    if(s_hotkeys[Editor_Hotkeys::jump_right].Is_Released())
    {
        s_global_data.active_menu = Menus::EE_text;
    }    
}


static void Do_Event_Editor_Text_Frame()
{
    void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        constexpr f32 s = 2.f;
        
        Editor_Event* event = Active_Event(&s_editor_state);
        
        if(GUI_Do_Button(context, &GUI_AUTO_TOP_LEFT, &GUI_AUTO_FIT, "<", GUI_Scale_Default(s)))
        {
            s_global_data.active_menu = Menus::EE_all_events;
        }
        f32 title_height = context->layout.last_element.dim.y;
        
        GUI_Push_Layout(context);
        
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        GUI_Do_Title_Text(context, AUTO, L1(event_text), GUI_Scale_Default(s));
        f32 title_max_x = context->layout.last_element.rect.max.x - context->anchor_base.x;
        GUI_Pop_Layout(context);
        
        
        GUI_Do_Spacing(context, v2f{0, s_post_title_y_spacing});
        {
            char* event_name_text = 
                (s_editor_state.active_event_index < s_editor_state.event_container.day_event_count)?
                L1(event_day_name): L1(event_night_name);
            GUI_Do_Text(context, AUTO, event_name_text);
            
            f32 event_name_sl_field_width = 250 * GUI_DEFAULT_TEXT_SCALE.x;
            GUI_Do_SL_Input_Field(context, AUTO, &event_name_sl_field_width, &event->name);
        }
        
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        // -- title bar buttons --    
        f32 padding = context->theme->padding;
        char* go_to_requirements_text = L1(parti);
        char* go_to_consequences_text = L1(cons);
        
        Font* font = &context->theme->font;
        f32 w1 = GUI_Tight_Fit_Text(go_to_requirements_text, font).x + padding;
        f32 w2 = GUI_Tight_Fit_Text(go_to_consequences_text, font).x + padding;
        
        f32 ctrl_buttons_width = w1 + w2 + context->dynamic_slider_girth + padding * 2;
        v2f title_row_pos 
            = Get_Title_Bar_Row_Placement(context, title_max_x, padding, ctrl_buttons_width);
        
        v2f dim = v2f{w1, title_height};
        if(GUI_Do_Button(context, &title_row_pos, &dim, go_to_requirements_text))
        {
            s_global_data.active_menu = Menus::EE_participants;
        }
        
        dim = v2f{w2, title_height};
        if(GUI_Do_Button(context, AUTO, &dim, go_to_consequences_text))
        {
            s_global_data.active_menu = Menus::EE_consequences;
        }
        
    }; // ----------------------------------------------------------------------------------------

    void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
    {        
        v2f dim = v2u::Cast<f32>(context->canvas->dim) - 50.f;
        if(dim.x >= 0 && dim.y >= 0)
        {
            Editor_Event* event 
                = Begin(s_editor_state.event_container.events) + s_editor_state.active_event_index;
            
            context->layout.anchor = GUI_Anchor::center;
            GUI_Do_ML_Input_Field(context, &GUI_AUTO_MIDDLE, &dim, &event->event_text, 0);
        }
        
    }; // ----------------------------------------------------------------------------------------

    Do_GUI_Frame_With_Banner(banner_func, menu_func);
    
    if(s_hotkeys[Editor_Hotkeys::jump_right].Is_Released())
    {
        s_global_data.active_menu = Menus::EE_consequences;
    }
    
    if(s_hotkeys[Editor_Hotkeys::jump_left].Is_Released())
    {
        s_global_data.active_menu = Menus::EE_participants;
    }
}


static void Do_Event_Editor_Consequences_Frame()
{
    void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        constexpr f32 s = 2.f;
        
        if(GUI_Do_Button(context, &GUI_AUTO_TOP_LEFT, &GUI_AUTO_FIT, "<", GUI_Scale_Default(s)))
        {
            s_global_data.active_menu = Menus::EE_all_events;
        }
        f32 title_height = context->layout.last_element.dim.y;
        
        GUI_Push_Layout(context);
        
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        GUI_Do_Title_Text(context, AUTO, L1(cons), GUI_Scale_Default(s));
        f32 title_max_x = context->layout.last_element.rect.max.x - context->anchor_base.x;
        
        GUI_Pop_Layout(context);
        
        
        GUI_Do_Spacing(context, v2f{0, s_post_title_y_spacing});
        {
            if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(add_global_con)))
            {
                Editor_Event* event 
                    = Begin(s_editor_state.event_container.events) + s_editor_state.active_event_index;
                
                Global_Mark_Consequence* gmc = Push(&event->global_mark_cons, &s_allocator);
                *gmc = {};
                gmc->mark = Create_String(&s_allocator, 4);
            }
        }
        
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        // -- title bar buttons --    
        f32 padding = context->theme->padding;
        
        char* go_to_requirements_text = L1(parti);
        char* go_to_event_text_text = L1(event_text);

        Font* font = &context->theme->font;
        f32 w1 = GUI_Tight_Fit_Text(go_to_requirements_text, font).x + padding;
        f32 w2 = GUI_Tight_Fit_Text(go_to_event_text_text, font).x + padding;
        
        f32 ctrl_buttons_width = w1 + w2 + context->dynamic_slider_girth + padding * 2;
        v2f title_row_pos 
            = Get_Title_Bar_Row_Placement(context, title_max_x, padding, ctrl_buttons_width);
        
        v2f dim = v2f{w1, title_height};
        if(GUI_Do_Button(context, &title_row_pos, &dim, go_to_requirements_text))
        {
            s_global_data.active_menu = Menus::EE_participants;
        }
        
        dim = v2f{w2, title_height};
        if(GUI_Do_Button(context, AUTO, &dim, go_to_event_text_text))
        {
            s_global_data.active_menu = Menus::EE_text;
        }
        
    }; // ----------------------------------------------------------------------------------------

    void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        // CONSIDER: local scope statics are a bit iffy?
        static f64 multiple_death_cons_error_time = 0;
        static u32 multiple_death_cons_error_idx = 0;
        
        static f64 death_con_with_additional_cons_time = 0;
        static u32 death_con_with_additional_cons_idx = 0;
        
        f32 collumn_min_width = 150 * GUI_DEFAULT_TEXT_SCALE.x;
        
        v2f* pos = &GUI_AUTO_TOP_LEFT;
        
        Editor_Event* event = Begin(s_editor_state.event_container.events) + s_editor_state.active_event_index;
        
        if(event->global_mark_cons->count)
        {
            GUI_Do_Text(context, pos, L1(global_cons));
            pos = 0;
            
            GUI_Push_Layout(context);
            
            f32 collumn_start = GUI_Get_Collumn_Start(context, X_AXIS);
            
            Global_Mark_Consequence* global_mark_cons = Begin(event->global_mark_cons);
            for(u32 i = 0; i < event->global_mark_cons->count; ++i)
            {
                GUI_Placement rcp = context->layout.last_element;

                if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "X"))
                {
                    context->layout.last_element = rcp;
                    
                    Delete_Global_Mark_Consequence(event->global_mark_cons, i--);
                    continue;
                }
                
                f32 left_size 
                    = context->layout.last_element.pos.x - context->layout.last_element.dim.x / 2; 
                
                GUI_Push_Layout(context);
                context->layout.build_direction = GUI_Build_Direction::right_center;
                
                GUI_Do_Text(context, AUTO, L1(global_mark), GUI_Highlight_Prev(context));
                
                f32 right_side 
                    = context->layout.last_element.pos.x + context->layout.last_element.dim.x / 2; 
                
                f32 width = right_side - left_size;
                
                GUI_Pop_Layout(context);
                
                
                Global_Mark_Consequence* gmc = global_mark_cons + i;
                GUI_Do_SL_Input_Field(context, AUTO, &width, &gmc->mark);
                
                v2f dim = context->layout.last_element.dim;
                
                GUI_Do_Text(context, AUTO, L1(duration), GUI_Highlight_Next(context));
                
                if(u32 s = GUI_Do_Dropdown_Button(
                    context, 
                    AUTO, 
                    &dim, 
                    u32(gmc->mark_duration),
                    LN(duration_names)))
                {
                    gmc->mark_duration = i8(s - 1);
                }
                
                GUI_Do_Spacing(context, AUTO);
            }
            
            GUI_Pop_Layout(context);
        
            GUI_End_Collumn(context, collumn_min_width, collumn_start, X_AXIS);
            
            context->layout.build_direction = GUI_Build_Direction::right_top;
            
            Rect bounds = GUI_Get_Bounds_In_Pixel_Space(context);
            v2f pannel_dim = v2f{10, bounds.max.y - bounds.min.y};
            GUI_Do_Panel(context, AUTO, &pannel_dim);
        }
        
        GUI_Do_Text(context, pos, L1(parti));

        context->layout.build_direction = GUI_Build_Direction::down_left;
        
        for(u32 i = 0; i < event->participents->count; ++i)
        {
            Participent* parti = Begin(event->participents) + i;
            
            u8 typing_marker_buffer[12 + 2] = {0};
            char* typing_marker;
            {
                typing_marker = U32_To_Char_Buffer((u8*)&typing_marker_buffer[2], i + 1);
                typing_marker -= 1;
                *typing_marker = 'k';
                typing_marker -= 1;
                *typing_marker = '\\';
            }
            
            GUI_Do_Text(context, AUTO, typing_marker);
            
            f32 collumn_start = GUI_Get_Collumn_Start(context, X_AXIS);
            
            context->layout.build_direction = GUI_Build_Direction::down_left;
            GUI_Push_Layout(context);
            
            f64 time = s_platform.Get_Time_Stamp();
            
            bool contains_death_con = false;
            for(Event_Consequens* con = Begin(parti->cons); con < End(parti->cons); ++con)
            {
                if(con->type == Event_Consequens_Type::death)
                {
                    contains_death_con = true;
                    break;
                }
            }
            
            if(u32 s = GUI_Do_Dropdown_Button(
                context, 
                AUTO, 
                &GUI_AUTO_FIT, 
                L1(add_con), 
                LN(con_names)))
            {
                multiple_death_cons_error_time = 0;
                
                if(parti->cons->count < Participent::max_consequenses)
                {
                    bool allow_consequense = true;
                    
                    Event_Consequens_Type con_type = Event_Consequens_Type(s - 1);
                    
                    if(con_type == Event_Consequens_Type::death)
                    {
                        if(contains_death_con)
                        {
                            allow_consequense = false;
                            multiple_death_cons_error_time = time + 5.0;
                            multiple_death_cons_error_idx = i;
                        }
                    }
                    
                    if(allow_consequense)
                        Init_Event_Consequense(Push(&parti->cons, &s_allocator), con_type, &s_allocator);
                }
            }
            
            f32 collumn_button_width = context->layout.last_element.dim.x;
            
            if(multiple_death_cons_error_time != 0 &&
                time <= multiple_death_cons_error_time &&
                multiple_death_cons_error_idx == i)
            {
                GUI_Do_Title_Text(context, AUTO, L1(only_one_death_con_is_allowed), GUI_Scale_Default(0.5f));
            }
            
            if(contains_death_con && parti->cons->count >= 2)
            {
                GUI_Do_Title_Text(context, AUTO, L1(only_death_happens), GUI_Scale_Default(0.5f));
            }
            
            Event_Consequens* cons = Begin(parti->cons);
            for(u32 y = 0; y < parti->cons->count; ++y)
            {
                Event_Consequens* con = cons + y;
                if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "X"))
                {
                    Make_Consequense_Hollow(con);
                    u32 size = sizeof(*con);
                    Remove_Element_From_Packed_Array(cons, &parti->cons->count, size, y--);
                }
                
                GUI_Push_Layout(context);
                context->layout.build_direction = GUI_Build_Direction::right_center;
                
                GUI_Do_Text(context, AUTO, LN1(con_names, u32(con->type)), GUI_Highlight_Prev(context));
                
                GUI_Pop_Layout(context);
                
                switch(con->type)
                {
                    case Event_Consequens_Type::death:
                    {
                        v2f items_inherit_dim = 
                            v2f{context->layout.last_element.dim.y, context->layout.last_element.dim.y};
                        
                        GUI_Do_Checkbox(context, AUTO , &items_inherit_dim, &con->items_are_inherited);
                        
                        GUI_Push_Layout(context);
                        
                        context->layout.build_direction = GUI_Build_Direction::right_center;
                        GUI_Do_Text(context, AUTO, L1(do_items_get_inherited), GUI_Highlight_Prev(context));
                        
                        GUI_Pop_Layout(context);
                        
                        if(con->items_are_inherited)
                        {
                            GUI_Do_Text(context, AUTO, L1(who_inherits), GUI_Highlight_Next(context));
                            
                            GUI_Push_Layout(context);
                            f32 width = 40 * GUI_DEFAULT_TEXT_SCALE.x;
                            context->layout.build_direction = GUI_Build_Direction::right_center;
                            
                            context->flags |= GUI_Context_Flags::one_time_skip_padding;
                            GUI_Do_SL_Input_Field(
                                context, 
                                AUTO, 
                                &width, 
                                &con->str, 
                                3, 
                                GUI_DEFAULT_TEXT_SCALE, 
                                GUI_Character_Check_Numbers_Only);
                            
                            GUI_Do_Text(context, AUTO, "?", GUI_Highlight_Prev(context));
                            GUI_Pop_Layout(context);
                        }
                    }break;
                    
                    case Event_Consequens_Type::stat_change:
                    {    
                        if(u32 s = GUI_Do_Dropdown_Button(
                            context, 
                            AUTO, 
                            &GUI_AUTO_FIT, 
                            u32(con->stat),
                            LN(stat_names)))
                        {
                            con->stat = Character_Stats(s - 1);
                        }
                        
                        GUI_Push_Layout(context);
                        
                        context->layout.build_direction = GUI_Build_Direction::right_center;
                        
                        v2f arith_dim = {36, context->layout.last_element.dim.y};
                        
                        char* selected_option = (con->stat_change_amount < 0)?
                            (char*)s_arithmatic_names[1] : (char*)s_arithmatic_names[0]; 
                        
                        if(u32 s = GUI_Do_Dropdown_Button(
                            context, 
                            AUTO, 
                            &arith_dim, 
                            selected_option, 
                            Array_Lenght(s_arithmatic_names),
                            (char**)s_arithmatic_names))
                        {
                            if(con->stat_change_amount > 0 && s == 2)
                                con->stat_change_amount *= -1;
                            
                            if(con->stat_change_amount < 0 && s == 1)
                                con->stat_change_amount *= -1;
                        }
                        
                        constexpr char* num_options[] = {"1", "2", "3"};
                        u32 abs_stat_amount = Abs(con->stat_change_amount) - 1;
                        
                        Assert(abs_stat_amount < Array_Lenght(num_options));
                        
                        if(u32 s = GUI_Do_Dropdown_Button(
                            context, 
                            AUTO, 
                            &arith_dim, 
                            (char*)num_options[abs_stat_amount], 
                            Array_Lenght(num_options),
                            (char**)num_options))
                        {
                            i32 signed_selection = i32(s);
                            if(con->stat_change_amount < 0)
                                signed_selection *= -1;
                            
                            con->stat_change_amount = signed_selection;
                        }
                        
                        GUI_Pop_Layout(context);
                    }break;
                    
                    
                    case Event_Consequens_Type::gains_mark:
                    {
                        GUI_Do_SL_Input_Field(context, AUTO, &collumn_button_width, &con->str);
                        
                        if(u32 s = GUI_Do_Dropdown_Button(
                            context, 
                            AUTO, 
                            AUTO, 
                            u32(con->mark_type),
                            LN(mark_type_names)))
                        {
                            con->mark_type = Mark_Type(s - 1);
                        }
                        
                        v2f dim = context->layout.last_element.dim;
                        
                        GUI_Do_Text(context, AUTO, L1(duration), GUI_Highlight_Next(context));
                        
                        if(u32 s = GUI_Do_Dropdown_Button(
                            context, 
                            AUTO, 
                            &dim, 
                            u32(con->mark_duration), 
                            LN(duration_names)))
                        {
                            con->mark_duration = i8(s - 1);
                        }
                        
                    }break;
                    
                    case Event_Consequens_Type::loses_mark:
                    {
                        GUI_Do_SL_Input_Field(context, AUTO, &collumn_button_width, &con->str);
                        
                        if(u32 s = GUI_Do_Dropdown_Button(
                            context, 
                            AUTO, 
                            AUTO, 
                            u32(con->mark_type),
                            LN(mark_type_names)))
                        {
                            con->mark_type = Mark_Type(s - 1);
                        }
                        
                    }break;
                }
                
                v2f spacing = v2f{context->layout.last_element.dim.x, context->theme->padding * 2};
                GUI_Do_Spacing(context, &spacing);
            }
            
            GUI_Pop_Layout(context);
            context->layout.build_direction = GUI_Build_Direction::right_top;
            
            GUI_End_Collumn(context, collumn_min_width, collumn_start, X_AXIS);
        }
        
    }; // ----------------------------------------------------------------------------------------

    Do_GUI_Frame_With_Banner(banner_func, menu_func);
    
    if(s_hotkeys[Editor_Hotkeys::jump_left].Is_Released())
    {
        s_global_data.active_menu = Menus::EE_text;
    }
}


static void Do_Event_Editor_Campaigns_Menu_Frame()
{    
    void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        constexpr f32 s = 2.f;
        
        if(GUI_Do_Button(context, &GUI_AUTO_TOP_LEFT, &GUI_AUTO_FIT, "<", GUI_Scale_Default(s)))
        {
            s_global_data.active_menu = Menus::main_menu;
            Clear_Editor_Format_Campaigns();
        }
        
        GUI_Do_Title_Text(context, AUTO, L1(campaing_editor), GUI_Scale_Default(s));
        f32 title_height = context->layout.last_element.dim.y;
        f32 title_max_x = context->layout.last_element.rect.max.x - context->anchor_base.x;
        
        // -- title bar buttons --    
        char* create_new_text = L1(create_new_campaing);
        
        f32 padding = context->theme->padding;
        f32 w1 = GUI_Tight_Fit_Text(create_new_text, &context->theme->font).x + padding;    
        
        f32 ctrl_buttons_width = w1 + context->dynamic_slider_girth + padding * 2;
        
        v2f title_row_pos 
            = Get_Title_Bar_Row_Placement(context, title_max_x, padding, ctrl_buttons_width);
        
        v2f dim = v2f{w1, title_height};
        
        if(GUI_Do_Button(context, &title_row_pos, &dim, create_new_text))
        {           
            Set_Popup_Function(Do_Name_New_Campaign_Popup, Free_New_Campaing_Name);
        }
    
    }; // ----------------------------------------------------------------------------------------

    void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        constexpr f32 s = 0.5f;
        
        char* found_from = L1(found_from_loc);
        GUI_Do_Text(context, &GUI_AUTO_TOP_LEFT, found_from, {}, GUI_Scale_Default(s));
        GUI_Push_Layout(context);
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        context->flags |= GUI_Context_Flags::one_time_skip_padding;
        GUI_Do_Text(context, AUTO, campaign_folder_wildcard_path, {}, GUI_Scale_Default(s));
        
        context->flags |= GUI_Context_Flags::one_time_skip_padding;
        GUI_Do_Text(context, AUTO, L1(where_wildcard_is_campaing_name), {}, GUI_Scale_Default(s));
        
        GUI_Pop_Layout(context);
        GUI_Do_Text(context, AUTO, L1(load_save));
        
        Dynamic_Array<String>* on_disk_names = s_global_data.on_disk_campaign_names;
        if(on_disk_names)
        {
            int i = 0;
            for(each(String*, save_name, on_disk_names))
            {
                if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "X"))
                {
                    s_global_data.campaing_idx_to_delete = i;
                    Set_Popup_Function(Do_Are_You_Sure_You_Want_To_Delete_Campaing_Popup);
                }
                
                GUI_Push_Layout(context);
                
                context->layout.build_direction = GUI_Build_Direction::right_center;
                
                if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, save_name->buffer))
                {
                    Events_Container ec;
                    if(Load_Campaign(&ec, save_name, &s_allocator, &s_platform))
                    {
                        s_editor_state.event_container = ec;
                        s_global_data.active_menu = Menus::EE_all_events;
                    }
                }
                
                GUI_Pop_Layout(context);
                
                i += 1;
            }
        }
        
    }; // ----------------------------------------------------------------------------------------

    Do_GUI_Frame_With_Banner(banner_func, menu_func);
}


static void Do_Event_Editor_On_Exit_Editor(GUI_Context* context)
{
    f32 text_height = GUI_Character_Height(context);
    f32 title_height = text_height * GUI_DEFAULT_TITLE_SCALER.y;
    f32 padding = GUI_Padding(context);
    f32 frame_height = title_height + (text_height + padding * 2) * 3;
    
    v2f title_pos = Get_Middle(context->canvas);
    title_pos.y += frame_height * 0.5f;
    
    GUI_Do_Title_Text(context, &title_pos, L1(want_to_save));
    
    context->layout.build_direction = GUI_Build_Direction::down_center;
    
    char* t1 = L1(cancel);
    char* t2 = L1(save_and_continue);
    char* t3 = L1(continue_without_saving);
    
    char* t = Get_Longest_CSTR(t1, t2, t3);
    v2f button_dim = GUI_Tight_Fit_Text(t, &context->theme->font) + context->theme->padding;
    
    if(GUI_Do_Button(context, AUTO, &button_dim, t1))
    {
        Close_Popup();
    }
    
    if(GUI_Do_Button(context, AUTO, &button_dim, t2))
    {
        Serialize_Campaign(&s_editor_state.event_container, &s_platform);
        
        Delete_Event_Container(&s_editor_state.event_container, &s_allocator);
        
        s_global_data.active_menu = s_editor_state.source;
        
        Close_Popup();
    }
    
    if(GUI_Do_Button(context, AUTO, &button_dim, t3))
    {
        Delete_Event_Container(&s_editor_state.event_container, &s_allocator);
        s_global_data.active_menu = s_editor_state.source;
        
        Close_Popup();
    }
}


static void Do_Event_Editor_Quit_Popup(GUI_Context* context)
{
    f32 text_height = GUI_Character_Height(context);
    f32 title_height = text_height * GUI_DEFAULT_TITLE_SCALER.y;
    f32 padding = GUI_Padding(context);
    f32 frame_height = title_height + (text_height + padding * 2) * 3;
    
    v2f title_pos = Get_Middle(context->canvas);
    title_pos.y += frame_height * 0.5f;
    
    GUI_Do_Title_Text(context, &title_pos, L1(are_you_sure_close));
    
    context->layout.build_direction = GUI_Build_Direction::down_center;
    
    char* t1 = L1(cancel_and_continue);
    char* t2 = L1(save_and_close);
    char* t3 = L1(close_without_saving);
    
    char* t = Get_Longest_CSTR(t1, t2, t3);
    v2f button_dim = GUI_Tight_Fit_Text(t, &context->theme->font) + context->theme->padding;
    
    if(GUI_Do_Button(context, AUTO, &button_dim, t1))
    {
        Close_Popup();
    }
    
    if(GUI_Do_Button(context, AUTO, &button_dim, t2))
    {
        Serialize_Campaign(&s_editor_state.event_container, &s_platform);
        s_platform.Set_Flag(App_Flags::is_running, false);
    }
    
    if(GUI_Do_Button(context, AUTO, &button_dim, t3))
    {
        s_platform.Set_Flag(App_Flags::is_running, false);
    }
}


static void Do_Event_Editor_Delete_Event_Popup(GUI_Context* context)
{
    f32 text_height = GUI_Character_Height(context);
    f32 title_height = text_height * GUI_DEFAULT_TITLE_SCALER.y;
    f32 padding = GUI_Padding(context);
    f32 frame_height = title_height + (text_height + padding * 2) * 2;
    
    v2f title_pos = Get_Middle(context->canvas);
    title_pos.y += frame_height * 0.5f;
    
    GUI_Do_Title_Text(context, &title_pos, L1(want_to_delete));
    
    context->layout.build_direction = GUI_Build_Direction::down_center;
    
    char* t1 = L1(cancel);
    char* t2 = L1(_delete);

    char* t = Get_Longest_CSTR(t1, t2);
    v2f button_dim = GUI_Tight_Fit_Text(t, &context->theme->font) + context->theme->padding;
    
    if(GUI_Do_Button(context, AUTO, &button_dim, t1))
    {
        Close_Popup();
    }
    
    if(GUI_Do_Button(context, AUTO, &button_dim, t2))
    {
        if(s_editor_state.event_idx_to_delete < s_editor_state.event_container.day_event_count)
            s_editor_state.event_container.day_event_count -= 1;
        
        Delete_Editor_Event(
            s_editor_state.event_container.events, 
            &s_allocator, 
            s_editor_state.event_idx_to_delete);
        
        Close_Popup();
    }
}


static void Do_Event_Editor_Display_Active_Event_Errors_Popup(GUI_Context* context)
{
    Editor_Event* event = Active_Event(&s_editor_state);
    
    context->layout.build_direction = GUI_Build_Direction::down_left;
    context->layout.anchor = GUI_Anchor::center;
    
    v2f panel_dim = Hadamar_Product(v2f{450, 250}, GUI_DEFAULT_TEXT_SCALE);
    v2f panel_center = Get_Middle(context->canvas);
    
    GUI_Get_Placement(context, &panel_dim, &GUI_AUTO_MIDDLE);
    
    context->layout.anchor = GUI_Anchor::top_left;
    
    v2f title_pos = panel_center + v2f{panel_dim.x * -1, panel_dim.y} / 2;
    GUI_Do_Title_Text(context, &title_pos, L1(error_report));
    GUI_Do_Text(context, AUTO, L1(event_contains_following_problems));
    
    Canvas sub_canvas;
    GUI_Context* sub_context = Get_GUI_Context_From_Pool();
    
    v2f sub_space_dim = v2f
        {
            panel_dim.x, 
            (context->layout.last_element.pos.y - context->anchor_base.y - 
            context->layout.last_element.dim.y / 2) - (panel_center.y - panel_dim.y / 2)
        };
    
    if(GUI_Do_Sub_Context(
        context, 
        sub_context, 
        &sub_canvas, 
        AUTO, 
        &sub_space_dim, 
        &s_settings.list_bg_color))
    {
        v2f* pos = &GUI_AUTO_TOP_LEFT;
        
        if(event->issues.errors)
        {
            GUI_Do_Title_Text(sub_context, pos, L1(errors), GUI_DEFAULT_TEXT_SCALE);
            pos = 0;
            
            CSTR_List error_names = LN(event_error_names);
            
            for(u32 i = 0; i < error_names.count; ++i)
            {
                if(event->issues.errors & (1 << i))
                    GUI_Do_Text(sub_context, AUTO, error_names.list[i]);
            }
        }
        
        if(event->issues.warnings)
        {
            if(!pos)
            {
                GUI_Do_Spacing(sub_context, v2f{0.f, sub_context->layout.last_element.dim.y});
            }
            
            GUI_Do_Title_Text(sub_context, pos, L1(warnings), GUI_DEFAULT_TEXT_SCALE);
            
            CSTR_List warning_names = LN(event_warning_names);
            
            for(u32 i = 0; i < warning_names.count; ++i)
            {
                if(event->issues.warnings & (1 << i))
                    GUI_Do_Text(sub_context, AUTO, warning_names.list[i]);
            }        
        }
        
        GUI_End_Context(sub_context);
    }
    
    
    context->layout.anchor = GUI_Anchor::top_right;
    
    v2f close_pos = panel_center + panel_dim / 2;
    if(GUI_Do_Button(context, &close_pos, &GUI_AUTO_FIT, "X"))
    {
        Close_Popup();
    }
}


static void Do_New_Name_Already_In_Use_Popup(GUI_Context* context)
{
    GUI_Do_Title_Text(context, &GUI_AUTO_MIDDLE, L1(name_in_use));
    if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(ok)))
    {
        Set_Popup_Function(Do_Name_New_Campaign_Popup, Free_New_Campaing_Name);
    }
}


static void Do_Rename_Name_Already_In_Use_Popup(GUI_Context* context)
{
    GUI_Do_Title_Text(context, &GUI_AUTO_MIDDLE, L1(name_in_use));
    if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(ok)))
    {
        Set_Popup_Function(Do_Rename_Campaing_Popup, Free_New_Campaing_Name);
    }
}


static void Do_Name_New_Campaign_Popup(GUI_Context* context)
{
    GUI_Do_Title_Text(context, &GUI_AUTO_MIDDLE, L1(give_camaping_a_name));
    
    char* def_name = L1(new_campaing);
    if(!s_global_data.new_campaign_name.buffer)
    {
        Init_String(&s_global_data.new_campaign_name, &s_allocator, def_name);
    }
    
    bool force_create = GUI_Do_SL_Input_Field(
        context, 
        AUTO, 
        AUTO, 
        &s_global_data.new_campaign_name);
    
    
    v2f last_element_dim = context->layout.last_element.dim;
    last_element_dim.x -= context->theme->padding;
    last_element_dim.x *= 0.5f;
    
    if(GUI_Do_Button(context, AUTO, &last_element_dim, L1(create)) || force_create)
    {
        bool name_not_in_use = true;
        
        for(each(String*, on_disk_name, s_global_data.on_disk_campaign_names))
        {
            if(String_Compare(on_disk_name, &s_global_data.new_campaign_name))
            {
                name_not_in_use = false;
                break;
            }
        }
        
        if(name_not_in_use)
        {
            s_global_data.active_menu = Menus::EE_all_events;
            
            Init_Event_Container_Takes_Name_Ownership(
                &s_editor_state.event_container, 
                &s_allocator, 
                &s_global_data.new_campaign_name);
            
            Serialize_Campaign(&s_editor_state.event_container, &s_platform);
            
            Close_Popup();            
        }
        else
        {
            Set_Popup_Function(Do_New_Name_Already_In_Use_Popup);
        }
    }
    
    context->layout.build_direction = GUI_Build_Direction::right_center;
    
    if(GUI_Do_Button(context, AUTO, &last_element_dim, L1(cancel)))
        Close_Popup();
}


static void Do_Are_You_Sure_You_Want_To_Delete_Campaing_Popup(GUI_Context* context)
{
    GUI_Do_Title_Text(context, &GUI_AUTO_MIDDLE, L1(want_to_delete));
    GUI_Do_Text(context, AUTO, L1(campaing_deletion_is_permanent));
    
    context->layout.build_direction = GUI_Build_Direction::down_center;
    
    char* t1 = L1(cancel);
    char* t2 = L1(_delete);
    
    char* t = Get_Longest_CSTR(t1, t2);
    v2f button_dim = GUI_Tight_Fit_Text(t, &context->theme->font) + context->theme->padding;
   
    if(GUI_Do_Button(context, AUTO, &button_dim, t1))
    {
        Close_Popup();
    }
    
    if(GUI_Do_Button(context, AUTO, &button_dim, t2))
    {
        String* save_names = Begin(s_global_data.on_disk_campaign_names);
        
        String full_path = Create_Campaign_Full_Path(
            save_names + s_global_data.campaing_idx_to_delete,
            &s_platform,
            &s_allocator);

        s_platform.Delete_File(full_path.buffer);
        
        full_path.free();
        
        Gather_Editor_Format_Campaigns();
        
        Close_Popup();
    }
}


static void Do_Rename_Campaing_Popup(GUI_Context* context)
{
   GUI_Do_Title_Text(context, &GUI_AUTO_MIDDLE, L1(rename_campaign));

    if(!s_global_data.new_campaign_name.buffer)
    {
        Deep_Copy_String(
            &s_global_data.new_campaign_name, 
            &s_editor_state.event_container.campaign_name);
    }
    
    bool force_create = GUI_Do_SL_Input_Field(
        context, 
        AUTO, 
        AUTO, 
        &s_global_data.new_campaign_name);
    
    
    v2f last_element_dim = context->layout.last_element.dim;
    last_element_dim.x -= context->theme->padding;
    last_element_dim.x *= 0.5f;
    
    if(GUI_Do_Button(context, AUTO, &last_element_dim, L1(confirm)) || force_create)
    {
        bool name_not_in_use = true;
        
        Dynamic_Array<String>* on_disk_names = s_global_data.on_disk_campaign_names;
        if(!on_disk_names)
            Gather_Editor_Format_Campaigns();
        
        for(each(String*, on_disk_name, s_global_data.on_disk_campaign_names))
        {
            if(String_Compare(on_disk_name, &s_global_data.new_campaign_name))
            {
                name_not_in_use = false;
                break;
            }
        }
        
        if(!on_disk_names)
            Clear_Editor_Format_Campaigns();
        
        if(name_not_in_use)
        {
            String full_path_old = Create_Campaign_Full_Path(
                &s_editor_state.event_container.campaign_name,
                &s_platform,
                &s_allocator);

            String full_path_new = Create_Campaign_Full_Path(
                &s_global_data.new_campaign_name,
                &s_platform,
                &s_allocator);

            if(s_platform.Move_File(full_path_old.buffer, full_path_new.buffer))
            {
                Deep_Copy_String(
                    &s_editor_state.event_container.campaign_name,
                    &s_global_data.new_campaign_name);
            }
            else
            {
                // TODO: Handle error, with popup or something.
            }
            
            full_path_old.free();
            full_path_new.free();
            
            Close_Popup();
        }
        else
        {
            Set_Popup_Function(Do_Rename_Name_Already_In_Use_Popup);
        }
    }
    
    context->layout.build_direction = GUI_Build_Direction::right_center;
    
    if(GUI_Do_Button(context, AUTO, &last_element_dim, L1(cancel)))
        Close_Popup();
}
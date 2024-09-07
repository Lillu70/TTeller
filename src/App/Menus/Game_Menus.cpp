

// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#pragma once

constexpr f32 s_player_creation_collumn_min_width_base = 150;
constexpr f32 s_player_creation_text_box_width_base = s_player_creation_collumn_min_width_base - 25;
constexpr v2f s_player_picture_dim_base
    = v2f{s_player_creation_text_box_width_base, s_player_creation_text_box_width_base};


static void Do_Load_Failed_Popup(GUI_Context* context);
static void Free_Invalid_Event_Filter_Result_Memory();
static void Do_Display_Invalid_Event_Filter_Results_Popup(GUI_Context* context);
static void Do_GM_Campaign_Was_Unusable_Popup(GUI_Context* context);


static void Do_Create_Player_FI_Instruction_Popup(GUI_Context* context)
{
    char* instruction_text_cstr = 
        "Here is another video inspired by the Deus Ex series!\n"
        "I hope you all enjoy it.\n"
        "I love making music like this that has more tense rhythmic elements,\n"
        "as it\'s closer to a lot of the music I compose for my full time work.\n"
        "This track feels like the music,\n"
        "that plays when you are stealthily sneaking around hostile territory,\n"
        "possibly to hack into a system or quietly take down a target.\n" 
        "I hope it helps you feel energized and productive!\n"
        "Please help my channel by sharing, liking, commenting and subscribing!\n"
        "\n\nSee you in the next one";
    
    String instruction_text = {
        instruction_text_cstr, 
        0, 
        Null_Terminated_Buffer_Lenght(instruction_text_cstr),
        0};
    
    v2f title_scale = Hadamar_Product(GUI_DEFAULT_TEXT_SCALE, GUI_DEFAULT_TITLE_SCALER);
    
    char* title_text = L1(player_creation_instructions);
    v2f d = GUI_Tight_Fit_Text(title_text, &context->theme->font, title_scale);
    v2f b = v2f{d.y, d.y};
    
    context->layout.anchor = GUI_Anchor::center;
    
    v2f dim = v2f{d.x + context->theme->padding + b.x, 200.f * GUI_DEFAULT_TEXT_SCALE.x};
    GUI_Do_ML_Input_Field(
        context, 
        &GUI_AUTO_MIDDLE, 
        &dim, 
        &instruction_text,
        GUI_NO_CHARACTER_LIMIT,
        GUI_DEFAULT_TEXT_SCALE,
        GUI_Character_Check_View_Only);
    
    context->layout.build_direction = GUI_Build_Direction::up_left;
    GUI_Do_Title_Text(context, AUTO, title_text);
    
    context->layout.build_direction = GUI_Build_Direction::right_center;
    if(GUI_Do_Button(context, AUTO, &b, "X"))
        Close_Popup();
    
    // TODO: Check if this line is neccecary! --- LATER: Why would it be? We're done with the pop?
    context->layout.build_direction = GUI_Build_Direction::down_center;
}


static void Do_Out_Of_Template_Names_Popup(GUI_Context* context)
{
    GUI_Do_Title_Text(
        context, 
        &GUI_AUTO_MIDDLE, 
        L1(ran_out_of_premade_names));
    
    context->layout.build_direction = GUI_Build_Direction::down_center;
    
    if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(sad_confirm)))
        Close_Popup();
}


static void Do_New_Game_Players()
{
    void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        constexpr f32 s = 2.f;
        
        if(GUI_Do_Button(context, &GUI_AUTO_TOP_LEFT, &GUI_AUTO_FIT, "<", GUI_Scale_Default(s)))
        {
            if(s_game_state.memory)
                Delete_Game(&s_game_state, &s_allocator);
            
            s_global_data.active_menu = Menus::select_campaign_to_play_menu;
            return;
        }
        f32 title_height = context->layout.last_element.dim.y;
        
        GUI_Push_Layout(context);
        
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        GUI_Do_Title_Text(context, AUTO, L1(players), GUI_Scale_Default(s));
        
        f32 title_max_x = context->layout.last_element.rect.max.x - context->anchor_base.x;
        
        GUI_Pop_Layout(context);
        
        GUI_Do_Spacing(context, v2f{0, s_post_title_y_spacing});
        
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(add_player)))
        {
            Create_Player_User_Data(&s_game_state, &s_allocator);
            *Push(&s_game_state.player_images, &s_allocator) = {};
            
            s_gui.flags |= GUI_Context_Flags::maxout_horizontal_slider;
        }
        
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(instuction)))
        {
            Set_Popup_Function(Do_Create_Player_FI_Instruction_Popup);
        }
        
        if(s_game_state.player_images->count)
        {
            if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(fill_empty_spots)))
            {
                s_game_state.rm.seed = (i32)s_platform.Get_Time_Stamp();
                
                if(!Fill_Empty_Names(&s_game_state, &s_allocator))
                {
                    Set_Popup_Function(Do_Out_Of_Template_Names_Popup);
                }
            }            
        }
        
        GUI_Do_Text(context, AUTO, "Siemennys:", GUI_Highlight_Next(context));
        context->flags |= GUI_Context_Flags::one_time_skip_padding;
        
        f32 width = 50 * GUI_DEFAULT_TEXT_SCALE.x;
        GUI_Do_SL_Input_Field(
            context, 
            AUTO, 
            &width, 
            &s_game_state.seed_input, 
            11, 
            GUI_DEFAULT_TEXT_SCALE,
            GUI_Character_Check_Numbers_Only);
        
        // -- title bar buttons --    
        f32 padding = context->theme->padding;
        
        char* start_game_text = L1(ready);
        
        f32 w1 = GUI_Tight_Fit_Text(start_game_text, &context->theme->font).x + padding;
        
        f32 buttons_width = w1 + context->dynamic_slider_girth + padding * 3;
        v2f title_row_pos = Get_Title_Bar_Row_Placement(context, title_max_x, padding, buttons_width);
        
        v2f dim = v2f{w1, title_height};
        
        b32 live_player_count = s_game_state.player_images->count; 
        if(live_player_count > 1 && GUI_Do_Button(context, &title_row_pos, &dim, start_game_text))
            s_global_data.active_menu = Menus::GM_let_the_games_begin;
        
    }; // ----------------------------------------------------------------------------------------

    void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        if(!s_game_state.memory)
            return;
        
        Player_Image* player_images = Begin(s_game_state.player_images);
        
        v2f* pos = &GUI_AUTO_TOP_LEFT;
        
        context->layout.build_direction = GUI_Build_Direction::right_top;
        
        f32 player_creation_text_box_width 
            = s_player_creation_text_box_width_base * GUI_DEFAULT_TEXT_SCALE.x;
        
        v2f player_picture_dim = Hadamar_Product(s_player_picture_dim_base, GUI_DEFAULT_TEXT_SCALE);
        
        u32 i = 0;
        for(each(Player_UD*, user_data, s_game_state.player_ud))
        {
            Assert(i < s_game_state.player_images->count);
            Player_Image* player_image = player_images + i; 
            Image* img = &(player_image)->image;
            
            bool defer_del = false;
            
            if(GUI_Do_Button(context, pos, &GUI_AUTO_FIT, "X"))
            {
                defer_del = true;
            }
            
            pos = 0;
            
            f32 collumn_start = GUI_Get_Collumn_Start(context, X_AXIS);
            
            GUI_Push_Layout(context);
            context->layout.build_direction = GUI_Build_Direction::right_center;
            
            GUI_Do_Text(context, AUTO, L1(player));
            
            GUI_Pop_Layout(context);
            
            GUI_Push_Layout(context);
            context->layout.build_direction = GUI_Build_Direction::down_left;
            
            GUI_Do_Text(context, AUTO, L1(name), GUI_Highlight_Next(context));
            if(GUI_Do_SL_Input_Field(context, AUTO, &player_creation_text_box_width, &user_data->full_name))
            {
                context->selected_index += 1;
                if(!user_data->variant_name_1.lenght)
                {
                    Reserve_String_Memory(&user_data->variant_name_1, user_data->full_name.lenght + 1, false);
                    Mem_Copy(
                        user_data->variant_name_1.buffer, 
                        user_data->full_name.buffer, 
                        user_data->full_name.lenght + 1);
                    
                    user_data->variant_name_1.lenght = user_data->full_name.lenght;
                }
                
                if(!user_data->variant_name_2.lenght)
                {
                    Reserve_String_Memory(&user_data->variant_name_2, user_data->full_name.lenght + 1, false);
                    Mem_Copy(
                        user_data->variant_name_2.buffer, 
                        user_data->full_name.buffer, 
                        user_data->full_name.lenght + 1);
                    
                    user_data->variant_name_2.lenght = user_data->full_name.lenght;
                }
            }
            
            if(s_settings.language == Language::finnish)
            {
                GUI_Do_Text(context, AUTO, L1(name_form_1), GUI_Highlight_Next(context));
                
                if(GUI_Do_SL_Input_Field(
                    context, 
                    AUTO, 
                    &player_creation_text_box_width, 
                    &user_data->variant_name_1))
                {
                    context->selected_index += 1;
                }
                
                GUI_Do_Text(context, AUTO, L1(name_form_2), GUI_Highlight_Next(context));
                
                GUI_Do_SL_Input_Field(
                    context, 
                    AUTO, 
                    &player_creation_text_box_width, 
                    &user_data->variant_name_2);
            
                
                v2f checkbox_dim = v2f{1.f, 1.f} * context->layout.last_element.dim.y;
                GUI_Do_Checkbox(context, AUTO, &checkbox_dim, &user_data->special_char_override);
                
                GUI_Push_Layout(context);
                
                context->layout.build_direction = GUI_Build_Direction::right_center;
                
                GUI_Do_Text(context, AUTO, L1(umlauts_override), GUI_Highlight_Prev(context));
                
                GUI_Pop_Layout(context);
            }
            
            GUI_Do_Text(context, AUTO, L1(gender));
            if(u32 s = GUI_Do_Dropdown_Button(context, AUTO, &GUI_AUTO_FIT, u32(user_data->gender), LN(genders)))
            {
                user_data->gender = Gender(s - 1);
            }
            
            if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(choose_image)))
            {
                char path[260];
                
                char* filters[] = {"png", "jpg", "bmp"};
                
                if(s_platform.Open_Select_File_Dialog(
                    path, 
                    Array_Lenght(path), 
                    L1(choose_image), 
                    L1(image), 
                    filters,
                    Array_Lenght(filters)))
                {
                    Image loaded_img;
                    if(Load_Image_Raw(&loaded_img, path, &s_platform))
                    {
                        if(img->buffer)
                        {
                            Assert(player_image->file_path.buffer);
                            
                            player_image->file_path.free();
                            s_allocator.free(img->buffer);
                        }
                        
                        img->dim = player_picture_dim.As<i32>();
                        u32 sm_size = img->dim.x * img->dim.y * sizeof(Color);
                        
                        img->buffer = (u8*)s_allocator.push(sm_size);
                    
                        Resize_Image(img, &loaded_img);
                        s_allocator.free(loaded_img.buffer);
                    
                        Convert_From_RGB_To_Color_And_Flip_Y(img);
                        
                        Init_String(&player_image->file_path, &s_allocator, path);
                    }
                }
            }
            
            GUI_Push_Layout(context);
            
            context->layout.build_direction = GUI_Build_Direction::right_center;
            if(img->buffer && GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "X"))
            {
                Assert(player_image->file_path.buffer);
                
                s_allocator.free(img->buffer);
                player_image->file_path.free();
                img->buffer = 0;
            }
            
            GUI_Pop_Layout(context);
            
            GUI_Do_Image_Panel(context, AUTO, &player_picture_dim, img);
            
            GUI_Pop_Layout(context);
            
            GUI_End_Collumn(
                context, 
                s_player_creation_collumn_min_width_base * GUI_DEFAULT_TEXT_SCALE.x, 
                collumn_start, 
                X_AXIS);
             
             
            if(defer_del)
            {
                Dynamic_Array<Player_UD>* user_data_array = s_game_state.player_ud;
                
                Assert(user_data_array->count == s_game_state.player_images->count);
                
                Hollow_Player_User_Data(user_data);
                Remove_Element_From_Packed_Array(
                    Begin(user_data_array), 
                    &user_data_array->count, 
                    sizeof(*user_data), 
                    i);
                
                if(img->buffer)
                    s_allocator.free(img->buffer);
                
                player_image->file_path.free();
                u32* img_count = &s_game_state.player_images->count;
                Remove_Element_From_Packed_Array(player_images, img_count, sizeof(*player_images), i);
             
                user_data -= 1;
            }
            else
            {
                i += 1;
            }
        }
        
    }; // ----------------------------------------------------------------------------------------

    Do_GUI_Frame_With_Banner(banner_func, menu_func);
}


static void Do_Select_Campagin_To_Play_Frame()
{
    void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        constexpr f32 s = 2.f;
        
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        if(GUI_Do_Button(context, &GUI_AUTO_TOP_LEFT, &GUI_AUTO_FIT, "<", GUI_Scale_Default(s)))
        {
            s_global_data.active_menu = Menus::main_menu;
            Clear_Editor_Format_Campaigns();
        }
        
        GUI_Do_Title_Text(context, AUTO, L1(new_game), GUI_Scale_Default(s));
        f32 title_height = context->layout.last_element.dim.y;
        f32 title_max_x = context->layout.last_element.rect.max.x - context->anchor_base.x;
    
    }; // ----------------------------------------------------------------------------------------

    void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        GUI_Do_Text(context, &GUI_AUTO_TOP_LEFT, L1(found_from_loc), {}, GUI_Scale_Default(.5f));
        GUI_Push_Layout(context);
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        context->flags |= GUI_Context_Flags::one_time_skip_padding;
        GUI_Do_Text(context, AUTO, s_campaign_folder_wildcard_path, {}, GUI_Scale_Default(.5f));
        
        context->flags |= GUI_Context_Flags::one_time_skip_padding;
        GUI_Do_Text(context, AUTO, L1(where_wildcard_is_campaing_name), {}, GUI_Scale_Default(.5f));
        
        GUI_Pop_Layout(context);
        GUI_Do_Text(context, AUTO, L1(choose_campaign));
        
        if(s_global_data.on_disk_campaign_names)
        {
            for(each(String*, save_name, s_global_data.on_disk_campaign_names))
            {
                if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, save_name->buffer))
                {
                    Events_Container editor_format_campagin = {};
                    if(Load_Campaign(&editor_format_campagin, save_name, &s_allocator, &s_platform))
                    {
                        Dynamic_Array<Invalid_Event_Filter_Result>* filter_results = 
                            Unordered_Filter_Prolematic_Events(&editor_format_campagin, &s_allocator);
                        
                        if(filter_results)
                        {
                            Set_Popup_Function(
                                Do_Display_Invalid_Event_Filter_Results_Popup, 
                                Free_Invalid_Event_Filter_Result_Memory);
                            
                            Assert(!s_global_data.IEFR);
                            s_global_data.IEFR = filter_results;
                            s_editor_state.event_container = editor_format_campagin;
                        }
                        else
                        {
                            Game_State game_state;
                            
                            u32 errors = Convert_Editor_Campaign_Into_Game_Format(
                                &game_state,
                                &editor_format_campagin,
                                &s_platform,
                                &s_allocator);
                            
                            if(!errors)
                            {
                                if(s_game_state.memory)
                                    Delete_Game(&s_game_state, &s_allocator);
                                
                                s_game_state = game_state;
                                
                                s_global_data.active_menu = Menus::GM_players;
                            }
                            else
                            {
                                s_global_data.GM_conversion_errors = errors;
                                Set_Popup_Function(Do_GM_Campaign_Was_Unusable_Popup);
                            }

                            Delete_Event_Container(&editor_format_campagin, &s_allocator);
                        }
                    }
                    else
                    {
                        Set_Popup_Function(Do_Load_Failed_Popup);
                    }
                }
            }
        }
        
    }; // ----------------------------------------------------------------------------------------

    Do_GUI_Frame_With_Banner(banner_func, menu_func);
}


static void Do_Let_The_Games_Begin_Frame()
{
    void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        constexpr f32 s = 2.f;
        
        context->layout.anchor = GUI_Anchor::top;
        context->layout.build_direction = GUI_Build_Direction::down_center;
        
        GUI_Do_Title_Text(context, &GUI_AUTO_TOP_CENTER, s_game_state.campaign_name, GUI_Scale_Default(s));
    
        GUI_Do_Text(context, AUTO, L1(partis_compete_to_death));
        GUI_Do_Text(context, AUTO, L1(there_is_only_one_winner));
        
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(start_game)))
        {
            s_global_data.active_menu = Menus::GM_day_counter;
            Begin_Game(&s_game_state, &s_platform, &s_allocator);
        }
        
        context->layout.anchor = GUI_Anchor::top_left;
        v2f back_button_dim = GUI_Tight_Fit_Text("<", &context->theme->font, GUI_Scale_Default(s));
        if(GUI_Do_Button(context, &GUI_AUTO_TOP_LEFT, &back_button_dim, "<"))
        {
            s_global_data.active_menu = Menus::GM_players;
        }
        
    }; // ----------------------------------------------------------------------------------------

    void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        context->layout.anchor = GUI_Anchor::top;
        context->layout.build_direction = GUI_Build_Direction::down_center;
        
        v2f* p = &GUI_AUTO_TOP_CENTER;
        
        Player_Image* player_images = Begin(s_game_state.player_images);
        Player_UD* user_data_array = Begin(s_game_state.player_ud);
        
        v2f player_picture_dim = Hadamar_Product(s_player_picture_dim_base, GUI_DEFAULT_TEXT_SCALE);
        
        for(u32 i = 0; i < s_game_state.player_images->count; ++i)
        {
            char* name = (user_data_array + i)->full_name.buffer;
            Image* img = &(player_images + i)->image;
            
            GUI_Do_Text(context, p, name);
            p = AUTO;
        
            GUI_Do_Image_Panel(context, AUTO, &player_picture_dim, img);
        }
    }; // ----------------------------------------------------------------------------------------

    Do_GUI_Frame_With_Banner(banner_func, menu_func);
}


static void Do_Event_Display_Frame()
{
    Assert(s_game_state.live_player_count);
    
    static Event* active_event;
    static Event_Header* active_event_header;
    static bool skip_frame;
    
    skip_frame = false;
    
    Get_Active_Event_Ref_And_Header(
        &s_game_state, 
        s_game_state.display_event_idx, 
        &active_event, 
        &active_event_header);    
    
    void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        v2f* p = &GUI_AUTO_TOP_LEFT;
        
        bool not_last_event_in_set = 
            s_game_state.display_event_idx < s_game_state.active_events->count - 1;
        
        if(not_last_event_in_set)
        {
            if(GUI_Do_Button(context, p, &GUI_AUTO_FIT, L1(next_event)))
            {
                skip_frame = true;
                s_game_state.display_event_idx += 1;
                Generate_Display_Text(&s_game_state);
            }
            p = 0;
            GUI_Push_Layout(context);
        }
        
        char* t = (not_last_event_in_set)? L1(skip_set) : L1(_continue);
        if(GUI_Do_Button(context, p, &GUI_AUTO_FIT, t))
        {
            Resolve_Current_Event_Set(&s_game_state, &s_allocator);

            if(!s_game_state.live_player_count)
            {
                s_global_data.active_menu = Menus::GM_everyone_is_dead;
            }
            else if(s_game_state.live_player_count == 1)
            {
                s_global_data.active_menu = Menus::GM_we_have_a_winner;
            }
            else
            {
                switch(s_game_state.active_event_list)
                {
                    case Event_List::day:
                    {
                        s_global_data.active_menu = Menus::GM_night_falls;
                        f64 time = s_platform.Get_Time_Stamp();
                        s_game_state.night_falls_start_time = time;
                    }break;
                    
                    case Event_List::night:
                    {
                        s_game_state.day_counter += 1;
                        Tickdown_Marks(&s_game_state);
                        s_global_data.active_menu = Menus::GM_day_counter;
                    }break;
                }
            }
            
            while(context->layout_stack_count)
                GUI_Pop_Layout(context);
            
            skip_frame = true;
            return;
        }
        
        while(context->layout_stack_count)
            GUI_Pop_Layout(context);
        
        context->layout.build_direction = GUI_Build_Direction::down_center;
        context->layout.anchor = GUI_Anchor::top;
        
        v2f player_picture_dim = Hadamar_Product(s_player_picture_dim_base, GUI_DEFAULT_TEXT_SCALE);
        
        Player_Image* player_images = Begin(s_game_state.player_images);
        
        f32 padding = f32(context->theme->padding);
        
        v2f name_pos = v2f{padding, context->layout.last_element.pos.y};
        name_pos.y -= context->layout.last_element.dim.y / 2 + padding; 
        
        f32 min_half_lenght = player_picture_dim.x / 2.f;
        
        for(u32 i = 0; i < active_event->participant_count; ++i)
        {
            u32 player_idx = active_event->player_indices[i];
            Player_UD* user_data = Begin(s_game_state.player_ud) + player_idx;
            
            f32 name_lenght = GUI_Character_Width(context) * f32(user_data->full_name.lenght);
            f32 half_lenght = Max(name_lenght / 2.f, min_half_lenght);
            
            name_pos.x += half_lenght;
            
            GUI_Do_Text(context, &name_pos, user_data->full_name.buffer);
            
            name_pos.x += half_lenght + padding;            
            
            Player_Image* player_image = player_images + player_idx;
            GUI_Do_Image_Panel(context, AUTO, &player_picture_dim, &player_image->image);
        }
        
    }; // ----------------------------------------------------------------------------------------

    void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        if(skip_frame)
            return;
        
        v2f text_box_dim = v2f{f32(context->canvas->dim.x - 1), f32(context->canvas->dim.y - 1)};
        text_box_dim -= 50.f * GUI_DEFAULT_TEXT_SCALE;
        
        if(text_box_dim.x > 0 && text_box_dim.y > 0)
        {
            context->layout.anchor = GUI_Anchor::center;
            
            GUI_Do_ML_Input_Field(
                context, 
                &GUI_AUTO_MIDDLE, 
                &text_box_dim,
                &s_game_state.display_text, 
                0, 
                GUI_DEFAULT_TEXT_SCALE,
                GUI_Character_Check_View_Only);
        }
    }; // ----------------------------------------------------------------------------------------

    Do_GUI_Frame_With_Banner(banner_func, menu_func);
}


static void Do_Day_Counter_Display_Frame()
{
    void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        constexpr f32 s = 2.f;
        
        context->layout.anchor = GUI_Anchor::top;
        context->layout.build_direction = GUI_Build_Direction::down_center;
        
        GUI_Do_Title_Text(context, &GUI_AUTO_TOP_CENTER, L1(day), GUI_Scale_Default(s));
        v2f title_dim = context->layout.last_element.dim;
        
        char index_text_buffer[12] = {0};
        char* num = U32_To_Char_Buffer((u8*)&index_text_buffer, s_game_state.day_counter);
        GUI_Do_Text(context, AUTO, num, {},  GUI_Scale_Default(s));
        
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(_continue)))
        {
            if(!Assign_Events_To_Participants(&s_game_state, Event_List::day, &s_allocator))
            {
                s_global_data.active_menu = Menus::GM_event_assignement_failed;
            }
            else
            {
                s_global_data.active_menu = Menus::GM_event_display;
            }
        }
        
        context->layout.anchor = GUI_Anchor::top_right;
        
        
        
        if(u32 s = GUI_Do_Dropdown_Button(
            context, 
            &GUI_AUTO_TOP_RIGHT, 
            &GUI_AUTO_FIT,
            L1(game_actions),
            LN(game_actions)))
        {
            switch(s - 1)
            {
                case 0: // Quit
                {
                    Delete_Game(&s_game_state, &s_allocator);
                    s_global_data.active_menu = Menus::select_campaign_to_play_menu;
                    return;
                }break;
                
                case 1: // Reset
                {
                    Reset_Game(&s_game_state, &s_allocator);
                    s_global_data.active_menu = Menus::GM_let_the_games_begin;
                }break;
            }
        }
        
    }; // ----------------------------------------------------------------------------------------

    void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        if(s_global_data.active_menu != Menus::GM_day_counter)
            return;
        
        Assert(s_game_state.memory);
        
        context->layout.anchor = GUI_Anchor::top;
        context->layout.build_direction = GUI_Build_Direction::down_center;
        
        GUI_Do_Text(context, &GUI_AUTO_TOP_CENTER, L1(active_global_marks));
        
        GUI_Push_Layout(context);
        
        context->layout.build_direction = GUI_Build_Direction::left_center;
        
        v2f checkbox_dim = v2f{} + GUI_Character_Height(context);
        GUI_Do_Checkbox(context, AUTO, &checkbox_dim, &s_game_state.show_global_marks);
        
        GUI_Pop_Layout(context);
        
        char num_text_buffer[12] = {0};
        
        if(s_game_state.show_global_marks)
        {
            for(each(Mark_GM*, gmark, s_game_state.global_marks))
            {
                u32 offset = *(((u32*)s_game_state.mark_table.memory) + gmark->idx);
                char* mark_text = s_game_state.mark_data + offset;
                GUI_Do_Text(context, AUTO, mark_text);
                
                GUI_Push_Layout(context);
                
                if(gmark->duration)
                {
                    context->flags |= GUI_Context_Flags::one_time_skip_padding;
                    GUI_Do_Text(context, AUTO, ":");
                    
                    context->layout.build_direction = GUI_Build_Direction::right_center;
                    char* num = U32_To_Char_Buffer((u8*)&num_text_buffer, gmark->duration);
                    GUI_Do_Text(context, AUTO, num);
                }
                
                GUI_Pop_Layout(context);
            }            
        }
        
        GUI_Do_Title_Text(
            context, 
            AUTO, 
            L1(remaining_partis), 
            GUI_Scale_Default(1.5f));
        
        f32 padding = f32(context->theme->padding);
        
        v2f player_picture_dim = Hadamar_Product(s_player_picture_dim_base, GUI_DEFAULT_TEXT_SCALE);
        
        f32 f = 25.f * GUI_DEFAULT_TEXT_SCALE.x;
        v2f d = v2f
        {
            player_picture_dim.x + padding * 2 + context->dynamic_slider_girth * 2 + f, 
            Max(250.f * GUI_DEFAULT_TEXT_SCALE.y, 
            Min(f32(context->canvas->dim.y - 1) * 0.8f, 400.f * GUI_DEFAULT_TEXT_SCALE.y))
        };
        
        f32 total_width = (d.x + padding) * s_game_state.live_player_count - padding;
        
        v2f p = context->layout.last_element.pos - context->anchor_base;
        p.y -= (context->layout.last_element.dim.y / 2) + (padding * 3) + GUI_DEFAULT_TEXT_SCALE.y;
        p.x -= total_width / 2;
        
        GUI_Context* sub_context_a = Get_GUI_Context_From_Pool();
        GUI_Context* sub_context_b = Get_GUI_Context_From_Pool();
        
        Player_Image* images = Begin(s_game_state.player_images);
        Player_UD* user_data_array = Begin(s_game_state.player_ud);

        context->layout.anchor = GUI_Anchor::top_left;
        context->layout.build_direction = GUI_Build_Direction::down_left;
        

        for(u32 i = 0; i < s_game_state.live_player_count; ++i)
        {
            Game_Player* player = s_game_state.players + i;
            
            GUI_Context* sub_context = (i == s_game_state.active_player_card_idx)? 
                sub_context_a : sub_context_b;
            
            bool sub_context_is_active = false;
            Canvas sub_canvas;
            if(GUI_Do_Sub_Context(context, sub_context, &sub_canvas, &p, &d))
            {
                sub_context->layout.anchor = GUI_Anchor::top;
                sub_context->layout.build_direction = GUI_Build_Direction::down_center;
                
                GUI_Do_Title_Text(sub_context, &GUI_AUTO_TOP_CENTER, (user_data_array + i)->full_name.buffer);
                GUI_Do_Image_Panel(sub_context, AUTO, &player_picture_dim, &((images + i)->image));
                
                sub_context->layout.build_direction = GUI_Build_Direction::down_left;
                
                Color* c = &sub_context->theme->widget_text_color;
                
                for(u32 s = 0; s < u32(Character_Stats::COUNT); ++s)
                {
                    GUI_Do_Text(sub_context, AUTO, LN1(stat_names, s), c);
                    GUI_Push_Layout(sub_context);
                    
                    sub_context->layout.build_direction = GUI_Build_Direction::right_center;
                    
                    sub_context->flags |= GUI_Context_Flags::one_time_skip_padding;
                    GUI_Do_Text(sub_context, AUTO, ": ", c);
                    sub_context->flags |= GUI_Context_Flags::one_time_skip_padding;
                    
                    char* num = U32_To_Char_Buffer((u8*)&num_text_buffer, player->stats[s]);
                    GUI_Do_Text(sub_context, AUTO, num, c);
                    
                    GUI_Pop_Layout(sub_context);
                }
                
                u32 mark_counts[u32(Mark_Type::COUNT)] = {};
                for(each(Mark_GM*, mark, player->marks))
                {
                    // CONSIDER: break out after all counts are > 0?
                    mark_counts[u32(mark->type)] += 1;
                }
                
                if(mark_counts[u32(Mark_Type::item)])
                {
                    GUI_Do_Text(sub_context, AUTO, L1(items), c);
                    
                    for(each(Mark_GM*, mark, player->marks))
                    {
                        if(mark->type == Mark_Type::item)
                        {
                            u32 offset = *(((u32*)s_game_state.mark_table.memory) + mark->idx);
                            char* mark_text = s_game_state.mark_data + offset;
                            
                            GUI_Do_Text(sub_context, AUTO , " ", c);
                            
                            GUI_Push_Layout(sub_context);
                            sub_context->layout.build_direction = GUI_Build_Direction::right_center;
                            
                            sub_context->flags |= GUI_Context_Flags::one_time_skip_padding;
                            GUI_Do_Text(sub_context, AUTO, mark_text, c);
                            
                            if(mark->duration)
                            {
                                sub_context->flags |= GUI_Context_Flags::one_time_skip_padding;
                                GUI_Do_Text(sub_context, AUTO, ": ", c);
                                sub_context->flags |= GUI_Context_Flags::one_time_skip_padding;
                                
                                char* num = U32_To_Char_Buffer((u8*)&num_text_buffer, mark->duration);
                                
                                GUI_Do_Text(sub_context, AUTO, num, c);                            
                            }
                            
                            GUI_Pop_Layout(sub_context);
                        }
                    }
                }
                
                
                if(mark_counts[u32(Mark_Type::personal)])
                {
                    GUI_Do_Text(sub_context, AUTO, L1(character_marks), c);
                    for(each(Mark_GM*, mark, player->marks))
                    {
                        if(mark->type == Mark_Type::personal)
                        {
                            u32 offset = *(((u32*)s_game_state.mark_table.memory) + mark->idx);
                            char* mark_text = s_game_state.mark_data + offset;
                            
                            GUI_Do_Text(sub_context, AUTO , " ", c);
                            
                            GUI_Push_Layout(sub_context);
                            sub_context->layout.build_direction = GUI_Build_Direction::right_center;
                            
                            sub_context->flags |= GUI_Context_Flags::one_time_skip_padding;
                            GUI_Do_Text(sub_context, AUTO, mark_text, c);
                            
                            if(mark->duration)
                            {
                                sub_context->flags |= GUI_Context_Flags::one_time_skip_padding;
                                GUI_Do_Text(sub_context, AUTO, ": ", c);
                                sub_context->flags |= GUI_Context_Flags::one_time_skip_padding;
                                
                                char* num = U32_To_Char_Buffer((u8*)&num_text_buffer, mark->duration);
                                
                                GUI_Do_Text(sub_context, AUTO, num, c);
                            }
                            
                            GUI_Pop_Layout(sub_context);
                        }
                    }                    
                }
                
                sub_context_is_active = GUI_Is_Context_Active(sub_context);
                
                GUI_End_Context(sub_context);
            }
            
            if(sub_context_is_active && s_game_state.active_player_card_idx != i)
            {
                Swap(sub_context_a, sub_context_b);
                GUI_Reset_Context(sub_context_b);
                s_game_state.active_player_card_idx = i;  
            }
            
            p.x += d.x + padding;
        }
        
    }; // ----------------------------------------------------------------------------------------

    Do_GUI_Frame_With_Banner(banner_func, menu_func);
}


static void Do_Night_Falls_Frame()
{
    GUI_Context* context = &s_gui_banner;
    Inverse_Bit_Mask(&context->flags, GUI_Context_Flags::enable_dynamic_sliders);
    
    Clear_Canvas(&s_canvas, s_settings.background_color);
    
    GUI_Begin_Context(
        context,
        &s_canvas,
        &s_global_data.action_context, 
        &s_settings.theme,
        v2i{0, 0},
        GUI_Anchor::center,
        GUI_Build_Direction::down_center);
    
    Font* font = &context->theme->font;
    v2f text_scale = GUI_DEFAULT_TEXT_SCALE * 3.f;
    
    f32 half_char_height = (font->char_height * text_scale.y) * 0.5f;
    f32 x = Get_Middle(context->canvas).x;
    f32 top = f32(context->canvas->dim.y - 1) - half_char_height;
    f32 bottom = 0.f - half_char_height;
    
    f64 time = s_platform.Get_Time_Stamp();
    f64 delta = time - s_game_state.night_falls_start_time;
    f64 t = delta / s_game_state.night_falls_time;
    t = Clamp_To_Barycentric(t);
    t = Square(t);
    f32 y = Lerp(top, bottom, f32(t));
    
    v2f text_pos = v2f{x, y};
    
    GUI_Do_Title_Text(context, &text_pos, L1(night_falls), text_scale);
    
    v2i cursor_pos = s_platform.Get_Cursor_Position();
    bool cursor_on_canvas = Is_Point_On_Canvas(context->canvas, cursor_pos);
    
    if(t >= 1.0 || 
        GUI_Context::actions[GUI_Menu_Actions::enter].Is_Released() ||
        (cursor_on_canvas && GUI_Context::actions[GUI_Menu_Actions::mouse].Is_Released()))
    {
        if(!Assign_Events_To_Participants(&s_game_state, Event_List::night, &s_allocator))
        {
            s_global_data.active_menu = Menus::GM_event_assignement_failed;
        }
        else
        {            
            s_global_data.active_menu = Menus::GM_event_display;
        }
    }
    
    GUI_End_Context(context);
    
    context->flags |= GUI_Context_Flags::enable_dynamic_sliders;
}


static void Do_All_Players_Are_Dead_Frame()
{
    Assert(s_game_state.live_player_count == 0);
    
    static bool skip_frame;
    skip_frame = false;
    
    void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
    { 
        context->layout.anchor = GUI_Anchor::top;
        context->layout.build_direction = GUI_Build_Direction::down_center;
        
        char* title_text = L1(all_partis_are_dead);
        GUI_Do_Title_Text(context, &GUI_AUTO_TOP_CENTER, title_text, GUI_Scale_Default(2.f));
        GUI_Do_Text(context, AUTO, L1(nobody_survived));
        
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(play_again)))
        {
            skip_frame = true;
            Reset_Game(&s_game_state, &s_allocator);
            s_global_data.active_menu = Menus::GM_let_the_games_begin;
        }
        
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(quit_game)))
        {
            skip_frame = true;
            Delete_Game(&s_game_state, &s_allocator);
            s_global_data.active_menu = Menus::main_menu;
        }
    }; // ----------------------------------------------------------------------------------------

    void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        if(skip_frame)
            return;
        
        Player_Image* images = Begin(s_game_state.player_images);
        Player_UD* user_data_array = Begin(s_game_state.player_ud);
        
        context->layout.anchor = GUI_Anchor::top;
        context->layout.build_direction = GUI_Build_Direction::down_center;
        
        GUI_Do_Text(context, &GUI_AUTO_TOP_CENTER, L1(the_dead));
        
        v2f player_picture_dim = Hadamar_Product(s_player_picture_dim_base, GUI_DEFAULT_TEXT_SCALE);
        
        for(u32 i = 0; i < s_game_state.total_player_count; ++i)
        {
            GUI_Do_Text(context, AUTO, (user_data_array + i)->full_name.buffer);
            GUI_Do_Image_Panel(context, AUTO, &player_picture_dim, &((images + i)->image));
        }
        
    }; // ----------------------------------------------------------------------------------------

    Do_GUI_Frame_With_Banner(banner_func, menu_func);    
}


static void Do_We_Have_A_Winner_Frame()
{
    Assert(s_game_state.live_player_count == 1);
    
    static bool skip_frame;
    skip_frame = false;
    
    void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
    {     
        context->layout.anchor = GUI_Anchor::top;
        context->layout.build_direction = GUI_Build_Direction::down_center;
        
        GUI_Do_Title_Text(context, &GUI_AUTO_TOP_CENTER, L1(we_have_winner), GUI_Scale_Default(2.f));
        GUI_Do_Text(context, AUTO, L1(may_his_name_be_written_in_legends));
        
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(play_again)))
        {
            skip_frame = true;
            Reset_Game(&s_game_state, &s_allocator);
            s_global_data.active_menu = Menus::GM_let_the_games_begin;
        }
        
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(quit_game)))
        {
            skip_frame = true;
            Delete_Game(&s_game_state, &s_allocator);
            s_global_data.active_menu = Menus::main_menu;
        }
    }; // ----------------------------------------------------------------------------------------

    void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        if(skip_frame)
            return;
        
        context->layout.anchor = GUI_Anchor::top;
        context->layout.build_direction = GUI_Build_Direction::down_center;
        
        GUI_Do_Text(context, &GUI_AUTO_TOP_CENTER, L1(winner));
        
        Player_Image* images = Begin(s_game_state.player_images);
        Player_UD* user_data_array = Begin(s_game_state.player_ud);
        
        v2f player_picture_dim = Hadamar_Product(s_player_picture_dim_base, GUI_DEFAULT_TEXT_SCALE);
        
        GUI_Do_Text(context, AUTO, user_data_array->full_name.buffer);
        GUI_Do_Image_Panel(context, AUTO, &player_picture_dim, &images->image);
        
        v2f seperator_dim = v2f{f32(context->canvas->dim.x) - 50, 10};
        seperator_dim.x = Max(seperator_dim.x, player_picture_dim.x);
        
        GUI_Do_Panel(context, AUTO, &seperator_dim);
        
        GUI_Do_Text(context, AUTO, L1(the_dead));
        
        for(u32 i = 1; i < s_game_state.total_player_count; ++i)
        {
            GUI_Do_Text(context, AUTO, (user_data_array + i)->full_name.buffer);
            GUI_Do_Image_Panel(context, AUTO, &player_picture_dim, &((images + i)->image));
        }
        
    }; // ----------------------------------------------------------------------------------------

    Do_GUI_Frame_With_Banner(banner_func, menu_func);
}


static void Do_Event_Assignement_Failed_Frame()
{
    static bool skip_frame;
    skip_frame = false;
    
    void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
    {        
        context->layout.anchor = GUI_Anchor::top;
        context->layout.build_direction = GUI_Build_Direction::down_center;
        
        char* title_text = L1(event_error);
        GUI_Do_Title_Text(context, &GUI_AUTO_TOP_CENTER, title_text, GUI_Scale_Default(2.f));
        GUI_Do_Text(context, AUTO, L1(could_not_fit_every_parti));
        
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(quit_game)))
        {
            skip_frame = true;
            Delete_Game(&s_game_state, &s_allocator);
            s_global_data.active_menu = Menus::main_menu;
        }
        
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(play_again)))
        {
            skip_frame = true;
            Reset_Game(&s_game_state, &s_allocator);
            s_global_data.active_menu = Menus::GM_let_the_games_begin;
        }
        
    }; // ----------------------------------------------------------------------------------------

    void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        if(skip_frame)
            return;
        
        // Why are we doing this?
        // No idea- was just funny and didnt know what to put in this frame xD
        
        f64 time = s_platform.Get_Time_Stamp();
        
        f32 c = f32(s_game_state.total_player_count);
        f32 padding = f32(context->theme->padding);
        
        v2f screen_middle = Get_Middle(context->canvas);
        
        v2f player_picture_dim = Hadamar_Product(s_player_picture_dim_base, GUI_DEFAULT_TEXT_SCALE);
        
        f32 pos_x = (c * (padding + player_picture_dim.x) - padding) * -0.5f;
       
        context->layout.anchor = GUI_Anchor::left;
        
        for(each(Player_Image*, img, s_game_state.player_images))
        {
            f32 y_offset;
            if(context->canvas->dim.y > player_picture_dim.y + padding * 2)
            {
                f32 y_span = 0.5f * (f32(context->canvas->dim.y) - player_picture_dim.y) - padding;
                y_offset = f32(Sin(time)) * y_span;                
            }
            else
            {
                y_offset = 0;
            }
            
            v2f pos = {screen_middle.x + pos_x, screen_middle.y + y_offset};
            
            GUI_Do_Image_Panel(context, &pos, &player_picture_dim, &img->image);
            pos_x += padding + player_picture_dim.x;
            time += 0.5;
        }
        
    }; // ----------------------------------------------------------------------------------------
    
    u32 flags = s_gui.flags;
    Inverse_Bit_Mask(&s_gui.flags, GUI_Context_Flags::enable_dynamic_sliders);
    
    Do_GUI_Frame_With_Banner(banner_func, menu_func);
    
    s_gui.flags = flags;
}


static void Do_Load_Failed_Popup(GUI_Context* context)
{
    context->layout.anchor = GUI_Anchor::bottom;
    context->layout.build_direction = GUI_Build_Direction::down_center;
    
    GUI_Do_Title_Text(context, &GUI_AUTO_MIDDLE, L1(something_failed));
    GUI_Do_Text(context, AUTO, L1(campaign_could_not_be_loaded));
    
    GUI_Do_Spacing(context, v2f{0, 20});
    
    if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(confirm)))
    {
        Close_Popup();
    }
}


static void Free_Invalid_Event_Filter_Result_Memory()
{
    Assert(s_global_data.IEFR);
    
    Hollow_Invalid_Event_Filter_Results(s_global_data.IEFR);
    s_allocator.free(s_global_data.IEFR);
    
    s_global_data.IEFR = 0;
    
    //NOTE: Be aware that if this is removed memory cleanup has to be handeled elsewhere.
    Delete_Event_Container(&s_editor_state.event_container, &s_allocator);
}


static void Do_Display_Invalid_Event_Filter_Results_Popup(GUI_Context* context)
{
    Assert(s_global_data.IEFR);

    context->layout.anchor = GUI_Anchor::left;
    context->layout.build_direction = GUI_Build_Direction::down_left;
    
    f32 sh = f32(context->canvas->dim.y);
    f32 pf = .5f;
    f32 offset = sh * (pf / 2);
    
    f32 m = Get_Middle(context->canvas).x;
    
    char* title_text = L1(campaign_contains_invalid_events);
    char* sub_title_text = L1(for_game_functionality_they_have_been_removed);
    
    f32 char_width_title = 
        GUI_Character_Width(context, GUI_DEFAULT_TEXT_SCALE.x * GUI_DEFAULT_TITLE_SCALER.x);
    
    f32 char_width = GUI_Character_Width(context);
    
    f32 len_title = (f32)Null_Terminated_Buffer_Lenght(title_text) * char_width_title;
    f32 len_sub_title = (f32)Null_Terminated_Buffer_Lenght(sub_title_text) * char_width;
    
    f32 longest_text = len_title > len_sub_title? len_title : len_sub_title;
    v2f title_pos = {m - longest_text / 2, sh - offset};
    
    GUI_Do_Title_Text(context, &title_pos, title_text);
    
    f32 title_width = context->layout.last_element.dim.x;
    
    GUI_Do_Text(context, AUTO, sub_title_text);
    
    f32 sub_title_width = context->layout.last_element.dim.x;
    
    if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(continue_anyway)))
    {
        Game_State game_state;
        
        u32 errors = Convert_Editor_Campaign_Into_Game_Format(
            &game_state,
            &s_editor_state.event_container,
            &s_platform,
            &s_allocator);
        
        if(!errors)
        {
            Close_Popup();
            
            if(s_game_state.memory)
                Delete_Game(&s_game_state, &s_allocator);
            
            s_game_state = game_state;
            
            s_global_data.active_menu = Menus::GM_players;
        }
        else
        {
            s_global_data.GM_conversion_errors = errors;
            Set_Popup_Function(Do_GM_Campaign_Was_Unusable_Popup);
        }
    }
    
    GUI_Push_Layout(context);
    
    context->layout.build_direction = GUI_Build_Direction::right_center;
    
    if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(cancel)))
    {
        Close_Popup();
    }
    
    if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(open_in_editor)))
    {
        Events_Container ec;
        
        if(Load_Campaign(
            &ec, 
            &s_editor_state.event_container.campaign_name, 
            &s_allocator, 
            &s_platform))
        {
            Close_Popup();
            
            s_editor_state.event_container = ec;
            s_global_data.active_menu = Menus::EE_all_events;
            GUI_Pop_Layout(context);
            return;
        }
    }
    
    GUI_Pop_Layout(context);
    
    GUI_Do_Spacing(context, v2f{0, 30});
    
    GUI_Do_Text(context, AUTO, L1(the_following_events_contain_errors));
    
    if(s_global_data.IEFR)
    {
        f32 padding = f32(context->theme->padding);
        
        f32 bot = context->layout.last_element.pos.y - context->anchor_base.y;
        bot -= context->layout.last_element.dim.y / 2;
        bot -= padding;
        f32 container_height = bot - offset;

        if(container_height >= 1.f)
        {
            f32 w = title_width > sub_title_width? title_width : sub_title_width;
            v2f sub_canvas_dim = v2f{w, container_height};
            v2f sub_canvas_pos = context->layout.last_element.pos - context->anchor_base; 
            sub_canvas_pos.x -= context->layout.last_element.dim.x / 2;
            sub_canvas_pos.y -= 
                context->layout.last_element.dim.y / 2 + padding + sub_canvas_dim.y / 2;
            
            Canvas canvas;
            GUI_Context* sub_context = Get_GUI_Context_From_Pool();
            
            context->layout.anchor = GUI_Anchor::left;
            
            if(GUI_Do_Sub_Context(
                context, 
                sub_context, 
                &canvas, 
                &sub_canvas_pos, 
                &sub_canvas_dim,
                &s_settings.list_bg_color))
            {
                sub_context->layout.anchor = GUI_Anchor::top_left;
                
                v2f* p = &GUI_AUTO_TOP_LEFT;
                
                for(each(Invalid_Event_Filter_Result*, IEFR, s_global_data.IEFR))
                {
                    GUI_Do_Title_Text(sub_context, p, "-", GUI_DEFAULT_TEXT_SCALE);
                    GUI_Push_Layout(sub_context);
                    
                    sub_context->flags |= GUI_Context_Flags::one_time_skip_padding;
                    
                    sub_context->layout.build_direction = GUI_Build_Direction::right_center;
                    GUI_Do_Title_Text(sub_context, AUTO, IEFR->name.buffer, GUI_DEFAULT_TEXT_SCALE);
                    
                    sub_context->flags |= GUI_Context_Flags::one_time_skip_padding;
                    GUI_Do_Title_Text(sub_context, AUTO, ":", GUI_DEFAULT_TEXT_SCALE);
                    
                    GUI_Pop_Layout(sub_context);
                    p = 0;
                    
                    f32 x = sub_context->layout.last_element.pos.x - sub_context->anchor_base.x;
                    
                    CSTR_List error_names = LN(event_error_names);
                    
                    for(u32 i = 0; i < error_names.count; ++i)
                    {
                        if(IEFR->reasons & (1 << i))
                        {
                            GUI_Do_Spacing(sub_context, v2f{50, sub_context->layout.last_element.dim.y});
                            
                            GUI_Push_Layout(sub_context);
                            
                            sub_context->layout.build_direction = GUI_Build_Direction::right_center;
                            
                            GUI_Do_Text(sub_context, AUTO, error_names.list[i]);
                            
                            GUI_Pop_Layout(sub_context);
                        }
                    }
                }
                
                GUI_End_Context(sub_context);
            }
        }
    }
}


static void Do_GM_Campaign_Was_Unusable_Popup(GUI_Context* context)
{
    context->layout.build_direction = GUI_Build_Direction::down_left;
    context->layout.anchor = GUI_Anchor::center;
    
    v2f panel_dim = v2f{900, 500};

    v2f panel_center = Get_Middle(context->canvas);
    
    GUI_Get_Placement(context, &panel_dim, &GUI_AUTO_MIDDLE);
    
    context->layout.anchor = GUI_Anchor::top_left;
    
    v2f title_pos = panel_center + v2f{panel_dim.x * -1, panel_dim.y} / 2;
    GUI_Do_Title_Text(context, &title_pos, L1(error_report));
    GUI_Do_Text(context, AUTO, L1(game_can_not_be_played));
    
    Canvas sub_canvas;
    GUI_Context* sub_context = Get_GUI_Context_From_Pool();
    
    v2f sub_space_dim = v2f
        {
            panel_dim.x, 
            (context->layout.last_element.pos.y - context->layout.last_element.dim.y / 2) - 
                (panel_center.y - panel_dim.y / 2)
        };
    
    if(GUI_Do_Sub_Context(
        context,
        sub_context, 
        &sub_canvas, 
        AUTO, 
        &sub_space_dim, 
        &s_settings.list_bg_color))
    {
        GUI_Do_Title_Text(sub_context, &GUI_AUTO_TOP_LEFT, L1(errors), GUI_DEFAULT_TEXT_SCALE);

        CSTR_List game_error_names = LN(game_error_names);
        
        for(u32 i = 0; i < game_error_names.count; ++i)
        {
            if(s_global_data.GM_conversion_errors & (1 << i))
                GUI_Do_Text(sub_context, AUTO, game_error_names.list[i]);
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


// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#pragma once


static void Do_Settings_Menu_Frame()
{
    void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        f32 s = 2.f;
        
        if(GUI_Do_Button(context, &GUI_AUTO_TOP_LEFT, &GUI_AUTO_FIT, "<",  GUI_Scale_Default(s)))
        {
            s_global_data.active_menu = Menus::main_menu;
        }
        
        GUI_Push_Layout(context);
        
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        GUI_Do_Title_Text(context, AUTO, L1(settings_title), GUI_Scale_Default(s));
        
        GUI_Pop_Layout(context);
        
        context->layout.build_direction = GUI_Build_Direction::down_left;
        
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(reset_defaults)))
        {
            Set_Settings_To_Default();
        }
        
    }; // ----------------------------------------------------------------------------------------

    void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
    {
        GUI_Do_Title_Text(context, &GUI_AUTO_TOP_LEFT, L1(language));
        
        if(u32 s = GUI_Do_Dropdown_Button(
            context, 
            AUTO, 
            &GUI_AUTO_FIT, 
            u32(s_settings.language),
            LN(languages)))
        {
            s_settings.language = Language(s - 1);
        }
        
        GUI_Do_Spacing(context, AUTO);
        
        GUI_Do_Title_Text(context, AUTO, L1(app_appearance));
        
        GUI_Highlight highlight = GUI_Highlight_Next(context, 2);
        if(s_settings.allow_non_uniform_text_scale)
            highlight.count = 3;
        
        GUI_Do_Text(context, AUTO, L1(text_size), highlight);
        
        v2f check_box_dim = v2f{1.f, 1.f} * context->layout.last_element.dim.y;
        
        GUI_Do_Checkbox(context, AUTO, &check_box_dim, &s_settings.allow_non_uniform_text_scale);
        v2f slider_dim = v2f{300, context->layout.last_element.dim.y};
        
        GUI_Push_Layout(context);
        
        context->layout.build_direction = GUI_Build_Direction::right_center;
        
        GUI_Do_Text(context, AUTO, L1(allow_non_uniform_text_scale), GUI_Highlight_Prev(context));
        
        GUI_Pop_Layout(context);
        
        s_settings.text_scale = GUI_DEFAULT_TEXT_SCALE;
        
        if(s_settings.allow_non_uniform_text_scale)
        {
            context->flags |= GUI_Context_Flags::one_time_ignore_id;
            if(GUI_Do_Fill_Slider(
                context, 
                AUTO, 
                &slider_dim, 
                &s_settings.
                text_scale.x, 
                5.f, 
                1.f, 
                0.1f))
            { 
                GUI_DEFAULT_TEXT_SCALE.x = s_settings.text_scale.x;
            }
            
            context->flags |= GUI_Context_Flags::one_time_ignore_id;
            if(GUI_Do_Fill_Slider(
                context, 
                AUTO, 
                &slider_dim, 
                &s_settings.
                text_scale.y, 
                5.f, 
                1.f, 
                0.1f))
            {
                GUI_DEFAULT_TEXT_SCALE.y = s_settings.text_scale.y;
            }
        }
        else
        {
            GUI_DEFAULT_TEXT_SCALE.y = GUI_DEFAULT_TEXT_SCALE.x;
            s_settings.text_scale.y = s_settings.text_scale.x;
            
            context->flags |= GUI_Context_Flags::one_time_ignore_id;
            if(GUI_Do_Fill_Slider(
                context, 
                AUTO, 
                &slider_dim, 
                &s_settings.text_scale.x, 
                5.f, 
                1.f, 
                0.1f))
            {
                s_settings.text_scale.y = s_settings.text_scale.x;
                GUI_DEFAULT_TEXT_SCALE = s_settings.text_scale;
            }            
        }
        
        GUI_Do_Text(context, AUTO, L1(theme), GUI_Highlight_Next(context, 2));

        if(u32 s = GUI_Do_Dropdown_Button(context, AUTO, &GUI_AUTO_FIT, L1(prebuild), LN(theme_names)))
        {
            GUI_Default_Theme_Names name = GUI_Default_Theme_Names(s - 1);
            s_settings.theme = GUI_Create_Default_Theme(name, s_font);
            Set_Additional_Colors_Based_On_Default_Theme(name);
        }
        
        if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(create_custom)))
        {
            
        }
        
    }; // ----------------------------------------------------------------------------------------

    Do_GUI_Frame_With_Banner(banner_func, menu_func);
}


static void Do_Default_Quit_Popup(GUI_Context* context)
{
    f32 text_height = GUI_Character_Height(context);
    f32 title_height = text_height * GUI_DEFAULT_TITLE_SCALER.y;
    
    f32 frame_height = title_height + (text_height + GUI_Padding(context) * 2) * 2;
    
    v2f title_pos = Get_Middle(context->canvas);
    title_pos.y += frame_height * 0.5f;
    
    GUI_Do_Title_Text(context, &title_pos, L1(are_you_sure_close));
    
    context->layout.build_direction = GUI_Build_Direction::down_center;
    
    char* t1 = L1(cancel_and_continue);
    char* t2 = L1(close_app);
    
    char* t = Get_Longest_CSTR(t1, t2);
    v2f button_dim = GUI_Tight_Fit_Text(t, &context->theme->font) + context->theme->padding;
    
    if(GUI_Do_Button(context, AUTO, &button_dim, t1))
        Close_Popup();
    
    
    if(GUI_Do_Button(context, AUTO, &button_dim, t2))
        s_platform.Set_Flag(App_Flags::is_running, false);
}


static void Do_Main_Menu_Frame()
{
    Assert(s_mem.push_call_count == s_mem.free_call_count);
    
    Clear_Canvas(&s_canvas, s_settings.background_color);
    
    GUI_Context* context = &s_gui_banner;
    GUI_Begin_Context(
        context,
        &s_canvas, 
        &s_global_data.action_context, 
        &s_settings.theme,
        v2i{0, 0},
        GUI_Anchor::top);
    
    GUI_Do_Title_Text(context, &GUI_AUTO_TOP_CENTER, L1(menu_title), GUI_Scale_Default(4.f));
    
    f32 last_half_height = context->layout.last_element.dim.y / 2;
    GUI_Do_Spacing(context, v2f{0, last_half_height});
    
    context->layout.build_direction = GUI_Build_Direction::down_center;
    
    CSTR_List button_texts = LN(main_menu_buttons);
    
    constexpr f32 s = 2.f;

    v2f dim = GUI_Tight_Fit_Multi_Line_Text(
        &context->theme->font, 
        button_texts.list, 
        button_texts.count, GUI_Scale_Default(s));
    
    dim += context->theme->padding;
    
    u32 i = 0;
    // Continue
    if(GUI_Do_Button(context, AUTO, &dim, button_texts.list[i++], GUI_Scale_Default(s)))
    {
        
    }
    
    // New game
    if(GUI_Do_Button(context, AUTO, AUTO, button_texts.list[i++], GUI_Scale_Default(s)))
    {
        Gather_Editor_Format_Campaigns();
        s_global_data.active_menu = Menus::select_campaign_to_play_menu;
    }
    
    // Load game
    if(GUI_Do_Button(context, AUTO, AUTO, button_texts.list[i++], GUI_Scale_Default(s)))
    {
        
    }
    
    // Open editor
    if(GUI_Do_Button(context, AUTO, AUTO, button_texts.list[i++], GUI_Scale_Default(s)))
    {
        Gather_Editor_Format_Campaigns();
        s_global_data.active_menu = Menus::campaigns_menu;
    }
    
    // Settings
    if(GUI_Do_Button(context, AUTO, AUTO, button_texts.list[i++], GUI_Scale_Default(s)))
    {
        s_global_data.active_menu = Menus::settings_menu;
    }
    
    // Close 
    if(GUI_Do_Button(context, AUTO, AUTO, button_texts.list[i++], GUI_Scale_Default(s)))
    {
        s_global_data.force_quit_popup = true;
    }
    
    GUI_End_Context(context);
}
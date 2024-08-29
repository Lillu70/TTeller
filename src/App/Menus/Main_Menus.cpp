

// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#pragma once


static void Do_Settings_Menu_Frame();


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


static void Do_Color_Editor_Popup(GUI_Context* context)
{
    char* t = Get_Localised_Text(Loc_Identifier(s_global_data.color_edit_title_idx));
    GUI_Do_Title_Text(context, &GUI_AUTO_MIDDLE, t);
    
    v2f slider_dim = v2f{100.f * GUI_DEFAULT_TEXT_SCALE.x, GUI_Character_Height(context)};
    char num_buffer[12] = {};
    
    Color selected_color = context->theme->selected_color;
    Color inv_selected_color = 
        Make_Color(255 - selected_color.r, 255 - selected_color.g, 255 - selected_color.b);
    
    Color outline_color = context->theme->outline_color;
    Color inv_outline_color = 
        Make_Color(255 - outline_color.r, 255 - outline_color.g, 255 - outline_color.b);
    
    f32* v;
    v2f num_pos;
    char* num_text;
    
    v = &s_global_data.color_edit_color_up.r;
    GUI_Do_Fill_Slider(context, AUTO, &slider_dim, v, 255.f, 0.f, 1.f);
    f32 top = context->layout.last_element.pos.y + context->layout.last_element.dim.y / 2;
    GUI_Push_Layout(context);
    
    GUI_Push_Layout(context);
    num_text = U32_To_Char_Buffer((u8*)num_buffer, u32(*v));
    num_pos = context->layout.last_element.pos - context->anchor_base;
    context->layout.anchor = GUI_Anchor::center;
    GUI_Do_Text(
        context, 
        &num_pos, 
        num_text, 
        GUI_Was_Last_Selected(context)? &inv_selected_color : &inv_outline_color);
    GUI_Pop_Layout(context);
    
    v = &s_global_data.color_edit_color_up.g;
    GUI_Do_Fill_Slider(context, AUTO, &slider_dim, v, 255.f, 0.f, 1.f);
    GUI_Push_Layout(context);
    num_text = U32_To_Char_Buffer((u8*)num_buffer, u32(*v));
    num_pos = context->layout.last_element.pos - context->anchor_base;
    context->layout.anchor = GUI_Anchor::center;
    GUI_Do_Text(
        context, 
        &num_pos, 
        num_text, 
        GUI_Was_Last_Selected(context)? &inv_selected_color : &inv_outline_color);
    GUI_Pop_Layout(context);
    
    v = &s_global_data.color_edit_color_up.b;
    GUI_Do_Fill_Slider(context, AUTO, &slider_dim, v, 255.f, 0.f, 1.f);
    f32 bot = context->layout.last_element.pos.y - context->layout.last_element.dim.y / 2;
    bool last_slider_selected = GUI_Was_Last_Selected(context);
    GUI_Push_Layout(context);
    
    Color preview_color = Pack_Color(s_global_data.color_edit_color_up);
    if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(apply)))
    {
        *s_global_data.color_edit_color = preview_color;
        Close_Popup();
    }
    context->layout.build_direction = GUI_Build_Direction::right_center;
    
    if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(cancel)))
    {
        Close_Popup();
    }
    
    GUI_Pop_Layout(context);
    
    GUI_Push_Layout(context);
    num_text = U32_To_Char_Buffer((u8*)num_buffer, u32(*v));
    num_pos = context->layout.last_element.pos - context->anchor_base;
    context->layout.anchor = GUI_Anchor::center;
    GUI_Do_Text(
        context, 
        &num_pos, 
        num_text, 
        last_slider_selected? &inv_selected_color : &inv_outline_color);
    
    GUI_Pop_Layout(context);
    GUI_Pop_Layout(context);
    
    f32 h = top - bot;
    v2f panel_dim = v2f{h, h};
    
    context->layout.build_direction = GUI_Build_Direction::right_top;
    
    GUI_Do_Panel(context, AUTO, &panel_dim, &preview_color);
}


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
                &s_settings.text_scale.x, 
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
                &s_settings.text_scale.y, 
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
        
        GUI_Do_Text(context, AUTO, L1(theme), GUI_Highlight_Next(context));

        if(u32 s = GUI_Do_Dropdown_Button(context, AUTO, &GUI_AUTO_FIT, L1(prebuild), LN(theme_names)))
        {
            GUI_Default_Theme_Names name = GUI_Default_Theme_Names(s - 1);
            s_settings.theme = GUI_Create_Default_Theme(name, s_font);
            Set_Additional_Colors_Based_On_Default_Theme(name);
        }
        
        
        void (*Do_Color_Display_And_Edit)(GUI_Context*, Color*, Loc_Identifier) = 
            [](GUI_Context* context,Color* color, Loc_Identifier name)
        {
            v2f color_panel_dim = 
                v2f{} + GUI_Character_Height(context) + f32(context->theme->padding);
            
            GUI_Do_Panel(context, AUTO, &color_panel_dim, color, GUI_Highlight_Next(context));
            GUI_Push_Layout(context);
            context->layout.build_direction = GUI_Build_Direction::right_center;
            
            if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, L1(edit)))
            {
                s_global_data.color_edit_title_idx = u32(name);
                s_global_data.color_edit_color = color;
                s_global_data.color_edit_color_up = Unpack_Color(*color);
                Set_Popup_Function(Do_Color_Editor_Popup);
            }
            
            GUI_Do_Text(context, AUTO, Get_Localised_Text(name));
            
            GUI_Pop_Layout(context);
        };
        
        Do_Color_Display_And_Edit(
            context, 
            &s_settings.theme.selected_color, 
            Loc_Identifier::selected_color);
        
        Do_Color_Display_And_Edit(
            context, 
            &s_settings.theme.background_color, 
            Loc_Identifier::widget_background_color);
    
        Do_Color_Display_And_Edit(
            context, 
            &s_settings.theme.down_color, 
            Loc_Identifier::widget_down_color);
    
        Do_Color_Display_And_Edit(
            context, 
            &s_settings.theme.outline_color, 
            Loc_Identifier::widget_outline_color);
            
        Do_Color_Display_And_Edit(
            context, 
            &s_settings.theme.text_color, 
            Loc_Identifier::text_color);
            
        Do_Color_Display_And_Edit(
            context, 
            &s_settings.theme.widget_text_color, 
            Loc_Identifier::widget_text_color);

        Do_Color_Display_And_Edit(
            context, 
            &s_settings.theme.title_color, 
            Loc_Identifier::title_color);

        Do_Color_Display_And_Edit(
            context, 
            &s_settings.theme.write_cursor_color, 
            Loc_Identifier::write_cursor_color);

        Do_Color_Display_And_Edit(
            context, 
            &s_settings.theme.write_cursor_limit_color, 
            Loc_Identifier::write_cursor_limit_color);

        Do_Color_Display_And_Edit(
            context, 
            &s_settings.background_color, 
            Loc_Identifier::page_background_color);
            
        Do_Color_Display_And_Edit(
            context, 
            &s_settings.banner_background_color, 
            Loc_Identifier::banner_background_color);

        Do_Color_Display_And_Edit(
            context, 
            &s_settings.list_bg_color, 
            Loc_Identifier::list_bg_color);

        Do_Color_Display_And_Edit(
            context, 
            &s_settings.warning_theme.selected_color, 
            Loc_Identifier::warning_theme_selected_color);
            
        Do_Color_Display_And_Edit(
            context, 
            &s_settings.warning_theme.background_color, 
            Loc_Identifier::warning_theme_background_color);
            
        Do_Color_Display_And_Edit(
            context, 
            &s_settings.warning_theme.outline_color, 
            Loc_Identifier::warning_theme_outline_color);
            
        Do_Color_Display_And_Edit(
            context, 
            &s_settings.error_theme.selected_color, 
            Loc_Identifier::error_theme_selected_color);
            
        Do_Color_Display_And_Edit(
            context, 
            &s_settings.error_theme.background_color, 
            Loc_Identifier::error_theme_background_color);
            
        Do_Color_Display_And_Edit(
            context, 
            &s_settings.warning_theme.outline_color, 
            Loc_Identifier::error_theme_outline_color);
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
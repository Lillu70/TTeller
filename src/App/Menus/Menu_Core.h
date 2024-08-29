

// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#pragma once

#define AUTO 0

static GUI_Context s_gui;
static GUI_Context s_gui_banner;
static GUI_Context s_gui_pop_up;


static Font s_font = {
    s_terminus_font_char_width, 
    s_terminus_font_char_height, 
    (u8*)&s_terminus_font[0], 
    (u8*)&s_terminus_font_special_characters[0]
};


struct Pooled_GUI_Context
{
    GUI_Context context;
    u64 last_used_line;
    u64 last_used_func;
};
static Pooled_GUI_Context s_context_pool[3] = {};

static constexpr f32 s_post_title_y_spacing = 20;


// TODO: Figure out where to put these enums
namespace Global_Hotkeys
{
    enum T : u32
    {
        toggle_fullscreen = 0,
        open_quit_popup,
        toggle_language,
        display_memory,
        COUNT
    };
}

namespace Editor_Hotkeys
{
    enum T : u32
    {
        active_pannel_toggle = Global_Hotkeys::COUNT,
        save,
        jump_to_all_events,
        jump_to_participants,
        jump_to_event_text,
        jump_to_consequences,
        jump_right,
        jump_left,
        COUNT
    };
}
static constexpr u32 s_hotkey_count = Editor_Hotkeys::COUNT;
static Action s_hotkeys[s_hotkey_count] = {};
// ----------------------------------------- 

struct Settings
{
    bool allow_non_uniform_text_scale;
    v2f text_scale; // NOTE/CONSIDER: <- Redundant value. Actual text scale is handled by GUI_DEFAULT_TEXT_SCALE inside the GUI.h.
    
    GUI_Theme theme;
    GUI_Theme error_theme;
    GUI_Theme warning_theme;
    
    Color background_color;
    Color banner_background_color;
    Color list_bg_color;
    
    Language language;
};
static Settings s_settings = {};


struct Global_Data
{
    Action_Context action_context = Action_Context();
    
    Menus active_menu = Menus::main_menu;
    
    String new_campaign_name;
    
    Dynamic_Array<String>* on_disk_campaign_names = 0;
    Dynamic_Array<Invalid_Event_Filter_Result>* IEFR = 0;
    
    void(*popup_func)(GUI_Context*) = 0;
    void(*on_popup_close)() = 0;
    
    // ------------------
    u32 color_edit_title_idx = 0;
    Color* color_edit_color = 0;
    v3f color_edit_color_up = {};
    // ------------------
    
    v2f popup_panel_dim = v2f{0.f, 0.f};
    v2f title_scale = v2f{2.f, 2.f};

    Rect popup_panel_rect = {};
    
    bool force_quit_popup = false;
    
    u32 pooled_contexes_in_use_count = 0;
    u32 GM_conversion_errors = 0;
    u32 campaing_idx_to_delete = 0;
    
    Image edit_image = {};
    Image delete_image = {};
};
static Global_Data s_global_data = Global_Data();


static Game_State s_game_state = {};
static Editor_State s_editor_state = {};


static inline void Close_Popup()
{
    if(s_global_data.on_popup_close)
    {
        s_global_data.on_popup_close();
        s_global_data.on_popup_close = 0;
    }
    
    s_global_data.popup_func = 0;
    GUI_Activate_Context(&s_gui_banner);
}


static inline void Set_Popup_Function(void(*popup_function)(GUI_Context*))
{
    if(s_global_data.popup_func)
        Close_Popup();
    
    s_global_data.popup_panel_rect = Create_Rect_Min_Max_HZ(v2f{0,0}, v2f{0,0});
    s_global_data.popup_panel_dim = v2f{0.f, 0.f};
    
    GUI_Try_Reset_Context(&s_gui_pop_up);
    
    GUI_Activate_Context(&s_gui_pop_up);        
    
    s_global_data.popup_func = popup_function;
}


static inline void Set_Popup_Function(void(*popup_function)(GUI_Context*), void(*on_popup_close)())
{
    Set_Popup_Function(popup_function);
    s_global_data.on_popup_close = on_popup_close;
}


static inline v2f Get_Title_Bar_Row_Placement(
    GUI_Context* context, 
    f32 title_max_x, 
    f32 padding, 
    f32 ctrl_buttons_width)
{
    f32 back_button_x;
    if(title_max_x + padding > f32(context->canvas->dim.x) - ctrl_buttons_width)
        back_button_x = title_max_x + padding;
    else
        back_button_x = f32(context->canvas->dim.x) - ctrl_buttons_width;
    
    f32 back_button_y = f32(context->canvas->dim.y - 1) - padding;
    
    v2f result = v2f{back_button_x, back_button_y};
    return result;
}



static inline void Set_Additional_Colors_Based_On_Default_Theme(GUI_Default_Theme_Names theme)
{
    switch(theme)
    {
        case GUI_Default_Theme_Names::cyper:
        case GUI_Default_Theme_Names::vintage:
        {
            s_settings.background_color         = Make_Color(20, 20, 20);
            s_settings.banner_background_color  = Make_Color(40, 40, 40);
            s_settings.list_bg_color            = Make_Color(10, 10, 10);
        }break;
        
        case GUI_Default_Theme_Names::document:
        {
            s_settings.background_color         = Make_Color(240, 240, 240);
            s_settings.banner_background_color  = Make_Color(194, 248, 247);
            s_settings.list_bg_color            = WHITE;
        }break;
        
        case GUI_Default_Theme_Names::oasis:
        {
            s_settings.background_color         = Make_Color(255, 217, 159);
            s_settings.banner_background_color  = Make_Color(159, 232, 255);
            s_settings.list_bg_color            = Make_Color(10, 10, 10);
        }break;
        
        
        default:
        {
            Terminate;
        }
    }
}


static void Set_Settings_To_Default()
{
    s_settings = {};
    
    // -------------------------------------------------
    s_settings.text_scale = v2f{2.f, 2.f};
    GUI_DEFAULT_TEXT_SCALE = s_settings.text_scale;
    // -------------------------------------------------
    
    GUI_Default_Theme_Names def_theme = GUI_Default_Theme_Names::vintage;
    s_settings.theme = GUI_Create_Default_Theme(def_theme, s_font);
    Set_Additional_Colors_Based_On_Default_Theme(def_theme);
    
    // -------------------------------------------------
    s_settings.warning_theme = s_settings.theme;
    s_settings.warning_theme.selected_color         = Make_Color(128, 0, 0);
    s_settings.warning_theme.background_color       = Make_Color(255, 255, 128);
    s_settings.warning_theme.outline_color          = Make_Color(234, 143, 21);
    // -------------------------------------------------    
    s_settings.error_theme = s_settings.theme;
    s_settings.error_theme.selected_color           = Make_Color(128, 0, 0);
    s_settings.error_theme.background_color         = Make_Color(180, 70, 70);
    s_settings.error_theme.outline_color            = Make_Color(255, 235, 235);
    // -------------------------------------------------
}


// Usage
/*

void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
{
    
}; // ----------------------------------------------------------------------------------------

void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
{
    
}; // ----------------------------------------------------------------------------------------

Do_GUI_Frame_With_Banner(banner_func, menu_func);

*/

static void Do_GUI_Frame_With_Banner(
    void(*banner_func)(GUI_Context*), 
    void(*menu_func)(GUI_Context*), 
    bool enable_default_active_menu_hotkey_behavior = true)
{
    Assert(banner_func);
    Assert(menu_func);
    
    if(enable_default_active_menu_hotkey_behavior)
    {
        if(s_hotkeys[Editor_Hotkeys::active_pannel_toggle].Is_Released())
        {
            if(GUI_Is_Context_Active(&s_gui) && s_gui_banner.widget_count)
                GUI_Activate_Context(&s_gui_banner);
            
            else if(GUI_Is_Context_Active(&s_gui_banner)  && s_gui.widget_count)
                GUI_Activate_Context(&s_gui);
        }
        
        if(GUI_Is_Context_Active(&s_gui) && s_gui.widget_count == 0 && s_gui_banner.widget_count)
        {
            GUI_Activate_Context(&s_gui_banner);
        }
        else if(GUI_Is_Context_Active(&s_gui_banner) && !s_gui_banner.widget_count && s_gui.widget_count)
        {
            GUI_Activate_Context(&s_gui);
        }
    }
    
    GUI_Begin_Context_In_Layout_Only_Mode(&s_gui_banner, &s_canvas, &s_settings.theme);
    {
        banner_func(&s_gui_banner);
    }
    GUI_End_Context(&s_gui_banner);
    
    Rect banner_rect = GUI_Get_Bounds_In_Pixel_Space(&s_gui_banner);
    
    f32 banner_height = Get_Rect_Dimensions(banner_rect).y;
    banner_height = Ceil(banner_height);
    banner_height += f32(s_settings.theme.padding * 2) + s_gui.dynamic_slider_girth;
    banner_height = Clamp_Zero_To_Max(banner_height, f32(s_canvas.dim.y));
    
    v2u banner_dim = v2u{s_canvas.dim.x, u32(banner_height)};
    
    u32 banner_offset = banner_dim.x * banner_dim.y;
    u32 canvas_pixel_count = s_canvas.row_stride * s_canvas.dim.y;
    u32 banner_buffer_offset = canvas_pixel_count - banner_offset;
    
    Canvas banner_canvas = Create_Sub_Canvas(&s_canvas, banner_dim, banner_buffer_offset);
    
    // Clear works on vertical sub canvases
    Clear_Canvas(&banner_canvas, s_settings.banner_background_color);
    
    v2i banner_canvas_pos = v2i{0, i32(s_canvas.dim.y - banner_dim.y)};

    Action_Context* ac = &s_global_data.action_context;
    
    GUI_Begin_Context(&s_gui_banner, &banner_canvas, ac, &s_settings.theme, banner_canvas_pos);
    {
        banner_func(&s_gui_banner);
    }
    GUI_End_Context(&s_gui_banner);
    
    v2u menu_dim = v2u{s_canvas.dim.x, s_canvas.dim.y - banner_dim.y};
    
    if(menu_dim.y <= 0)
    {
        return;
    }
    
    Canvas menu_canvas = Create_Sub_Canvas(&s_canvas, menu_dim);
    
    // Clear works on vertical sub canvases
    Clear_Canvas(&menu_canvas, s_settings.background_color);
    
    GUI_Begin_Context(&s_gui, &menu_canvas, ac, &s_settings.theme);
    {
        menu_func(&s_gui);
    }
    GUI_End_Context(&s_gui);
}


static void Run_Popup_GUI_Frame()
{
    Assert(s_global_data.popup_func);
    
    Dim_Entire_Screen(&s_canvas, 0.333f);

    BEGIN:
    GUI_Begin_Context(
        &s_gui_pop_up,
        &s_canvas, 
        &s_global_data.action_context, 
        &s_settings.theme, 
        v2i{0, 0}, 
        GUI_Anchor::top);
    
    bool panel_prop_set = (
        s_global_data.popup_panel_rect.min != v2f{0, 0} &&
        s_global_data.popup_panel_rect.max != v2f{0, 0});
    
    if(panel_prop_set)
    {
        GUI_Do_Panel(
            &s_gui_pop_up, 
            s_global_data.popup_panel_rect, 
            &s_settings.banner_background_color, 
            GUI_Highlight_Everything());
    }
    
    s_global_data.popup_func(&s_gui_pop_up);
    
    if(!panel_prop_set)
    {
        Rect bounds = GUI_Get_Bounds_In_Pixel_Space(&s_gui_pop_up);
        s_global_data.popup_panel_rect = Expand_Rect(bounds, s_gui_pop_up.theme->padding);
        GUI_End_Context(&s_gui_pop_up);
        goto BEGIN;
    }
    
    GUI_End_Context(&s_gui_pop_up);
}


static inline void Init_GUI()
{
    // globals:
    Action* htkeys = s_hotkeys;
    htkeys[Global_Hotkeys::toggle_fullscreen]           = Make_Action(Key_Code::F11, Button::START);
    htkeys[Global_Hotkeys::open_quit_popup]             = Make_Action(Key_Code::ESC, Button::BUT_Y);
    htkeys[Global_Hotkeys::display_memory]              = Make_Action(Key_Code::F8, Button::NONE);
    htkeys[Global_Hotkeys::toggle_language]             = Make_Action(Key_Code::F9, Button::NONE);
    
    // editor:
    htkeys[Editor_Hotkeys::active_pannel_toggle]        = Make_Action(Key_Code::TAB, Button::BUT_X);
    htkeys[Editor_Hotkeys::jump_to_all_events]          = Make_Action(Key_Code::F1,  Button::NONE);
    htkeys[Editor_Hotkeys::jump_to_participants]        = Make_Action(Key_Code::F2,  Button::NONE);
    htkeys[Editor_Hotkeys::jump_to_event_text]          = Make_Action(Key_Code::F3,  Button::NONE);
    htkeys[Editor_Hotkeys::jump_to_consequences]        = Make_Action(Key_Code::F4,  Button::NONE);
    htkeys[Editor_Hotkeys::save]                        = Make_Action(Key_Code::F5,  Button::L_THUMB);
    htkeys[Editor_Hotkeys::jump_left]                   = Make_Action(Key_Code::F6,  Button::L_SHLD);
    htkeys[Editor_Hotkeys::jump_right]                  = Make_Action(Key_Code::F7,  Button::R_SHLD);
    // ------------
    
    GUI_Context::platform = &s_platform;
    s_gui = GUI_Create_Context();
    s_gui_banner = GUI_Create_Context();
    s_gui_pop_up = GUI_Create_Context();
    
    for(u32 i = 0; i < Array_Lenght(s_context_pool); ++i)
        s_context_pool[i].context = GUI_Create_Context();
    
    GUI_Activate_Context(&s_gui_banner);
    
    s_gui_pop_up.flags |= GUI_Context_Flags::enable_dynamic_sliders;
    s_gui_banner.flags |= GUI_Context_Flags::enable_dynamic_sliders;
    s_gui.flags |= GUI_Context_Flags::enable_dynamic_sliders;
    
    GUI_Set_Default_Menu_Actions();
}


static inline void Clear_Editor_Format_Campaigns()
{
    if(s_global_data.on_disk_campaign_names)
    {
        Dynamic_Array<String>* on_disk_names = s_global_data.on_disk_campaign_names;
        for(String* name = Begin(on_disk_names); name < End(on_disk_names); ++name)
            name->free();
        
        s_allocator.free(s_global_data.on_disk_campaign_names);
        s_global_data.on_disk_campaign_names = 0;
    }
}


static inline void Gather_Editor_Format_Campaigns()
{
    Clear_Editor_Format_Campaigns();
    
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
}


// DOC: Gives back next available gui context, but if the given context is different that the one
// resived by this caller before, the context is reseted.
// CONSIDER: Is this a crazy way of doing this??? 
#define Get_GUI_Context_From_Pool() Get_GUI_Context_From_Pool_(__LINE__, u64(__func__))

static GUI_Context* Get_GUI_Context_From_Pool_(u64 line_id, u64 func_id)
{
    GUI_Context* result = 0;
    
    for(u32 i = 0; i < s_global_data.pooled_contexes_in_use_count; ++i)
    {
        Pooled_GUI_Context* pgc = s_context_pool + i;
        if(pgc->last_used_line == line_id && 
            pgc->last_used_func == func_id &&
            !GUI_Is_Context_Ready(&pgc->context))
        {
            result = &pgc->context;
            break;
        }
    }
    
    if(!result)
    {
        Assert(s_global_data.pooled_contexes_in_use_count < Array_Lenght(s_context_pool));
        
        Pooled_GUI_Context* pgc = s_context_pool + s_global_data.pooled_contexes_in_use_count;
        result = &pgc->context;
        
        if(pgc->last_used_line != line_id || pgc->last_used_func != func_id)
            GUI_Reset_Context(&pgc->context);
        
        pgc->last_used_line = line_id;
        pgc->last_used_func = func_id;
        
        s_global_data.pooled_contexes_in_use_count += 1;
    }
    
    return result;
}


// NOTE: Crazy shit happens here. Please don't look at it xD

enum class Loc_Identifier : u32
{
    #define L(ID, ENG, FIN) ID##,
    #include "Localisation.h"
    
    #undef L
};

char* Get_Localised_Text(Loc_Identifier LI)
{ 
    char* result = "Localisation error!";
    
    switch(LI)
    {
        #define L(ID,ENG, FI) case Loc_Identifier::##ID:                \
        {                                                               \
            switch(s_settings.language)                                 \
            {                                                           \
                case Language::english:                                 \
                {                                                       \
                    result = ENG;                                       \
                }break;                                                 \
                                                                        \
                case Language::finnish:                                 \
                {                                                       \
                    result = FI;                                        \
                }break;                                                 \
            }                                                           \
        }break;
        
        #include "Localisation.h"
        
        #undef L
    }
   
    return result;
}
#define L1(X) Get_Localised_Text(Loc_Identifier::##X)


#define L(ID, ENG, FIN)                                                 \
static constexpr char* s_##ID##_FIN[] = FIN;                            \
static constexpr char* s_##ID##_ENG[] = ENG;                    

#include "Localisation_Multi.h"

#undef L


enum class Loc_List_Identifier : u32
{
    #define L(ID, ENG, FIN) ID##,
    #include "Localisation_Multi.h"
    #undef L
};


CSTR_List Get_Localised_Texts(Loc_List_Identifier LI)
{
    char* error_stud[] = {"Localisation list error!"};
    
    CSTR_List result = {};
    result.list = error_stud;
    result.count = Array_Lenght(error_stud);

    switch(LI)
    {
        #define L(ID, ENG, FI) case Loc_List_Identifier::##ID:      \
        {                                                           \
            switch(s_settings.language)                             \
            {                                                       \
                case Language::english:                             \
                {                                                   \
                    result.list = (char**)s_##ID##_ENG;             \
                    result.count = Array_Lenght(s_##ID##_ENG);      \
                }break;                                             \
                                                                    \
                case Language::finnish:                             \
                {                                                   \
                    result.list = (char**)s_##ID##_FIN;             \
                    result.count = Array_Lenght(s_##ID##_FIN);      \
                }break;                                             \
            }                                                       \
        }break;
        
        #include "Localisation_Multi.h"
        #undef L
    }
    
    return result;
}


#define LN(X) Get_Localised_Texts(Loc_List_Identifier::##X)

#define LN1(X, I) Get_Localised_Texts(Loc_List_Identifier::##X).list[I]

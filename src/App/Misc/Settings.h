
#pragma once

static Font s_font = {
    s_terminus_font_char_width, 
    s_terminus_font_char_height, 
    (u8*)&s_terminus_font[0], 
    (u8*)&s_terminus_font_special_characters[0]
};


enum class Text_Scale : u8
{
    small,
    medium,
    large,
    custom
};


struct Settings
{
    Language language;
    bool allow_non_uniform_text_scale;
    bool allow_color_customisation;
    
    Text_Scale text_scale;

    GUI_Theme theme;

    Color warning_theme_selected_color;
    Color warning_theme_background_color; 
    Color warning_theme_outline_color;

    Color error_theme_selected_color;   
    Color error_theme_background_color;
    Color error_theme_outline_color;  
    
    Color background_color;
    Color banner_background_color;
    Color list_bg_color;
    
};
static Settings s_settings = {};


struct Settings_Save_Format
{
    static constexpr char* file_name = "SETTINGS";
    static constexpr u32 identity = PCG_Hash(1969);
    
    u32 identifier;
    u32 version;
    v2f text_scale;
    Settings settings;
};


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


v2f Get_Size_For_Text_Scale_Name(Text_Scale name)
{
    v2f result = GUI_DEFAULT_TEXT_SCALE;
    
    switch(s_settings.text_scale)
    {
        case Text_Scale::small:
        {
            result = v2f{1.5f, 1.5f};
        }break;
        
        case Text_Scale::medium:
        {
            result = v2f{2.f, 2.f};
        }break;
        
        case Text_Scale::large:
        {
            result = v2f{3.f, 3.f};
        }break;
    }
    
    return result;
}


static void Set_Settings_To_Default(bool restore_language = false)
{
    Language language = s_settings.language;
    
    s_settings = {};
    
    // -------------------------------------------------
    s_settings.text_scale = Text_Scale::medium;
    GUI_DEFAULT_TEXT_SCALE = Get_Size_For_Text_Scale_Name(s_settings.text_scale);
    // -------------------------------------------------
    
    GUI_Default_Theme_Names def_theme = GUI_Default_Theme_Names::vintage;
    s_settings.theme = GUI_Create_Default_Theme(def_theme, s_font);
    Set_Additional_Colors_Based_On_Default_Theme(def_theme);
    
    // -------------------------------------------------
    s_settings.warning_theme_selected_color         = Make_Color(128, 0, 0);
    s_settings.warning_theme_background_color       = Make_Color(255, 255, 128);
    s_settings.warning_theme_outline_color          = Make_Color(234, 143, 21);
    // -------------------------------------------------    
    s_settings.error_theme_selected_color           = Make_Color(128, 0, 0);
    s_settings.error_theme_background_color         = Make_Color(180, 70, 70);
    s_settings.error_theme_outline_color            = Make_Color(255, 235, 235);
    // -------------------------------------------------

    if(restore_language)
        s_settings.language = language;
}


static inline bool Save_Settings()
{
    bool result = false;
    
    char exe_path[255];
    if(s_platform.Get_Executable_Path(exe_path, Array_Lenght(exe_path)))
    {
        char* t = Settings_Save_Format::file_name;
        String settings_path = Create_String(&s_allocator, exe_path, s_data_folder_path, t);
        
        Settings_Save_Format payload = {};
        payload.identifier = payload.identity;
        payload.version = 1;
        payload.text_scale = GUI_DEFAULT_TEXT_SCALE;
        
        payload.settings = s_settings;
        
        result = s_platform.Write_File(settings_path.buffer, (u8*)&payload, sizeof(payload));
        
        settings_path.free();
    }
    
    return result;
}


static inline bool Try_Load_Settings()
{
    bool result = false;

    char exe_path[255];
    if(s_platform.Get_Executable_Path(exe_path, Array_Lenght(exe_path)))
    {
        char* t = Settings_Save_Format::file_name;
        String settings_path = Create_String(&s_allocator, exe_path, s_data_folder_path, t);
        
        u32 file_size;
        if(s_platform.Get_File_Size(settings_path.buffer, &file_size))
        {
            Settings_Save_Format payload = {};
            
            if(sizeof(payload) == file_size)
            {
                result = s_platform.Read_File(settings_path.buffer, (u8*)&payload, file_size);
                if(result && payload.identifier == payload.identity && payload.version == 1)
                {
                    GUI_DEFAULT_TEXT_SCALE = payload.text_scale;
                    
                    s_settings = payload.settings;
                    s_settings.theme.font = s_font;
                }
            }
        }
        
        settings_path.free();
    }
    
    return result;
}
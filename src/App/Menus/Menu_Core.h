
#pragma once

#define AUTO 0

static Color s_background_color 		= Make_Color(20, 20, 20);
static Color s_banner_background_color 	= Make_Color(40, 40, 40);
static Color s_list_bg_color 			= Make_Color(10, 10, 10);

static GUI_Theme s_theme = {};
static GUI_Context s_gui;
static GUI_Context s_gui_banner;
static GUI_Context s_gui_pop_up;

static constexpr f32 s_post_title_y_spacing = 20;

static void(*s_popup_func)() = 0;


// TODO: Figure out where to put these enums
namespace Global_Hotkeys
{
	enum T : u32
	{
		toggle_fullscreen = 0,
		open_quit_popup,
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


enum class Menus : u32
{
	main_menu = 0,
	settings_menu,
	campaigns_menu,
	select_campaign_to_play_menu,
	
	EVENT_EDITOR_BEGIN,
		EE_all_events,
		EE_participants,
		EE_text,
		EE_consequences,
	EVENT_EDITOR_END,
	
	GAME_MODE_BEGIN,
		GM_players,
		GM_let_the_games_begin,
		GM_event_display,
		GM_day_counter,
		GM_night_falls,
		GM_everyone_is_dead,
		GM_we_have_a_winner,
	GAME_MODE_END
};


struct Editor_State
{
	Events_Container event_container;
	
	u32 active_event_index = 0;
	u32 event_idx_to_delete = 0;
};
static Editor_State s_editor_state;


struct Global_Data
{
	Action_Context action_context = Action_Context();
	
	Menus active_menu = Menus::main_menu;
	
	String new_campaign_name;
	Dynamic_Array<String>* on_disk_campaign_names = 0;
	
	GUI_Theme popup_panel_theme;
	v2f popup_panel_dim = v2f{0.f, 0.f};

	Rect popup_panel_rect = {};
	
	bool force_quit_popup = false;
};
static Global_Data s_global_data = Global_Data();


static Game_State s_game_state = {};


static inline void Close_Popup()
{
	s_popup_func = 0;
	GUI_Activate_Context(&s_gui_banner);
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
	
	f32 back_button_y = f32(context->canvas->dim.y) - padding;
	
	v2f result = v2f{back_button_x, back_button_y};
	return result;
}


static inline void Set_Popup_Function(void(*popup_function)())
{
	s_global_data.popup_panel_dim = v2f{0.f, 0.f};
	
	GUI_Reset_Context(&s_gui_pop_up);
	GUI_Activate_Context(&s_gui_pop_up);
	s_popup_func = popup_function;
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

static u32 DEFAULT_BANNER_HEIGHT = 200;

static void Do_GUI_Frame_With_Banner(
	void(*banner_func)(GUI_Context*), 
	void(*menu_func)(GUI_Context*), 
	u32 banner_height = DEFAULT_BANNER_HEIGHT,
	bool enable_default_active_menu_hotkey_behavior = true)
{
	Assert(banner_height);
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
	
	v2u banner_dim = s_canvas.dim;
	banner_dim.y = Min(s_canvas.dim.y, banner_height);
	
	u32 banner_offset = banner_dim.x * banner_dim.y;
	u32 canvas_pixel_count = s_canvas.row_stride * s_canvas.dim.y;
	u32 banner_buffer_offset = canvas_pixel_count - banner_offset;
	
	Canvas banner_canvas = Create_Sub_Canvas(&s_canvas, banner_dim, banner_buffer_offset);
	
	// Clear works on vertical sub canvases
	Clear_Canvas(&banner_canvas, s_banner_background_color);
	
	v2i banner_canvas_pos = v2i{0, i32(s_canvas.dim.y - banner_dim.y)};

	GUI_Context* context = &s_gui_banner;
	
	Action_Context* ac = &s_global_data.action_context;
	
	GUI_Begin_Context(context, &banner_canvas, ac, &s_theme, banner_canvas_pos);
	{
		banner_func(context);
	}
	GUI_End_Context(context);
	
	v2u menu_dim = v2u{s_canvas.dim.x, s_canvas.dim.y - banner_dim.y};
	
	if(menu_dim.y <= 0)
	{
		return;
	}
	
	Canvas menu_canvas = Create_Sub_Canvas(&s_canvas, menu_dim);
	
	// Clear works on vertical sub canvases
	Clear_Canvas(&menu_canvas, s_background_color);
	
	context = &s_gui;
	
	GUI_Begin_Context(context, &menu_canvas, ac, &s_theme);
	{
		menu_func(context);
	}
	GUI_End_Context(context);
}


static void Do_Popup_GUI_Frame(void(*popup_func)(GUI_Context*), f32 dim_factor = 0.333f)
{
	Assert(popup_func);
	
	Dim_Entire_Screen(&s_canvas, dim_factor);

	BEGIN:
	GUI_Begin_Context(
		&s_gui_pop_up,
		&s_canvas, 
		&s_global_data.action_context, 
		&s_theme, 
		v2i{0, 0}, 
		GUI_Anchor::top);
	
	bool panel_prop_set = (
		s_global_data.popup_panel_rect.min != v2f{0, 0} &&
		s_global_data.popup_panel_rect.max != v2f{0, 0});
	
	if(panel_prop_set)
	{
		s_gui_pop_up.theme = &s_global_data.popup_panel_theme;
		GUI_Do_Panel(&s_gui_pop_up, s_global_data.popup_panel_rect);
		s_gui_pop_up.theme = &s_theme;
	}
	
	popup_func(&s_gui_pop_up);
	
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
	htkeys[Global_Hotkeys::toggle_fullscreen]/*---*/= Make_Action(Key_Code::F11, Button::START);
	htkeys[Global_Hotkeys::open_quit_popup]/*-----*/= Make_Action(Key_Code::ESC, Button::BUT_Y);
	htkeys[Global_Hotkeys::display_memory]/*------*/= Make_Action(Key_Code::F8, Button::NONE);
	
	// editor:
	htkeys[Editor_Hotkeys::active_pannel_toggle]/**/= Make_Action(Key_Code::TAB, Button::BUT_X);
	htkeys[Editor_Hotkeys::jump_to_all_events]/*--*/= Make_Action(Key_Code::F1,  Button::NONE);
	htkeys[Editor_Hotkeys::jump_to_participants]/**/= Make_Action(Key_Code::F2,  Button::NONE);
	htkeys[Editor_Hotkeys::jump_to_event_text]/*--*/= Make_Action(Key_Code::F3,  Button::NONE);
	htkeys[Editor_Hotkeys::jump_to_consequences]/**/= Make_Action(Key_Code::F4,  Button::NONE);
	htkeys[Editor_Hotkeys::save]/*----------------*/= Make_Action(Key_Code::F5,  Button::L_THUMB);
	htkeys[Editor_Hotkeys::jump_left]/*************/= Make_Action(Key_Code::F6,  Button::L_SHLD);
	htkeys[Editor_Hotkeys::jump_right]/*----------*/= Make_Action(Key_Code::F7,  Button::R_SHLD);
	// ------------
	
	GUI_Context::platform = &s_platform;
	s_gui = GUI_Create_Context();
	s_gui_banner = GUI_Create_Context();
	s_gui_pop_up = GUI_Create_Context();
	
	GUI_Activate_Context(&s_gui_banner);
	
	s_gui_pop_up.flags |= GUI_Context_Flags::enable_dynamic_sliders;
	s_gui_banner.flags |= GUI_Context_Flags::enable_dynamic_sliders;
	s_gui.flags |= GUI_Context_Flags::enable_dynamic_sliders;
	
	GUI_Set_Default_Menu_Actions();

	GUI_DEFAULT_TEXT_SCALE = v2f{2, 2};
	
	v3<u8> c;
	f32 g;
	{		
		c = {165, 80, 80};
		s_theme.selected_color = Make_Color(c.r, c.g, c.b);
		
		g = 0.5f;
		s_theme.down_color = Make_Color(u8(c.r * g), u8(c.g * g), u8(c.b * g));
		
		g = 1.3f;
		s_theme.title_color = Make_Color(u8(c.r * g), u8(c.g * g), u8(c.b * g));
	}
	
	{
		c = {80, 55, 50};
		s_theme.background_color = Make_Color(c.r, c.g, c.b);
		g = 2.f;
		s_theme.text_color = Make_Color(u8(c.r * g), u8(c.g * g), u8(c.b * g));
	}
	s_theme.widget_text_color = BLACK;
	s_theme.outline_color = BLACK;
	s_theme.write_cursor_color = Make_Color(180, 130, 150);	
	s_theme.font.data_buffer = (u8*)&s_terminus_font[0];
	s_theme.font.data_buffer_sc = (u8*)&s_terminus_font_special_characters[0];
	s_theme.font.char_width = s_terminus_font_char_width;
	s_theme.font.char_height = s_terminus_font_char_height;

	s_global_data.popup_panel_theme = [](GUI_Theme* global_theme)
	{ 
		GUI_Theme result = *global_theme;
		result.background_color = s_banner_background_color;
		result.outline_color = global_theme->selected_color;
		return result;
	}(&s_theme);
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
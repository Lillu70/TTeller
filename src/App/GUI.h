
#pragma once

static thread_local v2f GUI_AUTO_FIT 			= v2f{0, 0};

static thread_local v2f GUI_AUTO_TOP_CENTER 	= v2f{0, 0};
static thread_local v2f GUI_AUTO_TOP_RIGHT 		= v2f{0, 0};
static thread_local v2f GUI_AUTO_TOP_LEFT 		= v2f{0, 0};

static thread_local v2f GUI_AUTO_MIDDLE_RIGHT	= v2f{0, 0};
static thread_local v2f GUI_AUTO_MIDDLE_LEFT	= v2f{0, 0};

static thread_local v2f GUI_AUTO_BOTTOM_CENTER 	= v2f{0, 0};
static thread_local v2f GUI_AUTO_BOTTOM_RIGHT 	= v2f{0, 0};
static thread_local v2f GUI_AUTO_BOTTOM_LEFT 	= v2f{0, 0};

static thread_local v2f GUI_AUTO_MIDDLE 		= v2f{0, 0};


struct GUI_Theme
{
	u32 selected_color = Put_Color(250, 220, 115);
	u32 background_color = Put_Color(20, 20, 20);
	u32 down_color = Put_Color(50, 50, 50);
	u32 outline_thickness = 2;
	u32 outline_color = Put_Color(110, 110, 130);
	u32 text_color = WHITE;
	u32 title_color = Put_Color(210, 210, 230);
	f32 padding = 10;
	
	Font font;
};

 
struct GUI_Input_Acceleration_Behavior
{
	f64 input_speed_up_time = 0.2f;
	f64 input_delay_time = 0.1f;
	f64 max_speed_up_factor = 50.f;
};


struct GUI_Highlight
{
	i32 idx = 0;
	i32 highlight_count = 0;
};

namespace GUI_Menu_Actions
{
	enum : u32
	{
		back = 0,
		enter,
		up,
		down,
		left,
		right,
		mouse,
		COUNT
	};
}


namespace GUI_Link_Direction
{
	enum Type : i8
	{
		up  = 1,
		down = -1
	};	
}


enum class GUI_Build_Direction : u8
{
	up_center = 0,
	up_left,
	up_right,
	down_center,
	down_left,
	down_right,
	left_center,
	left_top,
	left_bottom,
	right_center,
	right_top,
	right_bottom
};


enum class GUI_Anchor : u8
{
	center = 0,
	top,
	top_left,
	top_right,
	left,
	right,
	bottom,
	bottom_left,
	bottom_right
};


enum class GUI_Defered_Render_Type
{
	none = 0,
	dropdown_button
};


struct GUI_Placement
{
	v2f pos;
	v2f dim;
	
	Rect rect;
};


struct GUI_Button_State
{
	bool is_pressed_down;
};


struct GUI_Slider_State
{
	f64 input_start_time;
	f64 next_input_time;
	
	bool is_held_down;
};


struct GUI_Dropdown_Button_State
{
	char** element_names;
	GUI_Theme* theme;
	
	u32 selected_element_idx;
	u32 element_count;
	Rect open_rect;
	v2f text_scale;
	v2f pos;
	v2f dim;
	
	bool is_pressed_down;
	bool is_open;
};


struct GUI_SL_Input_Field_State
{
	u32 view_offset = 0;
	u32 write_cursor_position = 0;
	u32 text_select_start_point = 0;
	f64 flicker_start_time = 0;
	f64 next_input_time = 0;
	f64 input_start_time = 0;
	
	bool text_select_mode = 0;
	bool handel_drag_mode = 0;
	bool is_pressed_down = 0;
	bool is_active = 0;
	bool draw_cursor = 0;

	static constexpr f64 flicker_delay = 3.0;
	static constexpr f64 input_delay = 0.4;
	static constexpr f64 input_speed_up_time = 0.5;
	static constexpr f64 max_speed_up_factor = 10;
};


union GUI_Element_State
{
	GUI_Button_State button;
	GUI_Slider_State slider;
	GUI_Dropdown_Button_State dropdown_button;
	GUI_SL_Input_Field_State sl_input_field;
};


struct GUI_Layout
{
	GUI_Anchor anchor;
	GUI_Build_Direction build_direction;
	
	GUI_Theme* theme;

	v2f last_element_pos = {};
	v2f last_element_dim = {};
	
	bool one_time_skip_padding = false;
};


// Some data has to be persistent across frames,
// some done not. How to seperate them?
struct GUI_Context
{
	// CONSIDER: These variables, need further thought. How are they set? When?
	Canvas* canvas = 0;
	Platform_Calltable* platform = 0;
	Action actions[GUI_Menu_Actions::COUNT] = {}; // <- Make this global?
	
	i32 selected_index = 0;
	i32 widget_count = 0; 
	u32 selected_id = 0;
	
	i32 last_widget_count = 0;
	v2i last_cursor_position;
	v2i cursor_position;
	
	Rect cursor_mask_area = {};
	
	GUI_Layout layout;
	
	u32 layout_stack_count = 0;
	GUI_Layout layout_stack[5];
	
	GUI_Defered_Render_Type defered_render = GUI_Defered_Render_Type::none;
	
	GUI_Element_State selection_state = {};
	
	bool cursor_mask_validation = 0;
	bool cursor_mask_enabled = 0;
	
	bool disable_kc_navigation = 0; // kc = Keyboard/Controller
};
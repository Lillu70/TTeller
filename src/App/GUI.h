
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

static thread_local v2f GUI_DEFAULT_TEXT_SCALE 	= v2f{2, 2};

static thread_local f32 GUI_MOUSE_SCROLL_SPEED 	= 50;

struct GUI_Theme
{
	Color selected_color = Make_Color(250, 220, 115);
	Color background_color = Make_Color(20, 20, 20);
	Color down_color = Make_Color(50, 50, 50);
	Color outline_color = Make_Color(110, 110, 130);
	Color text_color = WHITE;
	Color title_color = Make_Color(210, 210, 230);
	Color write_cursor_color = BLACK;
	Color write_cursor_limit_color = RED;
	
	u32 outline_thickness = 2;
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


enum class GUI_Cardinal_Direction : u8
{
	left_right = 0,
	up_down
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
	v2f drag_offset;
	
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


// NOTE: Is actually used for ML as well atm.
struct GUI_SL_Input_Field_State
{
	u32 view_offset = 0;
	u32 write_cursor_position = 0;
	u32 text_select_start_point = 0;
	u32 mouse_press_down_point = 0;
	u32 click_p = 0;

	v2f mouse_scroll_bar_drag_offset = v2f{0, 0};
	
	f64 flicker_start_time = 0;
	f64 next_input_time = 0;
	f64 input_start_time = 0;
	f64 mouse_hold_time = 0;
	
	// CONSIDER: Flags?
	bool scroll_bar_drag_mode = 0;
	bool text_select_mode = 0;
	bool is_pressed_down = 0;
	bool is_active = 0;
	bool draw_cursor = 0;
	bool cursor_is_active = 0; // NOTE: ML only?

	static constexpr f64 mouse_hold_delay = 3.0;
	static constexpr f64 flicker_delay = 3.0;
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


namespace GUI_Context_Flags
{
	enum : u32
	{
		soft_ignore_selection 		= 1 << 0, // Makes it so that nothing is selected, but mouse selection is still possible to exit this state.
		enable_dynamic_sliders 		= 1 << 1,
		cursor_mask_validation 		= 1 << 2,
		cursor_mask_enabled 		= 1 << 3,
		disable_wrapping 			= 1 << 4,
		disable_kc_navigation 		= 1 << 5,
		disable_mouse_scroll 		= 1 << 6,
		context_ready 				= 1 << 7,
		maxout_vertical_slider 		= 1 << 8,
		maxout_horizontal_slider	= 1 << 9,
		hard_ignore_selection		= 1 << 10, // Makes it so that nothing is selected and to leave this state this flag has to manually set to 0.
	}; // max shift is 31
}


struct GUI_Context
{
	Canvas* canvas = 0;
	Platform_Calltable* platform = 0;
	Action* actions;
	Rect bounds_rel_anchor_base = {};
	
	u32 flags = 0;
	i32 selected_index = 0;
	i32 widget_count = 0; 
	u32 selected_id = 0;
	f32 dynamic_slider_girth = 15.f;
	
	i32 last_widget_count = 0;
	v2i last_cursor_position;
	v2i cursor_position;
	v2f cursor_fpos;
	v2i canvas_pos = {};
	v2f anchor_base = {};
	v2f selected_element_pos = {};
	v2f selected_element_dim = {};
	
	Rect cursor_mask_area = {};
	Rect canvas_rect = {};
	
	GUI_Layout layout;
	
	u32 layout_stack_count = 0;
	GUI_Layout layout_stack[5];
	
	GUI_Defered_Render_Type defered_render = GUI_Defered_Render_Type::none;
	
	GUI_Element_State selection_state = {};
};


static bool GUI_Do_Handle_Slider(
	GUI_Context* context, 
	v2f* pos, 
	v2f* dim, 
	f32* value,
	f32 max = 100,
	f32 min = 0,
	GUI_Cardinal_Direction cardinal_dir = GUI_Cardinal_Direction::left_right,
	f32 step_count = 0,
	GUI_Input_Acceleration_Behavior inp_accel = GUI_Input_Acceleration_Behavior());
	
	
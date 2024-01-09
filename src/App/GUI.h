
#pragma once

// TODO: Add function definations.


/*

*/

enum class GUI_Widget_Type : u16
{
	none = 0,
	button,
	slider,
	checkbox,
	key_listener,
	list_button,
	input_field,
	COUNT
};


enum class GUI_Static_Widget_Type : u16
{
	none = (u16)GUI_Widget_Type::COUNT,
	text,
	image
};


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


enum class GUI_Menu_Actions
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


enum class GUI_Link_Direction : u8
{
	up  = 0,
	down
};


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


struct GUI_Widget_Header
{
	GUI_Widget_Type type = GUI_Widget_Type::none;	
	v2f position = {};
	v2f dimensions = {};
	GUI_Theme* theme = 0;
};


struct GUI_Static_Widget_Header
{
	GUI_Static_Widget_Type type = GUI_Static_Widget_Type::none;
	u16 highlight_idx = U16_MAX;
	
	v2f position = {};
	GUI_Theme* theme = 0;
};


struct GUI_Button
{
	char* text = 0;
	v2f text_scale = v2f{1, 1};
	u32 event_id = 0;
	void(*on_click)(u32) = 0;
};


struct GUI_List_Button
{
	char* text = 0;
	v2f text_scale = v2f{1, 1};
	u32 event_id = 0;
	u32 list_element_count = 2;
	void(*on_select)(u32, u32) = 0;
};


struct GUI_List_Button_Element
{
	char* text;
};


struct GUI_Input_Field
{
	String* str = 0;
	bool(*character_check)(GUI_Input_Field*, char) = 0;
	v2f text_scale = v2f{1, 1};
	u16 character_limit = U16_MAX;
};


struct GUI_Text
{
	char* text = 0;
	v2f text_scale = v2f{1,1};
	bool is_title = 0;
};


struct GUI_Slider
{	
	f32 value = 0; // 0-1.f
	f32 step = 0.01f;
	f32 min = 0;
	f32 max = 1;
	f32 action_speed_up_time = 1.f;
	f32 action_tick_rate = 0.05f;
	
	void(*on_value_change)(GUI_Slider*) = 0;
};


struct GUI_Checkbox
{
	bool is_checked = 0;
	void(*on_value_change)(GUI_Checkbox*) = 0;
};


struct GUI_Key_Listener
{
	Action* action_array = 0;   //ptr
	u32 action_idx = 0;         //offset
	void(*on_trigger)(Key_Code, Button, Action*, u32) = 0;
};


struct GUI_Selection_State
{
	// NOTE: This shit is a fucking trainwreck..
	// Having to type this "handler->selection_state.element.XXXXXX.thing",
	// When accessing these gets old very fast.
	// CONSIDER: A better way of doing this.
	
	union Element
	{
		struct Button
		{
			bool is_pressed;
		};
		Button button;
		
		struct List_Button
		{
			bool is_pressed;
			bool is_open;
			u32 selected_idx;
		};
		List_Button list_button;
		
		struct Checkbox
		{
			bool is_pressed;
		};
		Checkbox checkbox;
		
		struct Slider
		{
			bool is_clicked;
			
			f64 last_time_stamp = 0;
			f32 full_action_time = 0;
			f32 accum_action_time = 0;
		};
		Slider slider;
		
		struct Input_Field
		{
			u32 write_cursor_position = 0;
			f64 flicker_start_time = 0;
			
			static constexpr f64 flicker_delay = 3.0;
			
			bool is_active = 0;
			bool is_pressed = 0;
			bool draw_cursor = 0;
		};
		Input_Field input_field;
	};
	Element element = {};
	
	bool cursor_on_selection = false;
};


struct GUI_Frame
{
	u16 widget_count = 0;
	u16 static_widget_count = 0;
	
	// CONSIDER: Frame could use a selection ptr instead.
	//		At this point the memory is stable and is reference safe.
	u16 selected_idx = U16_MAX;
	
	GUI_Link_Direction ld = GUI_Link_Direction::up;
	
	void* memory = 0; 
	GUI_Widget_Header** random_access_table = 0;
	GUI_Widget_Header* widgets = 0;
	GUI_Static_Widget_Header* static_widgets = 0;
	
	
	void(*on_frame_close)() = 0;
	void(*on_back_action)() = 0;
	GUI_Frame* prev_frame = 0;
};


struct GUI_Handler
{
	GUI_Frame active_frame = GUI_Frame();
	
	v2i last_cursor_position = v2i{-1, -1};
	
	GUI_Selection_State selection_state = {};

	GUI_Theme default_theme = GUI_Theme();
	Action actions[(u32)GUI_Menu_Actions::COUNT] = {};
};


// TODO: Rename this thing!
struct GUI_Add_Result
{
	GUI_Widget_Header* widget = 0;
	u16 idx = 0;
};


struct Placement_Storage
{
	v2f pos;
	v2f dim;
	GUI_Build_Direction bl;
};


struct GUI_Builder
{
	Linear_Allocator* allocator = 0;
	Linear_Allocator allocator_state_copy = {};
	
	GUI_Theme* theme = 0;
	
	GUI_Link_Direction link_direction = GUI_Link_Direction::up;
	GUI_Build_Direction build_direction = GUI_Build_Direction::down_center;
	GUI_Anchor anchor = GUI_Anchor::center;
	
	v2f last_element_pos = {};
	v2f last_element_dim = {};
	
	u16 widget_count = 0;
	u16 static_widget_count = 0;
	
	u16 selected_idx = 0;
	
	u16 widget_memory_count = 0;
	u16 static_widget_memory_count = 0;
	
	GUI_Add_Result first_element = {};
	GUI_Add_Result last_element = {};
	
	Placement_Storage placement_stack[5] = {};
	u32 placement_stack_count = 0;
};

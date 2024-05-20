
# pragma once

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

static void Do_Main_Menu_Name_New_Campaign_Popup();
static void Do_Event_Editor_On_Exit_Popup();

// TODO: FUCKING RENAME THESE!
enum class Menus : u32
{
	none = 0,
	main_menu,
	campaigns_menu,
	event_editor_participants,
	event_editor_text,
	event_editor_consequences,
	all_events,
};


struct Global_Data
{
	Action menu_actions[GUI_Menu_Actions::COUNT] = {};
	
	Menus active_menu = Menus::none;
	
	Events_Container event_container;
	
	u32 active_event_index = 0;
	
	String new_campaign_name;
	Dynamic_Array<String>* on_disk_campaign_names = 0;
};
static Global_Data s_global_data = Global_Data();


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
	GUI_Reset_Context(&s_gui_pop_up);
	GUI_Context::active_context_id = s_gui_pop_up._context_id;
	s_popup_func = popup_function;
}


static inline void Init_GUI()
{
	s_gui = GUI_Create_Context();
	s_gui_banner = GUI_Create_Context();
	s_gui_pop_up = GUI_Create_Context();
	
	s_gui_pop_up.flags |= GUI_Context_Flags::enable_dynamic_sliders;
	
	GUI_Set_Default_Menu_Actions(&s_global_data.menu_actions[0]);

	GUI_DEFAULT_TEXT_SCALE = v2f{2, 2};
	
	// Make Test event:
	#if 0
	String event_name = Create_String(&s_allocator, "Peformance test event");
	Event_State* event = Push(&s_global_data.event_container.events, &s_allocator);
	Init_Event_Takes_Name_Ownership(event, &s_allocator, &event_name);
	
	static constexpr char* long_text = 
		"\xE4\xF6\xC4\xD6QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm,.:;!#1234567890";
	
	u32 long_text_leng = Null_Terminated_Buffer_Lenght(long_text);
	u32 t = 0;
	u32 t2 = 0;
	for(u32 p = 0; p < Event_State::max_participent_count; ++p)
	{
		Participent* new_participent = Create_Participent(&event->participents, &s_allocator);
		
		for(u32 r = 0; r < Participent::max_requirements; ++r)
		{
			u32 mod = u32(Participation_Requirement::Type::COUNT);
			Participation_Requirement::Type new_type = (Participation_Requirement::Type)(t++ % mod);
			
			Participation_Requirement* new_req = Push(&new_participent->reqs, &s_allocator);
			Init_Participation_Requirement(new_req, new_type, &s_allocator, long_text_leng + 1);
			if(new_req->mark.buffer)
			{
				Mem_Copy(new_req->mark.buffer, long_text, long_text_leng + 1);
				new_req->mark.lenght = long_text_leng;
			}
		}
		
		for(u32 c = 0; c < Participent::max_consequenses - 1; ++c)
		{
			u32 mod = u32(Event_Consequens::Type::COUNT) - 1;
			Event_Consequens::Type new_type = (Event_Consequens::Type)((t2++ % mod) + 1);
			
			Event_Consequens* new_con = Push(&new_participent->cons, &s_allocator);
			Init_Event_Consequense(new_con, new_type, &s_allocator, long_text_leng + 1);
			if(new_con->str.buffer)
			{
				Mem_Copy(new_con->str.buffer, long_text, long_text_leng + 1);
				new_con->str.lenght = long_text_leng;
			}
		}
		
		// add death
		{
			static constexpr char* death_text = "123";
			Event_Consequens* new_con = Push(&new_participent->cons, &s_allocator);
			
			Init_Event_Consequense(new_con, Event_Consequens::Type::death, &s_allocator);
			
			Mem_Copy(new_con->str.buffer, death_text, 4);
			new_con->str.lenght = 3;
		}
	}
	
	#endif
	// -----
	
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
		
		s_theme.outline_color = BLACK;
		s_theme.text_color = BLACK;
		s_theme.write_cursor_color = Make_Color(180, 130, 150);
		
	}
	
	s_theme.font.data_buffer = (u8*)&s_terminus_font[0];
	s_theme.font.data_buffer_sc = (u8*)&s_terminus_font_special_characters[0];
	s_theme.font.char_width = s_terminus_font_char_width;
	s_theme.font.char_height = s_terminus_font_char_height;
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
	u32 banner_height = 200)
{
	Assert(banner_height);
	Assert(banner_func);
	Assert(menu_func);
	
	Action* actions = s_global_data.menu_actions;
	
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
	
	GUI_Begin_Context(context, &s_platform, &banner_canvas, actions, &s_theme, banner_canvas_pos);
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
	
	GUI_Begin_Context(context, &s_platform, &menu_canvas, actions, &s_theme);
	{
		menu_func(context);
	}
	GUI_End_Context(context);
}


static void Do_Event_Editor_All_Events_Frame()
{
	void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		v2f title_scale = v2f{4.f, 4.f};
		Font* font = &context->theme->font;
		v2f back_button_dim = GUI_Tight_Fit_Text("<", font, title_scale);
		if(GUI_Do_Button(context, &GUI_AUTO_TOP_LEFT, &back_button_dim, "<"))
			Set_Popup_Function(Do_Event_Editor_On_Exit_Popup);
		
		GUI_Push_Layout(context);
		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		char* title_text = s_global_data.event_container.campaign_name.buffer;
		GUI_Do_Text(context, AUTO, title_text, {}, title_scale, true);
		
		f32 title_height = context->layout.last_element.dim.y;
		f32 title_max_x = context->layout.last_element.rect.max.x - context->anchor_base.x;
		
		GUI_Pop_Layout(context);
		
		static bool jump_into_new_event = true;
		
		GUI_Do_Spacing(context, v2f{0, s_post_title_y_spacing});
		
		v2f checkbox_dim = v2f{30, 30};
		GUI_Do_Checkbox(context, AUTO, &checkbox_dim, &jump_into_new_event);
		
		GUI_Push_Layout(context);
		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		GUI_Do_Text(context, AUTO, "Hypp\xE4\xE4 tapahtumaan.", GUI_Highlight_Prev(context));			
		
		GUI_Pop_Layout(context);
		
		if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Luo uusi p\xE4iv\xE4 tapahtuma"))
		{
			if(jump_into_new_event)
			{
				s_global_data.active_menu = Menus::event_editor_participants;
				s_global_data.active_event_index = s_global_data.event_container.day_event_count;			
			}

			String unique_name = Generate_Unique_Name(
				Begin(s_global_data.event_container.events), 
				s_global_data.event_container.day_event_count,
				&s_allocator);
			
			u32 temp_count = s_global_data.event_container.events->count;
			
			// NOTE: Just to grow the array if needed. 
			// the actual memory contents is irrelevant at this point in time.
			Push(&s_global_data.event_container.events, &s_allocator);
			
			Event_State* buffer = Begin(s_global_data.event_container.events);
			
			// Checks if there are any night events.
			if(temp_count - s_global_data.event_container.day_event_count > 0)
			{
				Insert_Element_Into_Packed_Array(
					buffer,
					buffer + temp_count,
					&temp_count,
					sizeof(*buffer),
					s_global_data.event_container.day_event_count);				
			}
			
			Event_State* event = buffer + s_global_data.event_container.day_event_count;
			Init_Event_Takes_Name_Ownership(event, &s_allocator, &unique_name);
			
			s_global_data.event_container.day_event_count += 1;
		}
		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		if(GUI_Do_Button(context, AUTO, AUTO, "Luo uusi y\xF6 tapahtuma"))
		{
			if(jump_into_new_event)
			{
				s_global_data.active_menu = Menus::event_editor_participants;
				s_global_data.active_event_index = s_global_data.event_container.events->count;			
			}
			
			String unique_name = Generate_Unique_Name(
				Begin(s_global_data.event_container.events) + s_global_data.event_container.day_event_count, 
				s_global_data.event_container.events->count - s_global_data.event_container.day_event_count,
				&s_allocator);
			
			Event_State* event = Push(&s_global_data.event_container.events, &s_allocator);
			Init_Event_Takes_Name_Ownership(event, &s_allocator, &unique_name);
		}
		
		// -- title bar buttons --	
		f32 padding = context->theme->padding;
		
		static constexpr char* save_text = "Tallenna";
		
		f32 w1 = GUI_Tight_Fit_Text(save_text, font).x + padding;
		
		f32 ctrl_buttons_width = w1 + context->dynamic_slider_girth + padding * 2;
		v2f title_row_pos 
			= Get_Title_Bar_Row_Placement(context, title_max_x, padding, ctrl_buttons_width);
		
		v2f dim = v2f{w1, title_height};
	
		// Save button.
		if(GUI_Do_Button(context, &title_row_pos, &dim, save_text))
		{
			Serialize_Campaign(s_global_data.event_container, &s_platform);
		}
		
	}; // ----------------------------------------------------------------------------------------
	
	void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		if(s_global_data.active_menu != Menus::all_events)
		{
			return;
		}
		
		u32 border_width = 30;
		u32 border_width_x2 = border_width * 2;
		u32 canvas_half_width = context->canvas->dim.x / 2;
		u32 sub_x = canvas_half_width - border_width_x2;
		
		char* day_list_text = "P\xE4iv\xE4 tapahtumat:";

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
			GUI_Do_Text(context, &day_list_text_pos, day_list_text, {}, GUI_DEFAULT_TEXT_SCALE, true);
			GUI_Do_Text(context, &night_list_text_pos, "Y\xF6 tapahtumat:", {}, GUI_DEFAULT_TEXT_SCALE, true);			
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
		
		// Day list sub gui
		{
			v2u day_list_buffer_offset = v2u{border_width, border_width};
			Canvas event_list_day_canvas = Create_Sub_Canvas(&s_canvas, sub_dim, day_list_buffer_offset);
			Clear_Sub_Canvas(&event_list_day_canvas, s_list_bg_color);
			
			static GUI_Context gui_event_list_day = GUI_Create_Context();
			
			Inverse_Bit_Mask(&gui_event_list_day.flags,  GUI_Context_Flags::hard_ignore_selection);
			u32 set_mask = GUI_Context_Flags::enable_dynamic_sliders;
			set_mask |= (context->flags &  GUI_Context_Flags::hard_ignore_selection);
			gui_event_list_day.flags |= set_mask;
			
			GUI_Begin_Context(
				&gui_event_list_day,
				&s_platform,
				&event_list_day_canvas,
				(Action*)s_global_data.menu_actions,
				&s_theme,
				day_list_buffer_offset.As<i32>());
			{
				Event_State* begin = Begin(s_global_data.event_container.events);
				
				v2f* pos = &GUI_AUTO_TOP_LEFT;
				for(u32 i = 0; i < s_global_data.event_container.day_event_count; ++i)
				{
					Event_State* e = begin + i;
					
					GUI_Placement rcp = gui_event_list_day.layout.last_element;
					
					// Destroy event
					if(GUI_Do_Button(&gui_event_list_day, pos, &GUI_AUTO_FIT, "X"))
					{
						pos = 0;
						gui_event_list_day.layout.last_element = rcp;
						
						if(i < s_global_data.event_container.day_event_count)
							s_global_data.event_container.day_event_count -= 1;
						
						Delete_Event(s_global_data.event_container.events, &s_allocator, i--);
						continue;
					}
					pos = 0;
					
					GUI_Push_Layout(&gui_event_list_day);
					
					gui_event_list_day.layout.build_direction = GUI_Build_Direction::right_center;
					
					if(GUI_Do_Button(&gui_event_list_day, AUTO, &GUI_AUTO_FIT, e->name.buffer))
					{
						s_global_data.active_menu = Menus::event_editor_participants;
						s_global_data.active_event_index = i;
					}
					
					GUI_Pop_Layout(&gui_event_list_day);
				}
				
			}
			GUI_End_Context(&gui_event_list_day);
		}
		
		// Night list sub gui
		{
			v2u night_list_buffer_offset = v2u{border_width * 3 + sub_x, border_width};
			Canvas event_list_night_canvas = Create_Sub_Canvas(&s_canvas, sub_dim, night_list_buffer_offset);
			Clear_Sub_Canvas(&event_list_night_canvas, s_list_bg_color);
			
			static GUI_Context gui_event_list_night = GUI_Create_Context();
			
			Inverse_Bit_Mask(&gui_event_list_night.flags,  GUI_Context_Flags::hard_ignore_selection);
			u32 set_mask = GUI_Context_Flags::enable_dynamic_sliders;
			set_mask |= (context->flags &  GUI_Context_Flags::hard_ignore_selection);
			gui_event_list_night.flags |= set_mask;
			
			GUI_Begin_Context(
				&gui_event_list_night,
				&s_platform,
				&event_list_night_canvas,
				(Action*)s_global_data.menu_actions,
				&s_theme,
				night_list_buffer_offset.As<i32>());
			{
				Event_State* begin = Begin(s_global_data.event_container.events);
				
				v2f* pos = &GUI_AUTO_TOP_LEFT;
				for(u32 i = s_global_data.event_container.day_event_count; 
					i < s_global_data.event_container.events->count; ++i)
				{
					Event_State* e = begin + i;
					
					GUI_Placement rcp = gui_event_list_night.layout.last_element;
					
					// Destroy event
					if(GUI_Do_Button(&gui_event_list_night, pos, &GUI_AUTO_FIT, "X"))
					{
						pos = 0;
						gui_event_list_night.layout.last_element = rcp;
						if(i < s_global_data.event_container.day_event_count)
							s_global_data.event_container.day_event_count -= 1;
						
						Delete_Event(s_global_data.event_container.events, &s_allocator, i--);
						continue;
					}
					pos = 0;
					
					GUI_Push_Layout(&gui_event_list_night);
					
					gui_event_list_night.layout.build_direction = GUI_Build_Direction::right_center;
					
					if(GUI_Do_Button(&gui_event_list_night, AUTO, &GUI_AUTO_FIT, e->name.buffer))
					{
						s_global_data.active_menu = Menus::event_editor_participants;
						s_global_data.active_event_index = i;
					}
					
					GUI_Pop_Layout(&gui_event_list_night);
				}
				
			}
			GUI_End_Context(&gui_event_list_night);
		}
		
	}; // ----------------------------------------------------------------------------------------
	
	Do_GUI_Frame_With_Banner(banner_func, menu_func);
}


static void Do_Event_Editor_Participants_Frame()
{
	void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		
		v2f title_scale = v2f{4.f, 4.f};
		Font* font = &context->theme->font;
		v2f back_button_dim = GUI_Tight_Fit_Text("<", font, title_scale);
		if(GUI_Do_Button(context, &GUI_AUTO_TOP_LEFT, &back_button_dim, "<"))
		{
			s_global_data.active_menu = Menus::all_events;
		}
		
		GUI_Push_Layout(context);
		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		GUI_Do_Text(context, AUTO, "Osallistujat", {}, title_scale, true);
		f32 title_height = context->layout.last_element.dim.y;
		f32 title_max_x = context->layout.last_element.rect.max.x - context->anchor_base.x;
		
		GUI_Pop_Layout(context);
		
		GUI_Do_Spacing(context, v2f{0, s_post_title_y_spacing});
		
		Event_State* event = Begin(s_global_data.event_container.events) + s_global_data.active_event_index;
		if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Lis\xE4\xE4 uusi osallistuja"))
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
			if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Poista osallistujat"))
				Delete_All_Participants_From_Event(event, &s_allocator);
		}	
		
		if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Lis\xE4\xE4 yleis vaatimus"))
		{
			Global_Mark_Requirement* gmr = Push(&event->global_mark_reqs, &s_allocator);
			*gmr = {};
			gmr->mark = Create_String(&s_allocator, 12);
		}
		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		// -- title bar buttons --	
		f32 padding = context->theme->padding;
		static constexpr char* go_to_event_text_text = "Tapahtuma Teksti";
		static constexpr char* go_to_consequences_text = "Seuraamukset";
		
		f32 w1 = GUI_Tight_Fit_Text(go_to_event_text_text, font).x + padding;
		f32 w2 = GUI_Tight_Fit_Text(go_to_consequences_text, font).x + padding;
		
		f32 ctrl_buttons_width = w1 + w2 + context->dynamic_slider_girth + padding * 2;
		v2f title_row_pos 
			= Get_Title_Bar_Row_Placement(context, title_max_x, padding, ctrl_buttons_width);
		
		v2f dim = v2f{w1, title_height};
		if(GUI_Do_Button(context, &title_row_pos, &dim, go_to_event_text_text))
		{
			s_global_data.active_menu = Menus::event_editor_text;	
		}
		
		dim = v2f{w2, title_height};
		if(GUI_Do_Button(context, AUTO, &dim, go_to_consequences_text))
		{
			s_global_data.active_menu = Menus::event_editor_consequences;
		}
		
	}; // ----------------------------------------------------------------------------------------
	
	
	void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		Event_State* event = Begin(s_global_data.event_container.events) + s_global_data.active_event_index;
	
		static constexpr f32 collumn_min_width = 300;
		f32 padding = context->theme->padding;
		
		v2f* start_pos_ptr = &GUI_AUTO_TOP_LEFT;
		if(event->global_mark_reqs->count)
		{
			
			GUI_Do_Text(context, start_pos_ptr, "Yleis vaatimukset:", {}, GUI_DEFAULT_TEXT_SCALE, true);
			start_pos_ptr = 0;
			
			GUI_Push_Layout(context);
			
			f32 collumn_start = GUI_Get_Collumn_Start(context, X_AXIS);
			
			Global_Mark_Requirement* global_mark_reqs = Begin(event->global_mark_reqs);
			for(u32 i = 0; i < event->global_mark_reqs->count; ++i)
			{
				GUI_Placement rcp = context->layout.last_element;
				
				v2f dim = v2f{36, 36};
				if(GUI_Do_Button(context, start_pos_ptr, &dim, "X"))
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
				
				GUI_Do_Text(context, AUTO, "Yleis merkki", GUI_Highlight_Prev(context));
				
				f32 right_side 
					= context->layout.last_element.pos.x + context->layout.last_element.dim.x / 2; 
				
				GUI_Pop_Layout(context);
				
				f32 width = right_side - left_size;
				GUI_Do_SL_Input_Field(context, AUTO, &width, &gmr->mark);
				
				if(u32 s = GUI_Do_Dropdown_Button(
					context, AUTO, AUTO, 
					(char*)s_exists_statement_names[u32(gmr->mark_exists)],
					Array_Lenght(s_exists_statement_names),
					(char**)s_exists_statement_names))
				{
					gmr->mark_exists = Exists_Statement(s - 1);
				}
				
				if(gmr->mark_exists == Exists_Statement::does_have)
				{
					GUI_Do_Text(context, AUTO, "Kestoa j\xE4ljell\xE4:", GUI_Highlight_Next(context, 2));
					
					// Numerical_Relation
					if(u32 s = GUI_Do_Dropdown_Button(
						context, AUTO, &GUI_AUTO_FIT, 
						(char*)s_numerical_relation_names[u32(gmr->numerical_relation)],
						Array_Lenght(s_numerical_relation_names),
						(char**)s_numerical_relation_names))
					{
						gmr->numerical_relation = Numerical_Relation(s - 1);
					}
					
					GUI_Push_Layout(context);
					
					context->layout.build_direction = GUI_Build_Direction::right_center;
					
					// Target value
					if(u32 s = GUI_Do_Dropdown_Button(
						context, AUTO, &GUI_AUTO_FIT, 
						(char*)s_duration_names[gmr->relation_target],
						Array_Lenght(s_duration_names) - 1,
						(char**)s_duration_names + 1))
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
			GUI_Do_Pannel(context, AUTO, &pannel_dim);
		}		
		
		// -- participants --
		
		GUI_Do_Text(context, start_pos_ptr, "Osallistujat:", {}, GUI_DEFAULT_TEXT_SCALE, true);
		
		context->layout.build_direction = GUI_Build_Direction::down_left;
		
		for(u32 p = 0; p < event->participents->count; ++p)
		{
			Participent* parti = Begin(event->participents) + p;
			
			v2f dim = v2f{36, 36};
			
			GUI_Placement rcp = context->layout.last_element;
			
			if(GUI_Do_Button(context, AUTO, &dim, "X"))
			{
				Delete_Participent(p--, event->participents, &s_allocator);
				
				context->layout.last_element = rcp;
				continue;
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
				*typing_marker = '\\';
			}
			
			GUI_Do_Text(context, AUTO, typing_marker, GUI_Highlight_Prev(context));
			
			GUI_Pop_Layout(context);
			
			if(u32 s = GUI_Do_Dropdown_Button(
				context, 
				AUTO, 
				&GUI_AUTO_FIT, 
				"Lis\xE4\xE4 vaatimus", 
				(u32)Participation_Requirement::Type::COUNT, 
				(char**)Participation_Requirement::type_names))
			{
				if(parti->reqs->count < parti->max_requirements)
				{
					Participation_Requirement::Type new_type = (Participation_Requirement::Type)(s - 1);
					Init_Participation_Requirement(Push(&parti->reqs, &s_allocator), new_type, &s_allocator);
				}
			}
			
			GUI_Do_Spacing(context, AUTO);
			
			Participation_Requirement* requirement_array = Begin(parti->reqs);
			for(u32 r = 0; r < parti->reqs->count; ++r)
			{
				Participation_Requirement* req = requirement_array + r;
				
				rcp = context->layout.last_element;
				
				if(GUI_Do_Button(context, AUTO, &dim, "X"))
				{
					Make_Requirement_Hollow(req);
					
					u32 size = sizeof(Participation_Requirement);
					Remove_Element_From_Packed_Array(requirement_array, &parti->reqs->count, size, r--);
					
					context->layout.last_element = rcp;
					continue;
				}
				
				GUI_Push_Layout(context);
				
				context->layout.build_direction = GUI_Build_Direction::right_center;
				
				char* req_type_text = (char*)Participation_Requirement::type_names[(u32)req->type];
				GUI_Do_Text(context, AUTO, req_type_text, GUI_Highlight_Prev(context));
				
				f32 right_side_x = context->layout.last_element.pos.x + context->layout.last_element.dim.x / 2;
					
				GUI_Pop_Layout(context);
				
				f32 left_side_x = context->layout.last_element.pos.x - context->layout.last_element.dim.x / 2; 
				
				switch(req->type)
				{
					case Participation_Requirement::Type::mark_item:
					case Participation_Requirement::Type::mark_personal:
					{
						f32 width = right_side_x - left_side_x;
						GUI_Do_SL_Input_Field(context, AUTO, &width, &req->mark);
						
						if(u32 s = GUI_Do_Dropdown_Button(
							context, AUTO, AUTO, 
							(char*)s_exists_statement_names[u32(req->mark_exists)],
							Array_Lenght(s_exists_statement_names),
							(char**)s_exists_statement_names))
						{
							req->mark_exists = Exists_Statement(s - 1);
						}
						
						if(req->mark_exists == Exists_Statement::does_have)
						{
							GUI_Do_Text(context, AUTO, "Kestoa j\xE4ljell\xE4:", GUI_Highlight_Next(context, 2));
							
							
							// Numerical_Relation
							if(u32 s = GUI_Do_Dropdown_Button(
								context, AUTO, &GUI_AUTO_FIT, 
								(char*)s_numerical_relation_names[u32(req->numerical_relation)],
								Array_Lenght(s_numerical_relation_names),
								(char**)s_numerical_relation_names))
							{
								req->numerical_relation = Numerical_Relation(s - 1);
							}
							
							GUI_Push_Layout(context);
							
							context->layout.build_direction = GUI_Build_Direction::right_center;
							
							// Target value
							if(u32 s = GUI_Do_Dropdown_Button(
								context, AUTO, &GUI_AUTO_FIT, 
								(char*)s_duration_names[req->relation_target],
								Array_Lenght(s_duration_names) - 1,
								(char**)s_duration_names + 1))
							{
								req->relation_target = u16(s);
							}
							
							GUI_Pop_Layout(context);
						}
					}break;
					
					case Participation_Requirement::Type::character_stat:
					{
						// Stat
						if(u32 s = GUI_Do_Dropdown_Button(
							context, AUTO, &GUI_AUTO_FIT, 
							(char*)Character_Stat::stat_names[u32(req->stat_type)],
							Array_Lenght(Character_Stat::stat_names),
							(char**)Character_Stat::stat_names))
						{
							req->stat_type = Character_Stat::Stats(s - 1);
						}
						
						GUI_Push_Layout(context);
						
						context->layout.build_direction = GUI_Build_Direction::right_center;
						
						// Numerical_Relation
						static const char* value_option_names[] = {"0", "1", "2", "3"};
						if(u32 s = GUI_Do_Dropdown_Button(
							context, AUTO, &GUI_AUTO_FIT, 
							(char*)s_numerical_relation_names[u32(req->numerical_relation)],
							Array_Lenght(s_numerical_relation_names),
							(char**)s_numerical_relation_names))
						{
							req->numerical_relation = Numerical_Relation(s - 1);
						}
						
						// Target value
						if(u32 s = GUI_Do_Dropdown_Button(
							context, AUTO, AUTO, 
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
							GUI_Do_Text(context, AUTO, "Mahdoton!", {0}, v2f{1.f,1.f}, true);
						}
						else if((req->numerical_relation == Numerical_Relation::less_than_equals &&
							req->relation_target == Array_Lenght(value_option_names) - 1) || 
							(req->numerical_relation == Numerical_Relation::greater_than_equals &&
							req->relation_target == 0))
						{
							GUI_Do_Text(context, AUTO, "Aina totta!", {0}, v2f{1.f,1.f}, true);
						}
						
					}break;
				}
				
				v2f spacing = v2f{context->layout.last_element.dim.x, s_theme.padding * 2};
				GUI_Do_Spacing(context, &spacing);	
			}
			
			GUI_Pop_Layout(context);
			
			GUI_End_Collumn(context, collumn_min_width, collumn_start, X_AXIS);
		}
	}; // ----------------------------------------------------------------------------------------
	
	Do_GUI_Frame_With_Banner(banner_func, menu_func, 160);
}


static void Do_Event_Editor_Text_Frame()
{
	void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		Event_State* event = Begin(s_global_data.event_container.events) + s_global_data.active_event_index;
		
		v2f title_scale = v2f{4.f, 4.f};
		Font* font = &context->theme->font;
		v2f back_button_dim = GUI_Tight_Fit_Text("<", font, title_scale);
		if(GUI_Do_Button(context, &GUI_AUTO_TOP_LEFT, &back_button_dim, "<"))
		{
			s_global_data.active_menu = Menus::all_events;
		}
		
		GUI_Push_Layout(context);
		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		GUI_Do_Text(context, AUTO, "Tapahtuma Teksti", {}, title_scale, true);
		f32 title_height = context->layout.last_element.dim.y;
		f32 title_max_x = context->layout.last_element.rect.max.x - context->anchor_base.x;
		GUI_Pop_Layout(context);
		
		
		GUI_Do_Spacing(context, v2f{0, s_post_title_y_spacing});
		{
			char* event_name_text = 
				(s_global_data.active_event_index < s_global_data.event_container.day_event_count)?
				"Tapahtuman (p\xE4iv\xE4) nimi:" : "Tapahtuman (y\xF6) nimi:";
			GUI_Do_Text(context, AUTO, event_name_text);
			
			f32 event_name_sl_field_width = 500;
			GUI_Do_SL_Input_Field(context, AUTO, &event_name_sl_field_width, &event->name);			
		}
		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		// -- title bar buttons --	
		f32 padding = context->theme->padding;
		static constexpr char* go_to_requirements_text = "Osallistujat";
		static constexpr char* go_to_consequences_text = "Seuraamukset";
		
		f32 w1 = GUI_Tight_Fit_Text(go_to_requirements_text, font).x + padding;
		f32 w2 = GUI_Tight_Fit_Text(go_to_consequences_text, font).x + padding;
		
		f32 ctrl_buttons_width = w1 + w2 + context->dynamic_slider_girth + padding * 2;
		v2f title_row_pos 
			= Get_Title_Bar_Row_Placement(context, title_max_x, padding, ctrl_buttons_width);
		
		v2f dim = v2f{w1, title_height};
		if(GUI_Do_Button(context, &title_row_pos, &dim, go_to_requirements_text))
		{
			s_global_data.active_menu = Menus::event_editor_participants;	
		}
		
		dim = v2f{w2, title_height};
		if(GUI_Do_Button(context, AUTO, &dim, go_to_consequences_text))
		{
			s_global_data.active_menu = Menus::event_editor_consequences;
		}
		
	}; // ----------------------------------------------------------------------------------------

	void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
	{		
		v2f dim = v2u::Cast<f32>(context->canvas->dim) - 50.f;
		if(dim.x >= 0 && dim.y >= 0)
		{
			Event_State* event 
				= Begin(s_global_data.event_container.events) + s_global_data.active_event_index;
			
			context->layout.anchor = GUI_Anchor::center;
			GUI_Do_ML_Input_Field(context, &GUI_AUTO_MIDDLE, &dim, &event->event_text, 0);
		}
		
	}; // ----------------------------------------------------------------------------------------

	Do_GUI_Frame_With_Banner(banner_func, menu_func);
}


static void Do_Event_Editor_Consequences_Frame()
{
	void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		v2f title_scale = v2f{4.f, 4.f};
		Font* font = &context->theme->font;
		v2f back_button_dim = GUI_Tight_Fit_Text("<", font, title_scale);
		if(GUI_Do_Button(context, &GUI_AUTO_TOP_LEFT, &back_button_dim, "<"))
		{
			s_global_data.active_menu = Menus::all_events;
		}
		
		GUI_Push_Layout(context);
		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		GUI_Do_Text(context, AUTO, "Seuraamukset", {}, title_scale, true);
		f32 title_height = context->layout.last_element.dim.y;
		f32 title_max_x = context->layout.last_element.rect.max.x - context->anchor_base.x;
		
		GUI_Pop_Layout(context);
		
		
		GUI_Do_Spacing(context, v2f{0, s_post_title_y_spacing});
		{
			if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Lis\xE4\xE4 yleis seuraamus"))
			{
				Event_State* event 
					= Begin(s_global_data.event_container.events) + s_global_data.active_event_index;
				
				Global_Mark_Consequence* gmc = Push(&event->global_mark_cons, &s_allocator);
				*gmc = {};
				gmc->mark = Create_String(&s_allocator, 4);
			}
		}
		
		context->layout.build_direction = GUI_Build_Direction::right_center;			
		
		// -- title bar buttons --	
		f32 padding = context->theme->padding;
		static constexpr char* go_to_requirements_text = "Osallistujat";
		static constexpr char* go_to_event_text_text = "Tapahtuma Teksti";

		f32 w1 = GUI_Tight_Fit_Text(go_to_requirements_text, font).x + padding;
		f32 w2 = GUI_Tight_Fit_Text(go_to_event_text_text, font).x + padding;
		
		f32 ctrl_buttons_width = w1 + w2 + context->dynamic_slider_girth + padding * 2;
		v2f title_row_pos 
			= Get_Title_Bar_Row_Placement(context, title_max_x, padding, ctrl_buttons_width);
		
		v2f dim = v2f{w1, title_height};
		if(GUI_Do_Button(context, &title_row_pos, &dim, go_to_requirements_text))
		{
			s_global_data.active_menu = Menus::event_editor_participants;	
		}
		
		dim = v2f{w2, title_height};
		if(GUI_Do_Button(context, AUTO, &dim, go_to_event_text_text))
		{
			s_global_data.active_menu = Menus::event_editor_text;
		}
		
	}; // ----------------------------------------------------------------------------------------

	void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		static f64 multiple_death_cons_error_time = 0;
		static u32 multiple_death_cons_error_idx = 0;
		
		static f64 death_con_with_additional_cons_time = 0;
		static u32 death_con_with_additional_cons_idx = 0;
		
		static constexpr f32 collumn_min_width = 300;
		
		v2f* start_pos_ptr = &GUI_AUTO_TOP_LEFT;
		
		Event_State* event = Begin(s_global_data.event_container.events) + s_global_data.active_event_index;
		
		if(event->global_mark_cons->count)
		{
			GUI_Do_Text(context, start_pos_ptr, "Yleis seuraamukset:", {}, GUI_DEFAULT_TEXT_SCALE, true);
			start_pos_ptr = 0;
			
			GUI_Push_Layout(context);
			
			f32 collumn_start = GUI_Get_Collumn_Start(context, X_AXIS);
			
			Global_Mark_Consequence* global_mark_cons = Begin(event->global_mark_cons);
			for(u32 i = 0; i < event->global_mark_cons->count; ++i)
			{
				GUI_Placement rcp = context->layout.last_element;
				
				v2f dim = v2f{36, 36};
				if(GUI_Do_Button(context, start_pos_ptr, &dim, "X"))
				{
					context->layout.last_element = rcp;
					
					Delete_Global_Mark_Consequence(event->global_mark_cons, i--);
					continue;
				}
				
				f32 left_size 
					= context->layout.last_element.pos.x - context->layout.last_element.dim.x / 2; 
				
				GUI_Push_Layout(context);
				context->layout.build_direction = GUI_Build_Direction::right_center;
				
				GUI_Do_Text(context, AUTO, "Yleis merkki", GUI_Highlight_Prev(context));
				
				f32 right_side 
					= context->layout.last_element.pos.x + context->layout.last_element.dim.x / 2; 
				
				f32 width = right_side - left_size;
				
				GUI_Pop_Layout(context);
				
				
				Global_Mark_Consequence* gmc = global_mark_cons + i;
				GUI_Do_SL_Input_Field(context, AUTO, &width, &gmc->mark);
				
				dim = context->layout.last_element.dim;
				
				GUI_Do_Text(context, AUTO, "Kesto:", GUI_Highlight_Next(context));
				
				if(u32 s = GUI_Do_Dropdown_Button(
					context, 
					AUTO, 
					&dim, 
					(char*)s_duration_names[u32(gmc->mark_duration)], 
					Array_Lenght(s_duration_names),
					(char**)s_duration_names))
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
			GUI_Do_Pannel(context, AUTO, &pannel_dim);
		}
		
		GUI_Do_Text(context, start_pos_ptr, "Osallistujat:", {}, GUI_DEFAULT_TEXT_SCALE, true);

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
				if(con->type == Event_Consequens::Type::death)
				{
					contains_death_con = true;
					break;
				}
			}
			
			if(u32 s = GUI_Do_Dropdown_Button(
				context, 
				AUTO, 
				&GUI_AUTO_FIT, 
				"Lis\xE4\xE4 seuraamus", 
				Array_Lenght(Event_Consequens::type_names),
				(char**)Event_Consequens::type_names))
			{
				multiple_death_cons_error_time = 0;
				
				if(parti->cons->count < Participent::max_consequenses)
				{
					bool allow_consequense = true;
					
					Event_Consequens::Type con_type = Event_Consequens::Type(s - 1);
					
					if(con_type == Event_Consequens::Type::death)
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
				GUI_Do_Text(context, AUTO, "Vain yksi kuolema seuraamus on sallittu!", {0}, v2f{1.f,1.f}, true);
			}
			
			if(contains_death_con && parti->cons->count >= 2)
			{
				GUI_Do_Text(context, AUTO, "Vain kuolema seuraamus toteutuu.", {0}, v2f{1.f,1.f}, true);
			}
			
			Event_Consequens* cons = Begin(parti->cons);
			for(u32 y = 0; y < parti->cons->count; ++y)
			{
				Event_Consequens* con = cons + y;
				v2f del_dim = v2f{36, 36};
				if(GUI_Do_Button(context, AUTO, &del_dim, "X"))
				{
					Make_Consequense_Hollow(con);
					u32 size = sizeof(*con);
					Remove_Element_From_Packed_Array(cons, &parti->cons->count, size, y--);
				}
				
				GUI_Push_Layout(context);
				context->layout.build_direction = GUI_Build_Direction::right_center;
				
				
				char* con_type_name = (char*)Event_Consequens::type_names[u32(con->type)];
				GUI_Do_Text(context, AUTO, con_type_name, GUI_Highlight_Prev(context));
				
				GUI_Pop_Layout(context);
				
				switch(con->type)
				{
					case Event_Consequens::Type::death:
					{
						v2f items_inherit_dim = 
							v2f{context->layout.last_element.dim.y, context->layout.last_element.dim.y};
						
						GUI_Do_Checkbox(context, AUTO , &items_inherit_dim, &con->items_are_inherited);
						
						GUI_Push_Layout(context);
						
						context->layout.build_direction = GUI_Build_Direction::right_center;
						GUI_Do_Text(context, AUTO, "Periytyv\xE4tk\xF6 esineet?", GUI_Highlight_Prev(context));
						
						GUI_Pop_Layout(context);
						
						if(con->items_are_inherited)
						{
							GUI_Do_Text(context, AUTO, "Kuka perii \\k:", GUI_Highlight_Next(context));
							
							GUI_Push_Layout(context);
							f32 width = 80;
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
					
					case Event_Consequens::Type::stat_change:
					{	
						if(u32 s = GUI_Do_Dropdown_Button(
							context, 
							AUTO, 
							&GUI_AUTO_FIT, 
							(char*)Character_Stat::stat_names[u32(con->stat)], 
							Array_Lenght(Character_Stat::stat_names),
							(char**)Character_Stat::stat_names))
						{
							con->stat = Character_Stat::Stats(s - 1);
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
						
						static constexpr char* num_options[] = {"1", "2", "3"};
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
					
					
					case Event_Consequens::Type::gains_mark:
					{
						GUI_Do_SL_Input_Field(context, AUTO, &collumn_button_width, &con->str);
						
						if(u32 s = GUI_Do_Dropdown_Button(
							context, 
							AUTO, 
							&GUI_AUTO_FIT, 
							(char*)s_mark_type_names[u32(con->mark_type)], 
							Array_Lenght(s_mark_type_names),
							(char**)s_mark_type_names))
						{
							con->mark_type = Mark_Type(s - 1);
						}
						
						v2f dim = context->layout.last_element.dim;
						
						GUI_Do_Text(context, AUTO, "Kesto:", GUI_Highlight_Next(context));
						
						if(u32 s = GUI_Do_Dropdown_Button(
							context, 
							AUTO, 
							&dim, 
							(char*)s_duration_names[u32(con->mark_duration)], 
							Array_Lenght(s_duration_names),
							(char**)s_duration_names))
						{
							con->mark_duration = i8(s - 1);
						}
						
					}break;
					
					case Event_Consequens::Type::loses_mark:
					{
						GUI_Do_SL_Input_Field(context, AUTO, &collumn_button_width, &con->str);
						
						if(u32 s = GUI_Do_Dropdown_Button(
							context, 
							AUTO, 
							&GUI_AUTO_FIT, 
							(char*)s_mark_type_names[u32(con->mark_type)], 
							Array_Lenght(s_mark_type_names),
							(char**)s_mark_type_names))
						{
							con->mark_type = Mark_Type(s - 1);
						}
						
					}break;
				}
				
				v2f spacing = v2f{context->layout.last_element.dim.x, s_theme.padding * 2};
				GUI_Do_Spacing(context, &spacing);
			}
			
			GUI_Pop_Layout(context);
			context->layout.build_direction = GUI_Build_Direction::right_top;
			
			GUI_End_Collumn(context, collumn_min_width, collumn_start, X_AXIS);
		}
		
	}; // ----------------------------------------------------------------------------------------

	Do_GUI_Frame_With_Banner(banner_func, menu_func, 160);
}


static void Do_Event_Editor_Campaigns_Menu_Frame()
{	
	void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		v2f title_scale = v2f{4.f, 4.f};
		Font* font = &context->theme->font;
		v2f back_button_dim = GUI_Tight_Fit_Text("<", font, title_scale);
		if(GUI_Do_Button(context, &GUI_AUTO_TOP_LEFT, &back_button_dim, "<"))
		{
			s_global_data.active_menu = Menus::main_menu;
		}
		
		GUI_Do_Text(context, AUTO, "Kampanja Editori", {}, title_scale, true);
		f32 title_height = context->layout.last_element.dim.y;
		f32 title_max_x = context->layout.last_element.rect.max.x - context->anchor_base.x;
		
		// -- title bar buttons --	
		static constexpr char* create_new_text = "Luo uusi kampanja";
		
		f32 padding = context->theme->padding;
		f32 w1 = GUI_Tight_Fit_Text(create_new_text, font).x + padding;	
		
		f32 ctrl_buttons_width = w1 + context->dynamic_slider_girth + padding * 2;
		
		v2f title_row_pos 
			= Get_Title_Bar_Row_Placement(context, title_max_x, padding, ctrl_buttons_width);
		
		v2f dim = v2f{w1, title_height};
		
		if(GUI_Do_Button(context, &title_row_pos, &dim, create_new_text))
		{
			char* def_name = "Uusi kampanja";
			if(!s_global_data.new_campaign_name.buffer)
			{
				Init_String(&s_global_data.new_campaign_name, &s_allocator, def_name);
			}
			else
			{
				Mem_Copy(
					s_global_data.new_campaign_name.buffer, 
					def_name, 
					Null_Terminated_Buffer_Lenght(def_name));
			
			}
			
			Set_Popup_Function(Do_Main_Menu_Name_New_Campaign_Popup);
		}
	
	}; // ----------------------------------------------------------------------------------------

	void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		GUI_Do_Text(context, &GUI_AUTO_TOP_LEFT, "(l\xF6ydetty kohteesta \"", {}, v2f{1.f, 1.f}, true);
		GUI_Push_Layout(context);
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		context->flags |= GUI_Context_Flags::one_time_skip_padding;
		GUI_Do_Text(context, AUTO, campaign_folder_wildcard_path, {}, v2f{1.f, 1.f}, true);
		
		context->flags |= GUI_Context_Flags::one_time_skip_padding;
		GUI_Do_Text(context, AUTO, "\" jossa * on kampanjan nimi.)", {}, v2f{1.f, 1.f}, true);
		
		GUI_Pop_Layout(context);
		GUI_Do_Text(context, AUTO, "Lataa tallenus:", {}, GUI_DEFAULT_TEXT_SCALE, true);
		
		Dynamic_Array<String>* on_disk_names = s_global_data.on_disk_campaign_names;
		if(on_disk_names)
		{
			for(String* save_name = Begin(on_disk_names); save_name < End(on_disk_names); ++save_name)
			{
				if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, save_name->buffer))
				{
					Events_Container ec;
					if(Load_Campaign(&ec, save_name, &s_allocator, &s_platform))
					{
						s_global_data.event_container = ec;
						s_global_data.active_menu = Menus::all_events;
					}
				}
			}
		}
		
	}; // ----------------------------------------------------------------------------------------

	Do_GUI_Frame_With_Banner(banner_func, menu_func, 100);
}


static void Do_Event_Editor_On_Exit_Popup()
{
	Dim_Entire_Screen(&s_canvas, 0.333f);
	
	static v2f panel_dim = v2f{0.f, 0.f};
	
	GUI_Begin_Context(
		&s_gui_pop_up, 
		&s_platform, 
		&s_canvas, 
		s_global_data.menu_actions, 
		&s_theme, 
		v2i{0, 0}, 
		GUI_Anchor::top);
	
	BEGIN:
	bool panel_dim_set = panel_dim != v2f{0.f, 0.f};
	
	if(panel_dim_set)
	{
		static GUI_Theme panel_theme = [](GUI_Theme* global_theme)
		{ 
			GUI_Theme result = *global_theme;
			result.background_color = s_banner_background_color;
			result.outline_color = global_theme->selected_color;
			return result;
		}(&s_theme);

		s_gui_pop_up.theme = &panel_theme;
		GUI_Do_Pannel(&s_gui_pop_up, &GUI_AUTO_MIDDLE, &panel_dim);
		s_gui_pop_up.theme = &s_theme;
	}
	
	static constexpr char* text = "Tallenetaanko kampanja?";
	GUI_Do_Text(&s_gui_pop_up, &GUI_AUTO_MIDDLE, text, {}, GUI_DEFAULT_TEXT_SCALE * 1.5f, true);		
	
	s_gui_pop_up.layout.build_direction = GUI_Build_Direction::down_center;
	
	static constexpr char* t1 = "Peruuta";
	static constexpr char* t2 = "Tallena ja jatka";
	static constexpr char* t3 = "Jatka tallentamatta";
	
	v2f button_dim = GUI_Tight_Fit_Text(t3, &s_theme.font) + s_theme.padding;
	
	if(GUI_Do_Button(&s_gui_pop_up, AUTO, &button_dim, t1))
	{
		s_popup_func = 0;
	}
	
	if(GUI_Do_Button(&s_gui_pop_up, AUTO, &button_dim, t2))
	{
		Serialize_Campaign(s_global_data.event_container, &s_platform);
		s_global_data.active_menu = Menus::campaigns_menu;
		s_popup_func = 0;
	}
	
	if(GUI_Do_Button(&s_gui_pop_up, AUTO, &button_dim, t3))
	{
		s_global_data.active_menu = Menus::campaigns_menu;
		s_popup_func = 0;
	}
	
	if(!panel_dim_set)
	{
		Rect bounds = GUI_Get_Bounds_In_Pixel_Space(&s_gui_pop_up);
		panel_dim = bounds.max - bounds.min + s_gui_pop_up.theme->padding;
		goto BEGIN;
	}
	
	GUI_End_Context(&s_gui_pop_up);
}


static void Do_Event_Editor_Quit_Popup()
{
	Dim_Entire_Screen(&s_canvas, 0.333f);
	
	static v2f panel_dim = v2f{0.f, 0.f};
	
	GUI_Begin_Context(
		&s_gui_pop_up, 
		&s_platform, 
		&s_canvas, 
		s_global_data.menu_actions, 
		&s_theme, 
		v2i{0, 0}, 
		GUI_Anchor::top);
	
	BEGIN:
	bool panel_dim_set = panel_dim != v2f{0.f, 0.f};
	
	if(panel_dim_set)
	{
		static GUI_Theme panel_theme = [](GUI_Theme* global_theme)
		{ 
			GUI_Theme result = *global_theme;
			result.background_color = s_banner_background_color;
			result.outline_color = global_theme->selected_color;
			return result;
		}(&s_theme);

		s_gui_pop_up.theme = &panel_theme;
		GUI_Do_Pannel(&s_gui_pop_up, &GUI_AUTO_MIDDLE, &panel_dim);
		s_gui_pop_up.theme = &s_theme;
	}
	
	static constexpr char* text = "Suljetaanko varmasti?";
	GUI_Do_Text(&s_gui_pop_up, &GUI_AUTO_MIDDLE, text, {}, GUI_DEFAULT_TEXT_SCALE * 1.5f, true);		
	
	s_gui_pop_up.layout.build_direction = GUI_Build_Direction::down_center;
	
	static constexpr char* t1 = "Peruuta ja jatka";
	static constexpr char* t2 = "Tallenna ja sulje";
	static constexpr char* t3 = "Sulje tallentamatta";
	
	v2f button_dim = GUI_Tight_Fit_Text(t3, &s_theme.font) + s_theme.padding;
	
	if(GUI_Do_Button(&s_gui_pop_up, AUTO, &button_dim, t1))
	{
		s_popup_func = 0;
	}
	
	if(GUI_Do_Button(&s_gui_pop_up, AUTO, &button_dim, t2))
	{
		Serialize_Campaign(s_global_data.event_container, &s_platform);
		s_platform.Set_Flag(App_Flags::is_running, false);
	}
	
	if(GUI_Do_Button(&s_gui_pop_up, AUTO, &button_dim, t3))
	{
		s_platform.Set_Flag(App_Flags::is_running, false);
	}
	
	if(!panel_dim_set)
	{
		Rect bounds = GUI_Get_Bounds_In_Pixel_Space(&s_gui_pop_up);
		panel_dim = bounds.max - bounds.min + s_gui_pop_up.theme->padding;
		goto BEGIN;
	}
	
	GUI_End_Context(&s_gui_pop_up);
}


static void Do_Main_Menu_Quit_Popup()
{
	Dim_Entire_Screen(&s_canvas, 0.333f);
	
	static v2f panel_dim = v2f{0.f, 0.f};
	
	GUI_Begin_Context(
		&s_gui_pop_up, 
		&s_platform, 
		&s_canvas, 
		s_global_data.menu_actions, 
		&s_theme, 
		v2i{0, 0}, 
		GUI_Anchor::top);
	
	BEGIN:
	bool panel_dim_set = panel_dim != v2f{0.f, 0.f};
	
	if(panel_dim_set)
	{
		static GUI_Theme panel_theme = [](GUI_Theme* global_theme)
		{ 
			GUI_Theme result = *global_theme;
			result.background_color = s_banner_background_color;
			result.outline_color = global_theme->selected_color;
			return result;
		}(&s_theme);

		s_gui_pop_up.theme = &panel_theme;
		GUI_Do_Pannel(&s_gui_pop_up, &GUI_AUTO_MIDDLE, &panel_dim);
		s_gui_pop_up.theme = &s_theme;
	}
	
	static constexpr char* text = "Suljetaanko varmasti?";
	GUI_Do_Text(&s_gui_pop_up, &GUI_AUTO_MIDDLE, text, {}, GUI_DEFAULT_TEXT_SCALE * 1.5f, true);		
	
	s_gui_pop_up.layout.build_direction = GUI_Build_Direction::down_center;
	
	static constexpr char* t1 = "Peruuta ja jatka";
	static constexpr char* t2 = "Sulje";
	
	v2f button_dim = GUI_Tight_Fit_Text(t1, &s_theme.font) + s_theme.padding;
	
	if(GUI_Do_Button(&s_gui_pop_up, AUTO, &button_dim, t1))
	{
		s_popup_func = 0;
	}
	
	if(GUI_Do_Button(&s_gui_pop_up, AUTO, &button_dim, t2))
	{
		s_platform.Set_Flag(App_Flags::is_running, false);
	}
	
	if(!panel_dim_set)
	{
		Rect bounds = GUI_Get_Bounds_In_Pixel_Space(&s_gui_pop_up);
		panel_dim = bounds.max - bounds.min + s_gui_pop_up.theme->padding;
		goto BEGIN;
	}
	
	GUI_End_Context(&s_gui_pop_up);
}


static void Do_Main_Menu_Name_New_Campaign_Popup()
{
	Dim_Entire_Screen(&s_canvas, 0.333f);
	
	static v2f panel_dim = v2f{0.f, 0.f};
	
	GUI_Begin_Context(
		&s_gui_pop_up, 
		&s_platform, 
		&s_canvas, 
		s_global_data.menu_actions, 
		&s_theme, 
		v2i{0, 0}, 
		GUI_Anchor::top);
	
	BEGIN:
	bool panel_dim_set = panel_dim != v2f{0.f, 0.f};
	
	if(panel_dim_set)
	{
		static GUI_Theme panel_theme = [](GUI_Theme* global_theme)
		{ 
			GUI_Theme result = *global_theme;
			result.background_color = s_banner_background_color;
			result.outline_color = global_theme->selected_color;
			return result;
		}(&s_theme);

		s_gui_pop_up.theme = &panel_theme;
		GUI_Do_Pannel(&s_gui_pop_up, &GUI_AUTO_MIDDLE, &panel_dim);
		s_gui_pop_up.theme = &s_theme;
	}
	
	static constexpr char* text = "Nime\xE4 uusi kampanja:";
	GUI_Do_Text(&s_gui_pop_up, &GUI_AUTO_MIDDLE, text, {}, GUI_DEFAULT_TEXT_SCALE * 1.5f, true);		
	
	bool force_create = GUI_Do_SL_Input_Field(&s_gui_pop_up, AUTO, AUTO, &s_global_data.new_campaign_name);
	
	v2f last_element_dim = s_gui_pop_up.layout.last_element.dim;
	last_element_dim.x -= s_gui_pop_up.theme->padding;
	last_element_dim.x *= 0.5f;
	
	if(GUI_Do_Button(&s_gui_pop_up, AUTO, &last_element_dim, "Luo") || force_create)
	{
		s_global_data.active_menu = Menus::all_events;
		Init_Event_Container_Takes_Name_Ownership(
			&s_global_data.event_container, 
			&s_allocator, 
			&s_global_data.new_campaign_name);
		
		Serialize_Campaign(s_global_data.event_container, &s_platform);
		
		s_popup_func = 0;
	}
	
	s_gui_pop_up.layout.build_direction = GUI_Build_Direction::right_center;
	
	if(GUI_Do_Button(&s_gui_pop_up, AUTO, &last_element_dim, "Peruuta"))
	{
		s_popup_func = 0;
	}
	
	if(!panel_dim_set)
	{
		Rect bounds = GUI_Get_Bounds_In_Pixel_Space(&s_gui_pop_up);
		panel_dim = bounds.max - bounds.min + s_gui_pop_up.theme->padding;
		goto BEGIN;
	}
	
	GUI_End_Context(&s_gui_pop_up);
}


static void Do_Main_Menu_Frame()
{
	Clear_Canvas(&s_canvas, s_background_color);
	
	v2f def_text_scale = GUI_DEFAULT_TEXT_SCALE;
	
	GUI_DEFAULT_TEXT_SCALE = v2f{3, 3};
	
	GUI_Context* context = &s_gui;
	GUI_Begin_Context(
		context, 
		&s_platform, 
		&s_canvas, 
		(Action*)s_global_data.menu_actions, 
		&s_theme,
		v2i{0, 0},
		GUI_Anchor::top);
	
	f32 title_scale = 7.f;
	GUI_Do_Text(
		context, 
		&GUI_AUTO_TOP_CENTER, 
		"N\xE4lk\xE4pelit", 
		{}, 
		v2f{title_scale, title_scale}, 
		true);
	
	
	GUI_Do_Spacing(context, AUTO);
	
	context->layout.build_direction = GUI_Build_Direction::down_center;
	
	static constexpr char* button_texts[] = 
	{
		"Jatka",
		"Uusi Peli",
		"Lataa Peli",
		"Avaa editori",
		"Asetukset",
	};

	v2f dim = GUI_Tight_Fit_Multi_Line_Text(
		&context->theme->font, 
		(char**)button_texts, 
		Array_Lenght(button_texts));
	dim += context->theme->padding;
	
	
	u32 i = 0;
	// Continue
	if(GUI_Do_Button(context, AUTO, &dim, button_texts[i++]))
	{
		
	}
	
	// New game
	if(GUI_Do_Button(context, AUTO, AUTO, button_texts[i++]))
	{
		
	}
	
	// Load game
	if(GUI_Do_Button(context, AUTO, AUTO, button_texts[i++]))
	{
		
	}
	
	// Open editor
	if(GUI_Do_Button(context, AUTO, AUTO, button_texts[i++]))
	{
		s_global_data.active_menu = Menus::campaigns_menu;
	}
	
	// Settings
	if(GUI_Do_Button(context, AUTO, AUTO, button_texts[i++]))
	{
		
	}
	
	GUI_End_Context(context);
	
	GUI_DEFAULT_TEXT_SCALE = def_text_scale;
}


static void Run_Active_Menu(bool quit_app_pop_up)
{
	Menus current_menu = s_global_data.active_menu;
	
	Action* actions = s_global_data.menu_actions;
	
	Update_Actions(&s_platform, actions, GUI_Menu_Actions::COUNT);
	
	if(quit_app_pop_up)
	{
		s_platform.Set_Flag(App_Flags::wants_to_exit, false);
		
		switch(current_menu)
		{
			case Menus::event_editor_participants:
			case Menus::event_editor_text:
			case Menus::event_editor_consequences:
			case Menus::all_events:
			{
				Set_Popup_Function(Do_Event_Editor_Quit_Popup);
			}break;
			
			case Menus::main_menu:
			case Menus::campaigns_menu:
			{	
				Set_Popup_Function(Do_Main_Menu_Quit_Popup);
			}break;
			
			default:
			{
				s_platform.Set_Flag(App_Flags::is_running, false);
			}
		}
	}
	
	if(s_popup_func)
	{
		s_gui.flags 		|= GUI_Context_Flags::hard_ignore_selection;
		s_gui_banner.flags 	|= GUI_Context_Flags::hard_ignore_selection;
		
		for(Action* action = actions; action < actions + GUI_Menu_Actions::COUNT; ++action)
			action->disabled = true;
	}
	else
	{
		Inverse_Bit_Mask(&s_gui.flags, GUI_Context_Flags::hard_ignore_selection);
		Inverse_Bit_Mask(&s_gui_banner.flags, GUI_Context_Flags::hard_ignore_selection);
	}
	
	switch(current_menu)
	{
		case Menus::none:
		{
			if(s_global_data.event_container.events && s_global_data.event_container.events->count)
			{
				s_global_data.active_menu = Menus::event_editor_participants;
			}
			else
			{				
				s_global_data.active_menu = Menus::campaigns_menu;
			}
			
		}break;
		
		case Menus::main_menu:
		{
			Do_Main_Menu_Frame();
		}break;
		
		case Menus::campaigns_menu:
		{
			Do_Event_Editor_Campaigns_Menu_Frame();
		}break;
		
		case Menus::event_editor_participants:
		{
			Do_Event_Editor_Participants_Frame();
		}break;
		
		case Menus::event_editor_text:
		{
			Do_Event_Editor_Text_Frame();
		}break;
		
		case Menus::event_editor_consequences:
		{
			Do_Event_Editor_Consequences_Frame();
		}break;
		
		case Menus::all_events:
		{
			Do_Event_Editor_All_Events_Frame();
		}break;
		
		default:
			Terminate;
	}
	
	if(s_popup_func)
	{
		for(Action* action = actions; action < actions + GUI_Menu_Actions::COUNT; ++action)
			action->disabled = false;
		
		s_popup_func();
	}
	
	if(s_global_data.active_menu != current_menu)
	{
		GUI_Reset_Context(&s_gui);
		GUI_Reset_Context(&s_gui_banner);
		
		GUI_Context::active_context_id = s_gui_banner._context_id;
		
		// -- Cleanup on exiting a menu.
		switch(current_menu)
		{
			case Menus::campaigns_menu:
			{
				if(s_global_data.on_disk_campaign_names)
				{
					Dynamic_Array<String>* on_disk_names = s_global_data.on_disk_campaign_names;
					for(String* name = Begin(on_disk_names); name < End(on_disk_names); ++name)
						name->free();
					
					s_allocator.free(s_global_data.on_disk_campaign_names);
					s_global_data.on_disk_campaign_names = 0;
				}
			}break;
		}
		
		// -- Initialization on entering a menu.
		switch(s_global_data.active_menu)
		{
			case Menus::campaigns_menu:
			{
				Delete_All_Events(&s_global_data.event_container, &s_allocator);
				
				s_global_data.on_disk_campaign_names = s_platform.Search_Directory_For_Maching_Names(
					campaign_folder_wildcard_path, 
					&s_allocator);
				
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
			}break;
			
			case Menus::main_menu:
			{
				GUI_Context::active_context_id = s_gui._context_id;
			}break;
		}
	}
}
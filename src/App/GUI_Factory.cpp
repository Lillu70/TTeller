
# pragma once

#define AUTO 0

static Color s_background_color = Make_Color(20, 20, 20);
static Color s_banner_background_color = Make_Color(40, 40, 40);

static GUI_Theme s_theme = {};
static GUI_Context s_gui;
static GUI_Context s_gui_banner;


enum class Menus : u32
{
	none = 0,
	event_editor_participents,
	event_editor_text,
	all_events,
};


struct Event_State
{
	static constexpr u32 max_participent_count = 100;
	Dynamic_Array<Participent>* participents;
	String name;
};


struct Global_Data
{
	Action menu_actions[GUI_Menu_Actions::COUNT] = {};
	
	Menus active_menu;
	
	Dynamic_Array<Event_State>* all_events = 0;
	u32 active_event_index = 0;
};
static Global_Data s_global_data = Global_Data();


static inline Event_State* Push_New_Event(char* def_name = "Uusi tapahtuma")
{
	Assert(s_global_data.all_events);
	
	Event_State* event = Push(&s_global_data.all_events, &s_allocator);
	event->participents = Create_Dynamic_Array<Participent>(&s_allocator, 4);
	Init_String(&event->name, &s_allocator, def_name);
	
	return event;
}


static void Delete_All_Participants_From_Event(Event_State* event)
{
	for(Participent* p = Begin(event->participents); p < End(event->participents); ++p)
	{
		Participation_Requirement* reg_array = Begin(p->reqs);
		
		for(Participation_Requirement* r = Begin(p->reqs); r < End((p->reqs)); ++r)
			Make_Requirement_Hollow(r);
		
		s_allocator.free(p->reqs);
	}
	
	event->participents->count = 0;
}


static inline void Init_GUI()
{
	GUI_Set_Default_Menu_Actions(&s_global_data.menu_actions[0]);

	GUI_DEFAULT_TEXT_SCALE = v2f{2, 2};
	
	s_global_data.active_menu = Menus::none;
	s_global_data.all_events = Create_Dynamic_Array<Event_State>(&s_allocator, 12);
	
	// Make Test event:
	#if 1
	
	Event_State* event = Push_New_Event("Peformance test event");
	
	u32 t = 0;
	for(u32 p = 0; p < Event_State::max_participent_count; ++p)
	{
		Participent* new_participent = Push(&event->participents, &s_allocator);
		new_participent->reqs = 
			Create_Dynamic_Array<Participation_Requirement>(&s_allocator, 3);
		
		for(u32 r = 0; r < Participent::max_requirements; ++r)
		{
			u32 mod = u32(Participation_Requirement::Type::COUNT);
			Participation_Requirement::Type new_type = (Participation_Requirement::Type)(t++ % mod);
			
			Participation_Requirement* new_req = Push(&new_participent->reqs, &s_allocator);
			*new_req = { new_type };
			
			if(Requirement_Is_Mark_Type(new_type))
			{
				static constexpr char* long_text = 
					"\xE4\xF6\xC4\xD6QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm,.:;!#1234567890";
				
				Init_String(&new_req->mark, &s_allocator, long_text);
				new_req->mark_exists = Exists_Statement::does_have;
			}
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
	u32 banner_height = 255)
{
	Assert(banner_height);
	Assert(banner_func);
	Assert(menu_func);
	
	Action* actions = s_global_data.menu_actions;
	
	v2u banner_dim = s_canvas.dim;
	banner_dim.y = Min(s_canvas.dim.y, banner_height);
	
	u32 banner_offset = banner_dim.x * banner_dim.y;
	u32* canvas_top = s_canvas.buffer + s_canvas.dim.x * s_canvas.dim.y;
	u32* banner_buffer = canvas_top - banner_offset;
	
	Canvas banner_canvas;
	Init_Canvas(&banner_canvas, banner_buffer, banner_dim);
	Clear_Canvas(&banner_canvas, s_banner_background_color);
	
	v2i banner_canvas_pos = v2i{0, i32(s_canvas.dim.y - banner_dim.y)};

	GUI_Context* context = &s_gui_banner;
	
	if(Bit_Not_Set(s_gui.flags, GUI_Context_Flags::ignore_selection))
		s_gui_banner.flags |= GUI_Context_Flags::ignore_selection;
	
	GUI_Begin_Context(context, &s_platform, &banner_canvas, actions, &s_theme, banner_canvas_pos);
	{
		banner_func(context);
	}
	GUI_End_Context(context);
	
	v2u menu_dim = v2u{s_canvas.dim.x, s_canvas.dim.y - banner_dim.y};
	if(menu_dim.y == 0)	
		return;
	
	Canvas menu_canvas;
	Init_Canvas(&menu_canvas, s_canvas.buffer, menu_dim);
	Clear_Canvas(&menu_canvas, s_background_color);
	
	context = &s_gui;
	
	if(Bit_Not_Set(s_gui_banner.flags, GUI_Context_Flags::ignore_selection))
		s_gui.flags |= GUI_Context_Flags::ignore_selection;
	
	GUI_Begin_Context(context, &s_platform, &menu_canvas, actions, &s_theme);
	{
		menu_func(context);
	}
	GUI_End_Context(context);
}


static void Do_All_Events_Frame()
{
	void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		GUI_Do_Text(context, &GUI_AUTO_TOP_LEFT, "Kaikki Tapahtumat", {}, v2f{4, 4}, true);
		
		static bool jump_into_new_event = true;
		
		if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Luo uusi"))
		{
			if(jump_into_new_event)
			{
				s_global_data.active_menu = Menus::event_editor_participents;
				s_global_data.active_event_index = s_global_data.all_events->count;			
			}
			
			Push_New_Event();
		}
		GUI_Push_Layout(context);
		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		v2f dim = v2f{context->layout.last_element_dim.y, context->layout.last_element_dim.y};
		GUI_Do_Checkbox(context, AUTO, &dim, &jump_into_new_event);
		GUI_Do_Text(context, AUTO, "Hypp\xE4\xE4 tapahtumaan.", GUI_Highlight_Prev(context));
		
		GUI_Pop_Layout(context);
	}; // ----------------------------------------------------------------------------------------
	
	void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		GUI_Do_Text(context, &GUI_AUTO_TOP_LEFT, "Tapahtuma lista:");
	
		Event_State* begin = Begin(s_global_data.all_events);
		for(u32 i = 0; i < s_global_data.all_events->count; ++i)
		{
			Event_State* e = begin + i;
			
			if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "X"))
			{
				Delete_All_Participants_From_Event(e);
				s_allocator.free(e->participents);
				e->name.free();
				Remove_Element_From_Packed_Array(begin, &s_global_data.all_events->count, sizeof(*e), i--);
				continue;
			}
			
			GUI_Push_Layout(context);
			
			context->layout.build_direction = GUI_Build_Direction::right_center;
			
			if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, e->name.buffer))
			{
				s_global_data.active_menu = Menus::event_editor_participents;
				s_global_data.active_event_index = i;
			}
			
			GUI_Pop_Layout(context);
		}
	}; // ----------------------------------------------------------------------------------------
	
	Do_GUI_Frame_With_Banner(banner_func, menu_func);
}


static void Do_Event_Editor_Requirements_Frame()
{
	void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		Event_State* event = Begin(s_global_data.all_events) + s_global_data.active_event_index;
		
		GUI_Do_Text(context, &GUI_AUTO_TOP_LEFT, "Tapahtuma Editori", {}, v2f{4,4}, true);
		f32 title_height = context->layout.last_element_dim.y;
		v2f tile_bounds_max = GUI_Get_Bounds_In_Pixel_Space(context).max;
		
		GUI_Do_Text(context, AUTO, "Tapahtuman nimi:");
		
		GUI_Do_SL_Input_Field(context, AUTO, AUTO, &event->name);
	
		if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Lis\xE4\xE4 uusi osallistuja"))
		{
			s_gui.flags |= GUI_Context_Flags::maxout_horizontal_slider;
			if(event->participents->count < event->max_participent_count)
			{
				Participent* new_participent = Push(&event->participents, &s_allocator);
				new_participent->reqs = 
					Create_Dynamic_Array<Participation_Requirement>(&s_allocator, 3);
			}
		}
		
		if(event->participents->count > 0)
		{
			context->layout.build_direction = GUI_Build_Direction::right_center;
			
			if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Poista kaikki osallistujat"))
				Delete_All_Participants_From_Event(event);
		}		
		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		f32 padding = context->layout.theme->padding;
		
		static constexpr char* back_button_text = "<";
		v2f back_button_dim = GUI_Tight_Fit_Text(
			back_button_text, 
			GUI_DEFAULT_TEXT_SCALE, 
			&context->layout.theme->font);
		
		back_button_dim.x += padding;
		back_button_dim.y = title_height;
		f32 menu_buttons_width = back_button_dim.x * 2 + padding * 2 + context->dynamic_slider_girth;
		f32 back_button_x;
		
		if(tile_bounds_max.x + padding > f32(context->canvas->dim.x) - menu_buttons_width)
			back_button_x = tile_bounds_max.x + padding;
		
		else
			back_button_x = f32(context->canvas->dim.x) - menu_buttons_width;
		
		
		f32 back_button_y = f32(context->canvas->dim.y) - padding;
		v2f back_button_pos = v2f{back_button_x, back_button_y};
		
		if(GUI_Do_Button(context, &back_button_pos, &back_button_dim, back_button_text))
			s_global_data.active_menu = Menus::all_events;
		
		if(GUI_Do_Button(context, AUTO, AUTO, ">"))
			s_global_data.active_menu = Menus::event_editor_text;
		
	}; // ----------------------------------------------------------------------------------------
	
	void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		Event_State* event = Begin(s_global_data.all_events) + s_global_data.active_event_index;
		Participent* participents_array = Begin(event->participents);
	
		static constexpr f32 collumn_width = 300;
		
		for(u32 p = 0; p < event->participents->count; ++p)
		{
			Participent* parti = participents_array + p;
			
			GUI_Push_Layout(context);
			
			v2f dim = v2f{36, 36};
			f32 padding = context->layout.theme->padding;
			v2f collumn_pos = v2f{collumn_width * p + padding, f32(context->canvas->dim.y - 1) - padding};
			if(GUI_Do_Button(context, &collumn_pos, &dim, "X"))
			{
				for(Participation_Requirement* r = Begin(parti->reqs); r < End((parti->reqs)); ++r)
					Make_Requirement_Hollow(r);
				
				s_allocator.free(parti->reqs);
				
				u32 size = sizeof(Participent);
				Remove_Element_From_Packed_Array(
					participents_array, 
					&event->participents->count, 
					size, 
					p--);
				
				GUI_Pop_Layout(context);
				continue;
			}
			
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
				"Lisaa vaatimus", 
				(u32)Participation_Requirement::Type::COUNT, 
				(char**)Participation_Requirement::type_names))
			{
				if(parti->reqs->count < parti->max_requirements)
				{
					Participation_Requirement::Type new_type = (Participation_Requirement::Type)(s - 1);
					
					Participation_Requirement* new_req = Push(&parti->reqs, &s_allocator);
					*new_req = { new_type };
					
					if(Requirement_Is_Mark_Type(new_type))
					{
						Init_String(&new_req->mark, &s_allocator, new_req->initial_mark_capacity);
						new_req->mark_exists = Exists_Statement::does_have;
					}						
				}
			}
			
			Participation_Requirement* requirement_array = Begin(parti->reqs);
			for(u32 r = 0; r < parti->reqs->count; ++r)
			{
				Participation_Requirement* req = requirement_array + r;
				
				if(GUI_Do_Button(context, AUTO, &dim, "X"))
				{
					Make_Requirement_Hollow(req);
					
					u32 size = sizeof(Participation_Requirement);
					Remove_Element_From_Packed_Array(requirement_array, &parti->reqs->count, size, r--);
				}
				
				GUI_Push_Layout(context);
				
				context->layout.build_direction = GUI_Build_Direction::right_center;
				
				char* req_type_text = (char*)Participation_Requirement::type_names[(u32)req->type];
				GUI_Do_Text(context, AUTO, req_type_text, GUI_Highlight_Prev(context));
				
				f32 right_side_x = context->layout.last_element_pos.x + context->layout.last_element_dim.x / 2;
				
				GUI_Pop_Layout(context);
				
				f32 left_side_x = context->layout.last_element_pos.x - context->layout.last_element_dim.x / 2; 
				
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
					}break;
					
					case Participation_Requirement::Type::character_stat:
					{
						// Stat
						if(u32 s = GUI_Do_Dropdown_Button(
							context, AUTO, &GUI_AUTO_FIT, 
							(char*)Character_Stat::stat_names[u32(req->stat.type)],
							Array_Lenght(Character_Stat::stat_names),
							(char**)Character_Stat::stat_names))
						{
							req->stat.type = Character_Stat::Stats(s - 1);
						}
						
						GUI_Push_Layout(context);
						
						context->layout.build_direction = GUI_Build_Direction::right_center;
						
						// Numerical_Relation
						u32 nr_idx = u32(req->stat_numerical_relation);
						if(u32 s = GUI_Do_Dropdown_Button(
							context, AUTO, &GUI_AUTO_FIT, 
							(char*)s_numerical_relation_names[nr_idx],
							Array_Lenght(s_numerical_relation_names),
							(char**)s_numerical_relation_names))
						{
							req->stat_numerical_relation = Numerical_Relation(s - 1);
						}
						
						// Target value
						static const char* value_option_names[] = {"0", "1", "2", "3"};
						if(u32 s = GUI_Do_Dropdown_Button(
							context, AUTO, AUTO, 
							(char*)value_option_names[req->stat_relation_target],
							Array_Lenght(value_option_names),
							(char**)value_option_names))
						{
							req->stat_relation_target = u16(s - 1);
						}
						
						GUI_Pop_Layout(context);
						
						if((req->stat_numerical_relation == Numerical_Relation::greater_than &&
							req->stat_relation_target == Array_Lenght(value_option_names) - 1) ||
							(req->stat_numerical_relation == Numerical_Relation::less_than &&
							req->stat_relation_target == 0))
						{
							GUI_Do_Text(context, AUTO, "Mahdoton!", {0}, v2f{1.f,1.f}, true);
						}
						else if((req->stat_numerical_relation == Numerical_Relation::less_than_equals &&
							req->stat_relation_target == Array_Lenght(value_option_names) - 1) || 
							(req->stat_numerical_relation == Numerical_Relation::greater_than_equals &&
							req->stat_relation_target == 0))
						{
							GUI_Do_Text(context, AUTO, "Aina totta!", {0}, v2f{1.f,1.f}, true);
						}
						
					}break;
				}
				
				v2f spacing = v2f{context->layout.last_element_dim.x, s_theme.padding};
				GUI_Do_Spacing(context, &spacing);
				
			}
			
			GUI_Pop_Layout(context);
		}
	}; // ----------------------------------------------------------------------------------------
	
	Do_GUI_Frame_With_Banner(banner_func, menu_func);
}


static void Do_Event_Editor_Text_Frame()
{
	void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		Event_State* event = Begin(s_global_data.all_events) + s_global_data.active_event_index;
		
		GUI_Do_Text(context, &GUI_AUTO_TOP_LEFT, "Tapahtuma Teksti", {}, v2f{4,4}, true);
		f32 title_height = context->layout.last_element_dim.y;
		v2f tile_bounds_max = GUI_Get_Bounds_In_Pixel_Space(context).max;

		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		f32 padding = context->layout.theme->padding;
		
		static constexpr char* back_button_text = "<";
		v2f back_button_dim = GUI_Tight_Fit_Text(
			back_button_text, 
			GUI_DEFAULT_TEXT_SCALE, 
			&context->layout.theme->font);
		
		back_button_dim.x += padding;
		back_button_dim.y = title_height;
		f32 menu_buttons_width = back_button_dim.x * 1 + padding * 2 + context->dynamic_slider_girth;
		f32 back_button_x;
		
		if(tile_bounds_max.x + padding > f32(context->canvas->dim.x) - menu_buttons_width)
			back_button_x = tile_bounds_max.x + padding;
		
		else
			back_button_x = f32(context->canvas->dim.x) - menu_buttons_width;
		
		
		f32 back_button_y = f32(context->canvas->dim.y) - padding;
		v2f back_button_pos = v2f{back_button_x, back_button_y};
		
		if(GUI_Do_Button(context, &back_button_pos, &back_button_dim, back_button_text))
			s_global_data.active_menu = Menus::event_editor_participents;

	}; // ----------------------------------------------------------------------------------------

	void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		static String str = {0};
		if(str.buffer == 0)
		{
			Init_String(&str, &s_allocator, "2123123123123123123123123123123");
		}
		
		v2f dim = v2u::Cast<f32>(context->canvas->dim) - 50.f;
		if(dim.x >= 0 && dim.y >= 0)
		{
			GUI_Do_ML_Input_Field(context, &GUI_AUTO_MIDDLE, &dim, &str, 0);			
		}
		
	}; // ----------------------------------------------------------------------------------------

	Do_GUI_Frame_With_Banner(banner_func, menu_func);
}


static void Run_Active_Menu()
{
	Update_Actions(&s_platform, s_global_data.menu_actions, GUI_Menu_Actions::COUNT);
	
	Menus current_menu = s_global_data.active_menu;
	
	#if 0
	
	s_gui.anchor_base = v2f{0.5f, 0.5f};
	
	#endif
	
	switch(current_menu)
	{
		case Menus::none:
		{
			#if 0
			Clear_Canvas(&s_canvas, WHITE);

			GUI_Begin_Context(&s_gui, &s_platform, &s_canvas,  (Action*)&s_global_data.menu_actions, &s_theme);
			
			s_gui.layout.anchor = GUI_Anchor::center;
			
			f32 time = f32(s_platform.Get_Time_Stamp());
			v2f scale = {2.f + Sin(time), 2.f + Cos(time)};
			
			v2f p = v2u::Cast<f32>(s_canvas.dim) / 2 + (v2f{Cos(time), Sin(time)} * 100);
			
			GUI_Do_Text(&s_gui, &p, "does 1x1 text still work?!", {}, scale);
			
			GUI_End_Context(&s_gui);
			#else
			s_global_data.active_menu = Menus::event_editor_text;
			#endif
			
		}break;
		
		case Menus::event_editor_participents:
		{
			Do_Event_Editor_Requirements_Frame();
		}break;
		
		case Menus::event_editor_text:
		{
			Do_Event_Editor_Text_Frame();
		}break;
		
		case Menus::all_events:
		{
			Do_All_Events_Frame();
		}break;
		
		default:
			Terminate;
	}
	
	
	if(s_global_data.active_menu != current_menu)
	{
		GUI_Reset_Context(&s_gui);
		GUI_Reset_Context(&s_gui_banner);
		
		
		// Menu initialization -------------------------------------------------
		switch(s_global_data.active_menu)
		{
			case Menus::event_editor_participents:
			{
				u32 f = GUI_Context_Flags::ignore_selection | GUI_Context_Flags::enable_dynamic_sliders; 
				
				s_gui.flags |= f;
				s_gui_banner.flags |= GUI_Context_Flags::enable_dynamic_sliders;				
			}break;
			
			case Menus::all_events:
			{
				u32 f = GUI_Context_Flags::ignore_selection | GUI_Context_Flags::enable_dynamic_sliders;
				
				s_gui.flags |= f;
				s_gui_banner.flags |= GUI_Context_Flags::enable_dynamic_sliders;				
			}break;
		}
	}
}
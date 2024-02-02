
# pragma once

#define AUTO 0

static GUI_Theme s_theme = {};
static GUI_Context s_gui;
static String input_field_text;


struct Participation_Requirement
{
	static inline const char* type_names[] = 
		{"Ominaisuus", "Esine Merkki", "Hahmo Merkki"};
	
	enum class Type
	{
		character_stat = 0,
		mark_item,
		mark_personal,
		COUNT
	};
	
	Type type;
};


struct Participent
{
	u32 requirement_count = 0;
	Participation_Requirement requirements[5];
};


struct New_Event_State
{
	static inline const char* p[] = {"/k1", "/k2", "/k3", "/k4"};
	u32 participent_count = 0;
	Participent participents[4];
	
	String event_name;
};
static New_Event_State s_new_event_state;


static inline void Init_GUI()
{	
	s_gui.platform = &s_platform;
	s_gui.canvas = &s_canvas;
	
	Init_String(&input_field_text, &s_mem, "Type text here.");
	
	v3<u8> c;
	f32 g;
	
	{		
		c = {165, 80, 80};
		s_theme.selected_color = Put_Color(c.r, c.g, c.b);
		
		g = 0.5f;
		s_theme.down_color = Put_Color(u8(c.r * g), u8(c.g * g), u8(c.b * g));
		
		g = 1.3f;
		s_theme.title_color = Put_Color(u8(c.r * g), u8(c.g * g), u8(c.b * g));
	}
	
	{		
		c = {80, 55, 50};
		s_theme.background_color = Put_Color(c.r, c.g, c.b);
		g = 2.f;
		s_theme.outline_color = BLACK;//Put_Color(u8(c.r * g), u8(c.g * g), u8(c.b * g));
	}
	
	s_theme.font.data_buffer = (u8*)&s_terminus_font[0];
	s_theme.font.char_width = s_terminus_font_char_width;
	s_theme.font.char_height = s_terminus_font_char_height;

	GUI_Set_Default_Menu_Actions(&s_gui);
	Init_String(&s_new_event_state.event_name, &s_mem, "Uusi tapahtumagargaerg aerg aergaergargaergrg argargaergaergaerg");	
}


static void AUTO_TEST()
{
	GUI_Begin_Context(&s_gui, GUI_Anchor::top_right, GUI_Build_Direction::down_left, &s_theme);
	
	GUI_Do_Button(&s_gui, &GUI_AUTO_TOP_CENTER, 	&GUI_AUTO_FIT, "Button");
	GUI_Do_Button(&s_gui, &GUI_AUTO_TOP_LEFT, 		&GUI_AUTO_FIT, "Button");
	GUI_Do_Button(&s_gui, &GUI_AUTO_TOP_RIGHT, 		&GUI_AUTO_FIT, "Button");
	GUI_Do_Button(&s_gui, &GUI_AUTO_MIDDLE_RIGHT, 	&GUI_AUTO_FIT, "Button");
	GUI_Do_Button(&s_gui, &GUI_AUTO_MIDDLE_LEFT, 	&GUI_AUTO_FIT, "Button");
	GUI_Do_Button(&s_gui, &GUI_AUTO_BOTTOM_CENTER, 	&GUI_AUTO_FIT, "Button");
	GUI_Do_Button(&s_gui, &GUI_AUTO_BOTTOM_RIGHT, 	&GUI_AUTO_FIT, "Button");
	GUI_Do_Button(&s_gui, &GUI_AUTO_BOTTOM_LEFT, 	&GUI_AUTO_FIT, "Button");
	GUI_Do_Button(&s_gui, &GUI_AUTO_MIDDLE, 		&GUI_AUTO_FIT, "Button");
	
	GUI_End_Context(&s_gui);
}


static void Do_Color_Editor_Widget(u32* color, v2f slider_dim, char* text)
{
	GUI_Do_Text(&s_gui, AUTO, text, GUI_Highlight_Next(&s_gui, 3));
	
	GUI_Input_Acceleration_Behavior accel = {0.1, 0.05, 10000};
	
	v3f upc = Unpack_Color(*color);
	
	GUI_Do_Text(&s_gui, AUTO, "Red:", GUI_Highlight_Next(&s_gui));
	GUI_One_Time_Skip_Padding(&s_gui);
	if(GUI_Do_Fill_Slider(&s_gui, AUTO, &slider_dim, &upc.r, 255, 1, 1.f, accel))
	{
		*color = Put_Color((u8)upc.r, (u8)upc.g, (u8)upc.b);
	}
	
	
	GUI_Do_Text(&s_gui, AUTO, "Green:", GUI_Highlight_Next(&s_gui));
	GUI_One_Time_Skip_Padding(&s_gui);
	if(GUI_Do_Fill_Slider(&s_gui, AUTO, &slider_dim, &upc.g, 255, 1, 1.f, accel))
	{
		*color = Put_Color((u8)upc.r, (u8)upc.g, (u8)upc.b);
	}
	
	
	GUI_Do_Text(&s_gui, AUTO, "Blue:", GUI_Highlight_Next(&s_gui));
	GUI_One_Time_Skip_Padding(&s_gui);
	if(GUI_Do_Fill_Slider(&s_gui, AUTO, &slider_dim, &upc.b, 255, 1, 1.f, accel))
	{
		*color = Put_Color((u8)upc.r, (u8)upc.g, (u8)upc.b);
	}
}


static void Do_Theme_Menu()
{
	GUI_Begin_Context(&s_gui, GUI_Anchor::top_left, GUI_Build_Direction::down_left, &s_theme);
	
	v2f slider_dim = v2f{100, 15};
	
	// Color Editing.
	{
		v2f spacing = {0, 15};
		GUI_Layout layout_capture;
		
		GUI_Do_Text(&s_gui, &GUI_AUTO_TOP_LEFT, "Theme Editor:", {}, v2f{1.5f, 1.5f}, true);
		
		GUI_Do_Text(&s_gui, AUTO, "Colors:", GUI_Highlight_Next(&s_gui, 18), v2f{1.0f, 1.0f});
		
		GUI_Push_Layout(&s_gui);
		
		Do_Color_Editor_Widget(&s_theme.outline_color, slider_dim, "Outline:");

		GUI_Do_Spacing(&s_gui, &spacing);
		Do_Color_Editor_Widget(&s_theme.selected_color, slider_dim, "Selected:");
		
		layout_capture = s_gui.layout;
		
		GUI_Pop_Layout(&s_gui);
		s_gui.layout.last_element_pos.x += slider_dim.x + spacing.y + s_theme.padding * 2;
		GUI_Push_Layout(&s_gui);
		
		Do_Color_Editor_Widget(&s_theme.background_color, slider_dim, "Background:");

		GUI_Do_Spacing(&s_gui, &spacing);
		Do_Color_Editor_Widget(&s_theme.down_color, slider_dim, "Down:");
		
		GUI_Pop_Layout(&s_gui);
		s_gui.layout.last_element_pos.x += slider_dim.x + spacing.y + s_theme.padding * 2;
		
		Do_Color_Editor_Widget(&s_theme.title_color, slider_dim, "Title:");
		
		GUI_Do_Spacing(&s_gui, &spacing);
		Do_Color_Editor_Widget(&s_theme.text_color, slider_dim, "Text:");		
	
		s_gui.layout = layout_capture;
		
		GUI_Do_Text(&s_gui, AUTO, "Other Theme Elements:");
		
		GUI_Do_Text(&s_gui, AUTO, "Outline thickness:");
		GUI_One_Time_Skip_Padding(&s_gui);
		f32 v = f32(s_theme.outline_thickness);
		if(GUI_Do_Fill_Slider(&s_gui, AUTO, &slider_dim, &v, 5, 0, 1))
		{
			s_theme.outline_thickness = u32(v);
		}
		
		
		v2f p = {f32(s_canvas.dim.x) - 10, 10};
		s_gui.layout.anchor = GUI_Anchor::bottom_right;
		s_gui.layout.build_direction = GUI_Build_Direction::up_right;
		
		v = s_theme.padding;
		GUI_One_Time_Skip_Padding(&s_gui);
		if(GUI_Do_Fill_Slider(&s_gui, &p, &slider_dim, &v, 15, 0, 1.f))
		{
			s_theme.padding = v;
		}
		GUI_Do_Text(&s_gui, AUTO, "Padding:");
	}
	
	// Test widgets!
	{

		GUI_Do_Text(&s_gui, &GUI_AUTO_TOP_RIGHT, "Test Widgets:", {}, v2f{2.f, 2.f}, true);
		s_gui.layout.build_direction = GUI_Build_Direction::down_right;
		
		if(GUI_Do_Button(&s_gui, AUTO, &GUI_AUTO_FIT, "This Is a Button"))
		{
			
		}
		
		static char* opt[] = {"Option 1", "Option 2", "Something else", "Option 3", "Option 4"};
		static char* t = "This Is Dropdown Button";
		switch(u32 s = GUI_Do_Dropdown_Button(&s_gui, AUTO, &GUI_AUTO_FIT, t, Array_Lenght(opt), (char**)&opt))
		{
			case 1:
			{
				
			}break;
			
			case 2:
			{
				
			}break;
		}
		
		v2f d = {20, 20};
		static bool b = false;
		GUI_Do_Checkbox(&s_gui, AUTO, &d, &b);
		
		GUI_Push_Layout(&s_gui);
		s_gui.layout.build_direction = GUI_Build_Direction::left_center;
		GUI_One_Time_Skip_Padding(&s_gui);
		GUI_Do_Text(&s_gui, AUTO, "This is a checkbox:");
		GUI_Pop_Layout(&s_gui);
		
		d = { slider_dim.x * 2, 26 };
		GUI_Do_SL_Input_Field(&s_gui, AUTO, &d, &input_field_text);
	}
	
	GUI_End_Context(&s_gui);
}


static void Random_Test_UI()
{
	GUI_Begin_Context(&s_gui, GUI_Anchor::top_left, GUI_Build_Direction::right_center, &s_theme);
	
	static u32 button_count = 0;

	{
		v2f pos = v2f{s_theme.padding, (f32)s_canvas.dim.y - s_theme.padding};
		v2f dim = v2f{170, 30};
		
		if(GUI_Do_Button(&s_gui, &pos, &dim, "New Button!"))
		{
			button_count += 1;
		}
	}
	
	{
		v2f pp = s_gui.layout.last_element_pos;
		v2f dd = s_gui.layout.last_element_dim;
		
		v2f cc = v2f{20, 20};
		GUI_Do_Text(&s_gui, AUTO, "Setting 1:", GUI_Highlight_Next(&s_gui));
		
		GUI_One_Time_Skip_Padding(&s_gui);
		
		bool is_fullscreen = Is_Flag_Set(s_platform.Get_Flags(), (u32)App_Flags::is_fullscreen);
		if(GUI_Do_Checkbox(&s_gui, AUTO, &cc, &is_fullscreen))
		{
			s_platform.Set_Flag(App_Flags::is_fullscreen, is_fullscreen);
		}
		
		
		s_gui.layout.build_direction = GUI_Build_Direction::down_left;
		
		v2f dp_dim = v2f{170, 30};
		
		static char* options[] = {"Opt 1", "Opt 2", "Opt 3", "Opt 4", "Lonqg option :)", "Opt 6"};
		
		switch(GUI_Do_Dropdown_Button(&s_gui, 0, &GUI_AUTO_FIT, "short", Array_Lenght(options), (char**)&options))
		{
			case 1:
			{
				
			}break;
			
			case 2:
			{
				
			}break;
		}
		
		f32 y = s_gui.layout.last_element_dim.y;
		
		GUI_Do_Text(&s_gui, AUTO, "Character Name:");
		GUI_One_Time_Skip_Padding(&s_gui);
		v2f ilp{s_gui.layout.last_element_dim.x, y};
		GUI_Do_SL_Input_Field(&s_gui, AUTO, &ilp, &input_field_text);
		
		GUI_Do_Text(&s_gui, AUTO, "Setting 2:", GUI_Highlight_Next(&s_gui));
		GUI_One_Time_Skip_Padding(&s_gui);
		
		static bool ccv1 = false;
		if(GUI_Do_Checkbox(&s_gui, AUTO, &cc, &ccv1))
		{
			
		}
		
		GUI_Do_Text(&s_gui, AUTO, "Setting 3:", GUI_Highlight_Next(&s_gui));
		GUI_One_Time_Skip_Padding(&s_gui);
		
		static bool ccv2 = false;
		if(GUI_Do_Checkbox(&s_gui, AUTO, &cc, &ccv2))
		{
			
		}
		
		s_gui.layout.last_element_pos = pp;
		s_gui.layout.last_element_dim = dd;
	}
	
	
	s_gui.layout.build_direction = GUI_Build_Direction::down_right;
	
	for(u32 i = 0; i < button_count; ++i)
	{
		if(GUI_Do_Button(&s_gui, AUTO, &GUI_AUTO_FIT, "IMGUI BUTTON"))
		{
			button_count -= 1;
		}
	}
	
	
	s_gui.layout.build_direction = GUI_Build_Direction::down_left;
	v2f ld = s_gui.layout.last_element_dim;
	
	
	GUI_Do_Text(&s_gui, AUTO, "Red:", GUI_Highlight_Next(&s_gui));
	
	GUI_One_Time_Skip_Padding(&s_gui);

	static f32 r = (u8)Unpack_Color(s_theme.outline_color).r;
	if(GUI_Do_Fill_Slider(&s_gui, AUTO, &ld, &r, 255, 0, 1.f, {0.1, 0.05, 10000}))
	{
		v3f c = Unpack_Color(s_theme.outline_color);
		s_theme.outline_color = Put_Color((u8)r, (u8)c.g, (u8)c.b);
	}
	
	GUI_Do_Text(&s_gui, AUTO, "Green:", GUI_Highlight_Next(&s_gui));
	
	GUI_One_Time_Skip_Padding(&s_gui);
	
	static f32 g = (u8)Unpack_Color(s_theme.outline_color).g;
	if(GUI_Do_Fill_Slider(&s_gui, AUTO, &ld, &g, 255, 1, 1.f, {0.1, 0.05, 10000}))
	{
		v3f c = Unpack_Color(s_theme.outline_color);
		s_theme.outline_color = Put_Color((u8)c.r, (u8)g, (u8)c.b);
	}
	
	GUI_Do_Text(&s_gui, AUTO, "Blue:", GUI_Highlight_Next(&s_gui));
	
	GUI_One_Time_Skip_Padding(&s_gui);
	
	static f32 b = (u8)Unpack_Color(s_theme.outline_color).b;
	if(GUI_Do_Fill_Slider(&s_gui, AUTO, &ld, &b, 255, 1, 1.f, {0.1, 0.05, 10000}))
	{
		v3f c = Unpack_Color(s_theme.outline_color);
		s_theme.outline_color = Put_Color((u8)c.r, (u8)c.g, (u8)b);
	}


	GUI_Do_Spacing(&s_gui, AUTO);	
	
	GUI_Do_Text(&s_gui, AUTO, "Random test sliders: 3 highlights!", GUI_Highlight_Next(&s_gui, 3));
	static f32 sval0 = 0.f;
	GUI_Do_Fill_Slider(&s_gui, AUTO, &ld, &sval0);
	
	static f32 sval1 = 0.2f;
	GUI_Do_Fill_Slider(&s_gui, AUTO, AUTO, &sval1, 1, 0, 0.0005f, {0.01, 0.01, 10000});
	
	static f32 sval3 = 1.f;
	GUI_Do_Fill_Slider(&s_gui, AUTO, AUTO, &sval3, 10, 0, 1.f, {1, 0.2f, 1});
	
	v2f spacing = {s_gui.layout.last_element_dim.x, 5};
	GUI_Do_Spacing(&s_gui, &spacing);
	
	GUI_Do_Button(&s_gui, AUTO, &GUI_AUTO_FIT, "Random button at the end.");
	
	GUI_End_Context(&s_gui);
}


static void Do_New_Event_Frame()
{
	New_Event_State* state = &s_new_event_state;
	
	GUI_Begin_Context(&s_gui, GUI_Anchor::top_left, GUI_Build_Direction::down_left, &s_theme);
	
	GUI_Do_Text(&s_gui, &GUI_AUTO_TOP_LEFT, "Luo uusi tapahtuma.", {}, v2f{2,2}, true);
	GUI_Do_Text(&s_gui, AUTO, "Tapahtuman nimi:");
	
	{		
		v2f d = v2f{182, 26};
		GUI_Do_SL_Input_Field(&s_gui, AUTO, &d, &state->event_name);
	}
	
	if(GUI_Do_Button(&s_gui, AUTO, &GUI_AUTO_FIT, "Lisaa uusi osallistuja"))
	{
		if(state->participent_count < Array_Lenght(state->participents))
			state->participent_count += 1;
		
	}
	
	if(state->participent_count > 0)
	{
		GUI_Push_Layout(&s_gui);
		
		s_gui.layout.build_direction = GUI_Build_Direction::right_center;
		
		if(GUI_Do_Button(&s_gui, AUTO, &GUI_AUTO_FIT, "Poista kaikki osallistujat"))
		{
			for(u32 i = 0; i < state->participent_count; ++i)
			{
				*state->participents[i].requirements = {};
				state->participents[i].requirement_count = 0;
			}
			state->participent_count = 0;
		}
		
		GUI_Pop_Layout(&s_gui);
	}
	
	for(u32 i = 0; i < state->participent_count; ++i)
	{
		Participent* p = state->participents + i;
		
		GUI_Push_Layout(&s_gui);
		
		v2f dim = v2f{18, 18};
		if(GUI_Do_Button(&s_gui, AUTO, &dim, "X"))
		{
			u32 size = sizeof(Participent);
			Remove_Element_From_Packed_Array(state->participents, &state->participent_count, size, i--);
			GUI_Pop_Layout(&s_gui);
			continue;
		}
		
		GUI_Push_Layout(&s_gui);
		
		s_gui.layout.build_direction = GUI_Build_Direction::right_center;
		GUI_Do_Text(&s_gui, AUTO, (char*)state->p[i]);
		
		GUI_Pop_Layout(&s_gui);
		
		if(u32 s = GUI_Do_Dropdown_Button(
			&s_gui, 
			AUTO, 
			&GUI_AUTO_FIT, 
			"Lisaa vaatimus", 
			(u32)Participation_Requirement::Type::COUNT, 
			(char**)Participation_Requirement::type_names))
		{
			if(p->requirement_count < Array_Lenght(p->requirements))
			{
				p->requirements[p->requirement_count] = {(Participation_Requirement::Type)(s - 1)};
				p->requirement_count += 1;				
			}
		}
		
		for(u32 j = 0; j < p->requirement_count; ++j)
		{
			Participation_Requirement* r = p->requirements + j;
			
			if(GUI_Do_Button(&s_gui, AUTO, &dim, "X"))
			{
				u32 size = sizeof(Participation_Requirement);
				Remove_Element_From_Packed_Array(p->requirements, &p->requirement_count, size, j--);
			}
			
			GUI_Push_Layout(&s_gui);
			
			s_gui.layout.build_direction = GUI_Build_Direction::right_center;
			GUI_Do_Text(&s_gui, AUTO, (char*)Participation_Requirement::type_names[(u32)r->type]);
			
			GUI_Pop_Layout(&s_gui);
		}
		
		GUI_Pop_Layout(&s_gui);
		s_gui.layout.last_element_pos.x += 150;	
		
	}
	
	GUI_End_Context(&s_gui);
}

static void SL_Input()
{
	GUI_Begin_Context(&s_gui, GUI_Anchor::top_left, GUI_Build_Direction::down_left, &s_theme);
	
	GUI_Do_Text(&s_gui, &GUI_AUTO_TOP_LEFT, "Teksti laatikko testi:", {}, v2f{2,2}, true);
	v2f d = v2f{250, 26};
	GUI_Do_SL_Input_Field(&s_gui, AUTO, &d, &s_new_event_state.event_name);
	
	GUI_End_Context(&s_gui);
}


#if 0

static void GUI_Create_New_Event_Frame();


enum class Exists_Statement : u8
{
	does_not_have = 0,
	does_have
};


enum class Numerical_Relation : u8
{
	equal_to = 0,
	greater_than,
	greater_than_equals,
	less_than,
	less_than_equals,
};


struct Character_Stat
{
	enum class Stats : u8
	{
		body = 0,
		mind
	};
	
	Stats type;
	i8 value;
};
#endif

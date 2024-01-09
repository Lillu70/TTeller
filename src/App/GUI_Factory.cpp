
# pragma once

#define AUTO 0


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


struct Participation_Requirement
{
	static inline const char* Type_Names[] = 
		{"Ominaisuus", "Esine Merkki", "Hahmo Merkki"};
	
	enum class Type
	{
		character_stat = 0,
		mark_item,
		mark_personal,
		COUNT
	};
	
	Type type;
	
	union
	{
		struct
		{
			String tag_name;
			Exists_Statement es;		
		};
		
		struct 
		{
			Character_Stat stat;
			Numerical_Relation nr;
			i8 relation_value;
		};
	};
};


struct Participant_Requirements
{
	static constexpr u32 max_participation_requirements = 5;
	
	u32 requirement_count = 0;
	Participation_Requirement requirements[max_participation_requirements];
};


struct New_Event_State
{
	u32 participent_count = 0;
	static constexpr u32 max_participent_count = 4;
	
	static inline const char* p[] = {"/k1", "/k2", "/k3", "/k4"};
	
	String event_name; // ???
	
	Participant_Requirements participation_requirements[max_participent_count];

};
static New_Event_State s_new_event_state;


static inline void Init_GUI()
{	
	v3<u8> c;
	f32 g;
	
	{		
		c = {165, 80, 80};
		s_gui.default_theme.selected_color = Put_Color(c.r, c.g, c.b);
		
		g = 0.5f;
		s_gui.default_theme.down_color = Put_Color(u8(c.r * g), u8(c.g * g), u8(c.b * g));
		
		g = 1.3f;
		s_gui.default_theme.title_color = Put_Color(u8(c.r * g), u8(c.g * g), u8(c.b * g));
	}
	
	{		
		c = {80, 55, 50};
		s_gui.default_theme.background_color = Put_Color(c.r, c.g, c.b);
		g = 2.f;
		s_gui.default_theme.outline_color = Put_Color(u8(c.r * g), u8(c.g * g), u8(c.b * g));
	}
	
	s_gui.default_theme.font.data_buffer = (u8*)&s_terminus_font[0];
	s_gui.default_theme.font.char_width = s_terminus_font_char_width;
	s_gui.default_theme.font.char_height = s_terminus_font_char_height;

	GUI_Set_Default_Menu_Actions(&s_gui);
	
	Init_String(&s_new_event_state.event_name, &s_mem, 6);
}


static void GUI_Rebuild_New_Event_Frame()
{
	u32 selection = s_gui.active_frame.selected_idx;
	GUI_Pop_Frame(&s_gui, &s_platform, &s_mem);
	GUI_Create_New_Event_Frame();

	if(selection < s_gui.active_frame.widget_count)
		s_gui.active_frame.selected_idx = selection;
	else
		s_gui.active_frame.selected_idx = s_gui.active_frame.widget_count - 1;
}


static void GUI_Add_New_Participent(u32)
{
	if(s_new_event_state.participent_count < s_new_event_state.max_participent_count)
	{
		s_new_event_state.participent_count += 1;
		GUI_Rebuild_New_Event_Frame();	
	}
}


static void GUI_Add_Participation_Requirement(u32 event_id, u32 button_idx)
{
	Assert(event_id < s_new_event_state.participent_count);
	Participant_Requirements* prs = &s_new_event_state.participation_requirements[event_id];
	
	if(prs->requirement_count < Array_Lenght(prs->requirements))
	{
		Participation_Requirement* pr = prs->requirements + prs->requirement_count;
		*pr = {};
		pr->type = static_cast<Participation_Requirement::Type>(button_idx);
		prs->requirement_count += 1;
		
		GUI_Rebuild_New_Event_Frame();
	}
}


static void GUI_Delete_Participation_Requirement(Participation_Requirement* pr)
{
	// TODO: clean up any memory allocations. This get's hairy with strings!
	*pr = {};
}


static void GUI_Delete_Participation_Requirement(Participant_Requirements* prs, u32 idx)
{
	// NOTE shifts up elements, when deleting intivitual requirements.
	GUI_Delete_Participation_Requirement(prs->requirements + idx);
	
	Remove_Element_From_Packed_Array<Participation_Requirement>
		(prs->requirements, &prs->requirement_count, idx);
		
	prs->requirements[prs->requirement_count] = {};
};


static void GUI_Delete_Participation_Requirements(Participant_Requirements* prs)
{
	for(u32 i = 0; i < prs->requirement_count; ++i)
		GUI_Delete_Participation_Requirement(prs->requirements + i);
	
	prs->requirement_count = 0;
}


static void GUI_Remove_Participent(u32 event_id)
{
	GUI_Delete_Participation_Requirements(s_new_event_state.participation_requirements + event_id);
	
	Remove_Element_From_Packed_Array<Participant_Requirements>
		(s_new_event_state.participation_requirements, &s_new_event_state.participent_count, event_id);
	
	s_new_event_state.participation_requirements[s_new_event_state.participent_count] = {};
	
	GUI_Rebuild_New_Event_Frame();
}


static void GUI_Remove_All_Participents(u32)
{
	for(u32 i = 0; i < s_new_event_state.participent_count; ++i)
		GUI_Delete_Participation_Requirements(s_new_event_state.participation_requirements + i);
	
	s_new_event_state.participent_count = 0;
	GUI_Rebuild_New_Event_Frame();
}


static void GUI_Remove_Participation_Requirement(u32 event_id)
{
	u32 idx = event_id;
	for(u32 i = 0; i < s_new_event_state.participent_count; ++i)
	{
		u32 req_count = s_new_event_state.participation_requirements[i].requirement_count;
		if(idx < req_count)
		{
			GUI_Delete_Participation_Requirement(s_new_event_state.participation_requirements + i, idx);
			GUI_Rebuild_New_Event_Frame();
			return;
		}
		
		idx -= req_count;
	}
	
	Terminate;
}


static void Pop_GUI_Frame()
{
	GUI_Pop_Frame(&s_gui, &s_platform, &s_mem);	
}


static void GUI_Create_New_Event_Frame()
{	
	GUI_Builder builder = GUI_Builder_Create(
		&s_interim_mem, 
		&s_gui,
		GUI_Link_Direction::up, 
		GUI_Build_Direction::down_left);
	
	builder.anchor = GUI_Anchor::top_left;
	
	
	{
		GUI_Text frame_title;
		frame_title.text = "Luo uusi tapahtuma.";
		frame_title.text_scale = v2f{} + 2.0f;
		frame_title.is_title = true;
		
		v2f position = v2f{builder.theme->padding, f32(s_canvas.dim.y) - builder.theme->padding};
		GUI_Builder_Create_Text(&builder, &frame_title, &position);
	}
	
	{
		GUI_Text text;
		text.text = "Tapahtuman nimi:";
		text.text_scale = v2f{1,1};
		text.is_title = false;
		GUI_Builder_Create_Text(&builder, &text, 0)->highlight_idx = builder.widget_count;
		
		GUI_Input_Field event_name_field = GUI_Input_Field();
		event_name_field.str = &s_new_event_state.event_name;
		event_name_field.text_scale = v2f{1, 1};
		event_name_field.character_limit = U16_MAX;
		event_name_field.character_check = 0;
		
		v2f dimensions = v2f{182, 20};
		
		GUI_Builder_Create_Input_Field(&builder, &event_name_field, 0, &dimensions);
	}
	
	{
		GUI_Button button;
		button.text = "Lisaa uusi osallistuja";
		button.text_scale = v2f{} + 1;
		button.on_click = GUI_Add_New_Participent;
		
		v2f dim = GUI_Builder_Fit_Button_Dimensions_To_Text(&builder, &button);
		
		GUI_Builder_Create_Button(&builder, &button, 0, &dim);
	}
	
	if(s_new_event_state.participent_count > 0)
	{
		GUI_Builder_Push_Placement(&builder);
		
		builder.build_direction = GUI_Build_Direction::right_center;
		
		{
			GUI_Button button;
			button.text = "Poista kaikki osallistujat";
			button.text_scale = v2f{} + 1;
			button.on_click = GUI_Remove_All_Participents;
			
			v2f dim = GUI_Builder_Fit_Button_Dimensions_To_Text(&builder, &button);
			
			GUI_Builder_Create_Button(&builder, &button, 0, &dim);
		}
		
		GUI_Builder_Pop_Placement(&builder);

	}
	
	
	u32 delete_requirement_id = 0;
	for(u32 i = 0; i < s_new_event_state.participent_count; ++i)
	{
		{			
			GUI_Button delete_participent;
			delete_participent.text = "X";
			delete_participent.text_scale = v2f{} + 1;
			delete_participent.on_click = GUI_Remove_Participent;
			delete_participent.event_id = i;
			
			v2f dim = v2f{18,18};
			
			u32 button_idx = GUI_Builder_Create_Button(&builder, &delete_participent, 0, &dim).idx;
			
			builder.build_direction = GUI_Build_Direction::down_left;
			
			{
				GUI_Builder_Push_Placement(&builder);
				
				builder.build_direction = GUI_Build_Direction::right_center;
				GUI_Text participent;
				participent.text = (char*)s_new_event_state.p[i];
				participent.text_scale = v2f{} + 1;
				
				GUI_Builder_Create_Text(&builder, &participent, 0)->highlight_idx = button_idx;
				
				GUI_Builder_Pop_Placement(&builder);
			}
		
			GUI_Builder_Push_Placement(&builder);
		}

		{
			GUI_List_Button add_reg;
			add_reg.text = "Lisaa vaatimus";
			add_reg.text_scale = v2f{} + 1;
			add_reg.on_select = GUI_Add_Participation_Requirement;
			add_reg.event_id = i;
			add_reg.list_element_count = (u32)Participation_Requirement::Type::COUNT;
			
			v2f dim = GUI_Builder_Fit_List_Button_Dimensions_To_Text(&builder, &add_reg);
			
			GUI_List_Button_Element* element_array = 0;
			
			GUI_Builder_Create_List_Button(&builder, &add_reg, 0, &dim, &element_array);
			
			for(u32 j = 0; j < add_reg.list_element_count; ++j)
				element_array[j].text = (char*)Participation_Requirement::Type_Names[j];
		}
		
		Participant_Requirements* prs = s_new_event_state.participation_requirements + i;
		for(u32 j = 0; j < prs->requirement_count; ++j)
		{
			GUI_Button delete_requirement;
			delete_requirement.text = "X";
			delete_requirement.text_scale = v2f{} + 1;
			delete_requirement.on_click = GUI_Remove_Participation_Requirement;
			delete_requirement.event_id = delete_requirement_id++;
			
			v2f dim = v2f{18,18};
			
			u32 button_idx = GUI_Builder_Create_Button(&builder, &delete_requirement, 0, &dim).idx;
			
			GUI_Builder_Push_Placement(&builder);
			builder.build_direction = GUI_Build_Direction::right_bottom;
			
			Participation_Requirement* pr = prs->requirements + j;
			
			GUI_Text text;
			text.text = (char*)Participation_Requirement::Type_Names[(u32)pr->type];
			text.text_scale = v2f{} + 1.f;
			
			GUI_Builder_Create_Text(&builder, &text, 0)->highlight_idx = button_idx;
			
			GUI_Builder_Pop_Placement(&builder);
		}
		
		GUI_Builder_Pop_Placement(&builder);
		builder.build_direction = GUI_Build_Direction::right_top;
		
		builder.last_element_pos.x += 131;
	}
	
	GUI_Push_Frame(&s_gui, &builder, &s_platform, &s_mem, 0, 0);
}
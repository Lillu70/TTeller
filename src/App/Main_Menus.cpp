
#pragma once



static void Do_Main_Menu_Quit_Popup()
{
	Dim_Entire_Screen(&s_canvas, 0.333f);
	
	BEGIN:
	GUI_Begin_Context(
		&s_gui_pop_up,
		&s_canvas, 
		&s_global_data.action_context, 
		&s_theme,
		v2i{0, 0}, 
		GUI_Anchor::top);
	
	bool panel_dim_set = s_global_data.popup_panel_dim != v2f{0.f, 0.f};
	
	if(panel_dim_set)
	{
		s_gui_pop_up.theme = &s_global_data.popup_panel_theme;
		GUI_Do_Panel(&s_gui_pop_up, &GUI_AUTO_MIDDLE, &s_global_data.popup_panel_dim);
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
		Close_Popup();
	}
	
	if(GUI_Do_Button(&s_gui_pop_up, AUTO, &button_dim, t2))
	{
		s_platform.Set_Flag(App_Flags::is_running, false);
	}
	
	if(!panel_dim_set)
	{
		Rect bounds = GUI_Get_Bounds_In_Pixel_Space(&s_gui_pop_up);
		s_global_data.popup_panel_dim = bounds.max - bounds.min + s_gui_pop_up.theme->padding;
		GUI_End_Context(&s_gui_pop_up);
		goto BEGIN;
	}
	
	GUI_End_Context(&s_gui_pop_up);
}


static void Do_Main_Menu_Name_New_Campaign_Popup()
{
	Dim_Entire_Screen(&s_canvas, 0.333f);
	
	BEGIN:
	GUI_Begin_Context(
		&s_gui_pop_up,
		&s_canvas, 
		&s_global_data.action_context, 
		&s_theme, 
		v2i{0, 0}, 
		GUI_Anchor::top);
	
	bool panel_dim_set = s_global_data.popup_panel_dim != v2f{0.f, 0.f};
	
	if(panel_dim_set)
	{
		s_gui_pop_up.theme = &s_global_data.popup_panel_theme;
		GUI_Do_Panel(&s_gui_pop_up, &GUI_AUTO_MIDDLE, &s_global_data.popup_panel_dim);
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
		s_global_data.active_menu = Menus::EE_all_events;
		Init_Event_Container_Takes_Name_Ownership(
			&s_editor_state.event_container, 
			&s_allocator, 
			&s_global_data.new_campaign_name);
		
		Serialize_Campaign(s_editor_state.event_container, &s_platform);
		
		Close_Popup();
	}
	
	s_gui_pop_up.layout.build_direction = GUI_Build_Direction::right_center;
	
	if(GUI_Do_Button(&s_gui_pop_up, AUTO, &last_element_dim, "Peruuta"))
	{
		Close_Popup();
	}
	
	if(!panel_dim_set)
	{
		Rect bounds = GUI_Get_Bounds_In_Pixel_Space(&s_gui_pop_up);
		s_global_data.popup_panel_dim = bounds.max - bounds.min + s_gui_pop_up.theme->padding;
		GUI_End_Context(&s_gui_pop_up);
		goto BEGIN;
	}
	
	GUI_End_Context(&s_gui_pop_up);
}


static void Do_Main_Menu_Frame()
{
	Clear_Canvas(&s_canvas, s_background_color);
	
	v2f def_text_scale = GUI_DEFAULT_TEXT_SCALE;
	
	GUI_DEFAULT_TEXT_SCALE = v2f{3, 3};
	
	GUI_Context* context = &s_gui_banner;
	GUI_Begin_Context(
		context,
		&s_canvas, 
		&s_global_data.action_context, 
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
		"Sulje"
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
		s_global_data.active_menu = Menus::select_campaign_to_play_menu;
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
	
	// Close 
	if(GUI_Do_Button(context, AUTO, AUTO, button_texts[i++]))
	{
		s_global_data.force_quit_popup = true;
	}
	
	GUI_End_Context(context);
	
	GUI_DEFAULT_TEXT_SCALE = def_text_scale;
}


static void Do_Create_Player_FI_Instruction_Popup()
{
	void(*test)(GUI_Context*) = [](GUI_Context* context)
	{
		static String instruction_text = {};
		if(!instruction_text.buffer)
		{
			Init_String(&instruction_text, &s_allocator, 
			"Here is another video inspired by the Deus Ex series!\n"
			"I hope you all enjoy it.\n"
			"I love making music like this that has more tense rhythmic elements,\n"
			"as it\'s closer to a lot of the music I compose for my full time work.\n"
			"This track feels like the music,\n"
			"that plays when you are stealthily sneaking around hostile territory,\n"
			"possibly to hack into a system or quietly take down a target.\n" 
			"I hope it helps you feel energized and productive!\n"
			"Please help my channel by sharing, liking, commenting and subscribing!\n"
			"\n\nSee you in the next one");
		}
		
		v2f title_scale = GUI_DEFAULT_TEXT_SCALE * 1.5f;
		
		static constexpr char* title_text = "Pelihahmon luonti ohjeet";
		v2f d = GUI_Tight_Fit_Text(title_text, &context->theme->font, title_scale);
		v2f b = v2f{d.y, d.y};
		
		context->layout.anchor = GUI_Anchor::center;
		
		v2f dim = v2f{d.x + context->theme->padding + b.x, 400.f};
		GUI_Do_ML_Input_Field(
			context, 
			&GUI_AUTO_MIDDLE, 
			&dim, 
			&instruction_text,
			GUI_NO_CHARACTER_LIMIT,
			GUI_DEFAULT_TEXT_SCALE,
			GUI_Character_Check_View_Only);
		
		context->layout.build_direction = GUI_Build_Direction::up_left;
		GUI_Do_Title_Text(context, AUTO, title_text, title_scale);
		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		if(GUI_Do_Button(context, AUTO, &b, "X"))
			Close_Popup();
		
		context->layout.build_direction = GUI_Build_Direction::down_center;
		
	};
	
	Do_Popup_GUI_Frame(test);
	
}


static void Do_New_Game_Players()
{
	void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		v2f title_scale = v2f{4.f, 4.f};
		Font* font = &context->theme->font;
		v2f back_button_dim = GUI_Tight_Fit_Text("<", font, title_scale);
		if(GUI_Do_Button(context, &GUI_AUTO_TOP_LEFT, &back_button_dim, "<"))
		{
			s_global_data.active_menu = Menus::main_menu;
		}
		
		GUI_Push_Layout(context);
		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		GUI_Do_Text(context, AUTO, "Kampanjan pelaajat", {}, title_scale, true);
		
		GUI_Pop_Layout(context);
		
		GUI_Do_Spacing(context, v2f{0, s_post_title_y_spacing});
		
		if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Lis\xE4\xE4 pelaaja"))
		{
			Create_Player_Name_FI(&s_game_state, &s_allocator);
			*Push(&s_game_state.player_images, &s_allocator) = {};
		}
		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		
		if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Ohje"))
		{
			Set_Popup_Function(Do_Create_Player_FI_Instruction_Popup);
		}
		
		
	}; // ----------------------------------------------------------------------------------------

	void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		Dynamic_Array<Game_Player_Name_FI>* names = s_game_state.player_names;
		Image* images = Begin(s_game_state.player_images);
		
		static constexpr f32 collumn_min_width = 300;

		v2f* pos = &GUI_AUTO_TOP_LEFT;
		f32 text_box_width = collumn_min_width - 50;
		
		context->layout.build_direction = GUI_Build_Direction::right_top;
		
		u32 i = 0;
		for(auto n = Begin(names); n < End(names); ++n, ++i)
		{
			Assert(i < s_game_state.player_images->count);			
			Image* img = images + i;
			
			GUI_Placement rcp = context->layout.last_element;
			
			if(GUI_Do_Button(context, pos, &GUI_AUTO_FIT, "X"))
			{
				Assert(names->count == s_game_state.player_images->count);
				
				Hollow_Player_Name_FI(n);
				Remove_Element_From_Packed_Array(Begin(names), &names->count, sizeof(*n), i);
				
				if(img->buffer)
					s_allocator.free(img->buffer);
				
				u32* img_count = &s_game_state.player_images->count;
				Remove_Element_From_Packed_Array(images, img_count, sizeof(*img), i);
				
				if(!i)
					pos = &GUI_AUTO_TOP_LEFT;
				
				context->layout.last_element = rcp;
				
				n -= 1;
				i -= 1;
				continue;
			}
			
			pos = 0;
			
			f32 collumn_start = GUI_Get_Collumn_Start(context, X_AXIS);
			
			GUI_Push_Layout(context);
			context->layout.build_direction = GUI_Build_Direction::right_center;
			
			GUI_Do_Text(context, AUTO, "Pelaaja");
			
			GUI_Pop_Layout(context);
			
			GUI_Push_Layout(context);
			context->layout.build_direction = GUI_Build_Direction::down_left;
			
			GUI_Do_Text(context, AUTO, "Nimi:");
			if(GUI_Do_SL_Input_Field(context, AUTO, &text_box_width, &n->full_name))
				context->selected_index += 1;
			
			GUI_Do_Text(context, AUTO, "Muoto 1:");
			if(GUI_Do_SL_Input_Field(context, AUTO, &text_box_width, &n->variant_name_1))
				context->selected_index += 1;
			
			GUI_Do_Text(context, AUTO, "Muoto 2:");
			GUI_Do_SL_Input_Field(context, AUTO, &text_box_width, &n->variant_name_2);
			
			v2f test_button_dim = v2f{text_box_width, context->layout.last_element.dim.y};
			if(GUI_Do_Button(context, AUTO, &test_button_dim, "Testaa muodot"))
			{
				
			}
			
			GUI_Do_Text(context, AUTO, "Kuva:");

			v2f picture_dim = v2f{text_box_width, text_box_width};
			if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Valitse kuva"))
			{
				char* path_buffer[260];
				char* path = (char*)path_buffer;
				
				if(s_platform.Open_Select_File_Dialog(path, Array_Lenght(path_buffer)))
				{
					u32 buffer_size = 0;
					if(s_platform.Get_File_Size((const char*)path, &buffer_size))
					{
						u8* buffer = (u8*)s_allocator.push(buffer_size);
						if(s_platform.Read_File(path, buffer, buffer_size))
						{
							i32 out_x;
							i32 out_y;
							i32 out_c;
							
							u8* img_buffer = (u8*)stbi_load_from_memory(
								buffer, 
								buffer_size, 
								&out_x, 
								&out_y, 
								&out_c, 
								4);
							
							if(img_buffer)
							{
								if(img->buffer)
									s_allocator.free(img->buffer);
								
								img->buffer = img_buffer;
								img->dim = v2i{out_x, out_y};
							}
						}
					}
				}
			}
			
			GUI_Push_Layout(context);
			
			context->layout.build_direction = GUI_Build_Direction::right_center;
			if(img->buffer && GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "X"))
			{
				s_allocator.free(img->buffer);
				img->buffer = 0;
			}
			
			GUI_Pop_Layout(context);
			
			GUI_Do_Panel(context, AUTO, &picture_dim);
			
			if(img->buffer)
			{
				Rect r = Expand_Rect(context->layout.last_element.rect, f32(s_theme.outline_thickness) * -1);
				Draw_Image(context->canvas, img, r);
			}
			
			GUI_Pop_Layout(context);
				
			GUI_End_Collumn(context, collumn_min_width, collumn_start, X_AXIS);
		}
		
	}; // ----------------------------------------------------------------------------------------

	Do_GUI_Frame_With_Banner(banner_func, menu_func, 160);
}
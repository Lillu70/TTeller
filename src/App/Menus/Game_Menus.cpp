
#pragma once

static f32 s_player_creation_collumn_min_width = 300;
static f32 s_player_creation_text_box_width = s_player_creation_collumn_min_width - 50;
static v2f s_player_picture_dim 
	= v2f{s_player_creation_text_box_width, s_player_creation_text_box_width};


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
			
			if(s_game_state.memory)
				Delete_Game(&s_game_state, &s_allocator);
			
			return;
		}
		
		GUI_Push_Layout(context);
		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		GUI_Do_Title_Text(context, AUTO, "Kampanjan pelaajat", title_scale);
		
		f32 title_height = context->layout.last_element.dim.y;
		f32 title_max_x = context->layout.last_element.rect.max.x - context->anchor_base.x;
		
		GUI_Pop_Layout(context);
		
		GUI_Do_Spacing(context, v2f{0, s_post_title_y_spacing});
		
		if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Lis\xE4\xE4 pelaaja"))
		{
			Create_Player_Name_FI(&s_game_state, &s_allocator);
			*Push(&s_game_state.player_images, &s_allocator) = {};
			
			s_gui.flags |= GUI_Context_Flags::maxout_horizontal_slider;
		}
		
		context->layout.build_direction = GUI_Build_Direction::right_center;
		
		
		if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Ohje"))
		{
			Set_Popup_Function(Do_Create_Player_FI_Instruction_Popup);
		}
		
		// -- title bar buttons --	
		f32 padding = context->theme->padding;
		
		static constexpr char* start_game_text = "Aloita peli";
		
		f32 w1 = GUI_Tight_Fit_Text(start_game_text, font).x + padding;
		
		f32 buttons_width = w1 + context->dynamic_slider_girth + padding * 2;
		v2f title_row_pos = Get_Title_Bar_Row_Placement(context, title_max_x, padding, buttons_width);
		
		v2f dim = v2f{w1, title_height};
		
		b32 player_count = s_game_state.player_names->count; 
		if(player_count && GUI_Do_Button(context, &title_row_pos, &dim, start_game_text))
		{
			s_global_data.active_menu = Menus::GM_event_display;
			Begin_Game(&s_game_state, &s_allocator);
		}
		
	}; // ----------------------------------------------------------------------------------------

	void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		if(!s_game_state.memory)
			return;
		
		Dynamic_Array<Game_Player_Name_FI>* names = s_game_state.player_names;
		Player_Image* player_images = Begin(s_game_state.player_images);
		
		v2f* pos = &GUI_AUTO_TOP_LEFT;
		
		
		context->layout.build_direction = GUI_Build_Direction::right_top;
		
		u32 i = 0;
		for(auto n = Begin(names); n < End(names); ++n, ++i)
		{
			Assert(i < s_game_state.player_images->count);			
			Player_Image* player_image = player_images + i; 
			Image* img = &(player_image)->image;
			
			GUI_Placement rcp = context->layout.last_element;
			
			if(GUI_Do_Button(context, pos, &GUI_AUTO_FIT, "X"))
			{
				Assert(names->count == s_game_state.player_images->count);
				
				Hollow_Player_Name_FI(n);
				Remove_Element_From_Packed_Array(Begin(names), &names->count, sizeof(*n), i);
				
				if(img->buffer)
					s_allocator.free(img->buffer);
				
				u32* img_count = &s_game_state.player_images->count;
				Remove_Element_From_Packed_Array(player_images, img_count, sizeof(*player_images), i);
				
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
			if(GUI_Do_SL_Input_Field(context, AUTO, &s_player_creation_text_box_width, &n->full_name))
				context->selected_index += 1;
			
			GUI_Do_Text(context, AUTO, "Muoto 1:");
			if(GUI_Do_SL_Input_Field(context, AUTO, &s_player_creation_text_box_width, &n->variant_name_1))
				context->selected_index += 1;
			
			GUI_Do_Text(context, AUTO, "Muoto 2:");
			GUI_Do_SL_Input_Field(context, AUTO, &s_player_creation_text_box_width, &n->variant_name_2);
			
			v2f test_button_dim = v2f{s_player_creation_text_box_width, context->layout.last_element.dim.y};
			if(GUI_Do_Button(context, AUTO, &test_button_dim, "Testaa muodot"))
			{
				
			}
			
			GUI_Do_Text(context, AUTO, "Kuva:");

			
			if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Valitse kuva"))
			{
				char path[260];
				
				if(s_platform.Open_Select_File_Dialog(path, Array_Lenght(path)))
				{
					Image loaded_img;
					if(Load_Image_Raw(&loaded_img, path, &s_platform))
					{
						if(img->buffer)
						{
							Assert(player_image->file_path.buffer);
							
							player_image->file_path.free();
							s_allocator.free(img->buffer);
						}
						
						#if 1
							img->dim = s_player_picture_dim.As<i32>();
							u32 sm_size = img->dim.x * img->dim.y * sizeof(Color);
							
							img->buffer = (u8*)s_allocator.push(sm_size);
						
							Resize_Image(img, &loaded_img);
							s_allocator.free(loaded_img.buffer);
						#else
						
							*img = loaded_img;
						
						#endif
						
						Convert_From_RGB_To_Color_And_Flip_Y(img);
						
						Init_String(&player_image->file_path, &s_allocator, path);
					}
				}
			}
			
			GUI_Push_Layout(context);
			
			context->layout.build_direction = GUI_Build_Direction::right_center;
			if(img->buffer && GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "X"))
			{
				Assert(player_image->file_path.buffer);
				
				s_allocator.free(img->buffer);
				player_image->file_path.free();
				img->buffer = 0;
			}
			
			GUI_Pop_Layout(context);
			
			GUI_Do_Image_Panel(context, AUTO, &s_player_picture_dim, img);
			
			GUI_Pop_Layout(context);
			
			GUI_End_Collumn(
				context, 
				s_player_creation_collumn_min_width, 
				collumn_start, 
				X_AXIS);
		}
		
	}; // ----------------------------------------------------------------------------------------

	Do_GUI_Frame_With_Banner(banner_func, menu_func, 160);
}


static void Do_Event_Display_Frame()
{
	Assert(s_game_state.player_count);
	
	static Event* active_event;
	static Event_Header* active_event_header;
	static bool skip_frame;
	
	skip_frame = false;
	
	Table* active_event_table = (s_game_state.active_event_list == Event_List::day)?
		&s_game_state.event_table_day : 
		&s_game_state.event_table_night;
	
	active_event = Begin(s_game_state.active_events) + s_game_state.display_event_idx;
	
	u32 offset = *((u32*)active_event_table->memory + active_event->event_idx);
	active_event_header = (Event_Header*)((u8*)s_game_state.events_data + offset);
	
	
	void(*banner_func)(GUI_Context* context) = [](GUI_Context* context)
	{		
		v2f title_scale = v2f{4.f, 4.f};
		char* event_name = active_event_header->event_name.buffer;
		GUI_Do_Title_Text(context, &GUI_AUTO_TOP_LEFT, event_name, title_scale);
		
		if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Lopeta peli"))
		{
			Delete_Game(&s_game_state, &s_allocator);
			s_global_data.active_menu = Menus::main_menu;
			skip_frame = true;
			return;
		}
		
		if(GUI_Do_Button(context, AUTO, &GUI_AUTO_FIT, "Seuraava tapahtuma"))
		{
			skip_frame = true;
			s_game_state.display_event_idx += 1;
			if(s_game_state.display_event_idx >= s_game_state.active_events->count)
			{
				Assign_Events_To_Participants(&s_game_state, Event_List::day, &s_allocator);
				return;
			}
		}
	}; // ----------------------------------------------------------------------------------------

	void(*menu_func)(GUI_Context* context) = [](GUI_Context* context)
	{
		if(skip_frame)
			return;
		
		Assert(active_event->participant_count);
		Player_Image* player_images = Begin(s_game_state.player_images);
		
		f32 collumn_start;
		v2f* p = &GUI_AUTO_TOP_LEFT;
		for(u32 i = 0; i < active_event->participant_count; ++i)
		{
			u32 player_idx = *(active_event->player_indices + i);
			Game_Player_Name_FI* player_name 
				= Begin(s_game_state.player_names) + player_idx;
			
			GUI_Do_Text(context, p, player_name->full_name.buffer);
			if(i == 0)
			{
				p = AUTO;
				GUI_Push_Layout(context);
				collumn_start = GUI_Get_Collumn_Start(context, X_AXIS);
			}
			
			Player_Image* player_image = player_images + player_idx;
			GUI_Do_Image_Panel(context, AUTO, &s_player_picture_dim, &player_image->image);	
		}
		
		GUI_Pop_Layout(context);
		GUI_End_Collumn(
			context, 
			s_player_creation_collumn_min_width, 
			collumn_start, 
			X_AXIS);
		
		Rect bounds = GUI_Get_Bounds_In_Pixel_Space(context);
		f32 h = bounds.max.y - bounds.min.y;
		f32 w = bounds.max.x - bounds.min.x;
		
		f32 _ = context->layout.last_element.pos.x + context->layout.last_element.dim.x - context->anchor_base.x;
		f32 text_box_width = f32(context->canvas->dim.x) - _ - f32(context->theme->padding) * 2 - 50;
		v2f text_box_dim = v2f{text_box_width, h};
		
		String event_text = {};
		event_text.buffer = active_event_header->event_text.buffer;
		event_text.lenght = Null_Terminated_Buffer_Lenght(active_event_header->event_text.buffer);
		

		context->layout.build_direction = GUI_Build_Direction::right_top;
		
		GUI_Do_ML_Input_Field(
			context, 
			AUTO, 
			&text_box_dim,
			&event_text, 
			0, 
			GUI_DEFAULT_TEXT_SCALE,
			GUI_Character_Check_View_Only);
	}; // ----------------------------------------------------------------------------------------

	Do_GUI_Frame_With_Banner(banner_func, menu_func, 200);
}
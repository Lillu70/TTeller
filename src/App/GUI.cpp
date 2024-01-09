
#pragma once


static inline void GUI_Set_Default_Menu_Actions(GUI_Handler* handler)
{
	Assert(handler);
	
	Action* actions = &handler->actions[0];
	
	*(actions + (u32)GUI_Menu_Actions::mouse) 	= Make_Action(Key_Code::MOUSE_LEFT, Button::NONE);
	*(actions + (u32)GUI_Menu_Actions::up) 		= Make_Action(Key_Code::UP, Button::NONE);
	*(actions + (u32)GUI_Menu_Actions::down) 	= Make_Action(Key_Code::DOWN, Button::NONE);
	*(actions + (u32)GUI_Menu_Actions::left) 	= Make_Action(Key_Code::LEFT, Button::NONE);
	*(actions + (u32)GUI_Menu_Actions::right) 	= Make_Action(Key_Code::RIGHT, Button::NONE);
	*(actions + (u32)GUI_Menu_Actions::enter) 	= Make_Action(Key_Code::ENTER, Button::NONE);
	*(actions + (u32)GUI_Menu_Actions::back) 	= Make_Action(Key_Code::ESC, Button::NONE);
}


static inline u32 GUI_Get_Widget_Size(void* header)
{
	Assert(header);
	
	u32 result;
	
	u16 type_number = *((u16*)(header));
	
	if(type_number < (u16)GUI_Widget_Type::COUNT)
	{
		result = sizeof(GUI_Widget_Header);
		switch(*((GUI_Widget_Type*)header))
		{
			case GUI_Widget_Type::button:
			{
				result += sizeof(GUI_Button);
			}break;
			
			case GUI_Widget_Type::slider:
			{
				result += sizeof(GUI_Slider);
			}break;
			
			case GUI_Widget_Type::checkbox:
			{
				result += sizeof(GUI_Checkbox);
			}break;
			
			case GUI_Widget_Type::key_listener:
			{
				result += sizeof(GUI_Key_Listener);
			}break;
			
			case GUI_Widget_Type::list_button:
			{
				result += sizeof(GUI_List_Button);
				GUI_List_Button* lb = (GUI_List_Button*)(((GUI_Widget_Header*)header) + 1);
				Assert(lb->list_element_count > 1);
				
				result += sizeof(GUI_List_Button_Element) * lb->list_element_count;
			}break;
			
			case GUI_Widget_Type::input_field:
			{
				result += sizeof(GUI_Input_Field);
			}break;
			
			default:
			{
				Terminate;
			}
		}
		
		return result;
	}
	else
	{
		result = sizeof(GUI_Static_Widget_Header);
		switch(*((GUI_Static_Widget_Type*)header))
		{
			case GUI_Static_Widget_Type::text:
			{
				result += sizeof(GUI_Text);
			}break;
			
			default:
			{
				Terminate;
			}
		}
		
		return result;
	}
}


static GUI_Builder GUI_Builder_Create(
	Linear_Allocator* allocator, 
	GUI_Handler* handler, 
	GUI_Link_Direction ld, 
	GUI_Build_Direction bd)
{
	GUI_Builder builder = {};
	builder.allocator = allocator;
	builder.allocator_state_copy = *allocator;

	builder.theme = &handler->default_theme;
	
	builder.link_direction = ld;
	builder.build_direction = bd;
	
	return builder;
}


static void GUI_Builder_Calc_Dim_And_Pos(
	GUI_Builder* builder,
	v2f* out_position,
	v2f* out_dimensions,
	v2f* in_position, 
	v2f* in_dimensions)
{
	Assert(builder);
	Assert(out_position);
	Assert(out_dimensions);
	
	// Auto positioning.
	// NOTE: Dimension effects positioning, so it's handeled first.
	*out_dimensions = (in_dimensions)? *in_dimensions : builder->last_element_dim;
	
	if(!in_position)
	{
		f32 padding = builder->theme->padding;
		v2f offset; 
		switch(builder->build_direction)
		{
			case GUI_Build_Direction::up_center:
			{
				offset = v2f{0, builder->last_element_dim.y / 2 + out_dimensions->y / 2 + padding};
			}break;
			
			
			case GUI_Build_Direction::down_center:
			{
				offset = v2f{0, -builder->last_element_dim.y / 2 - out_dimensions->y / 2 - padding};
			}break;
			
			
			case GUI_Build_Direction::left_center:
			{
				offset = v2f{-builder->last_element_dim.x / 2 - out_dimensions->x / 2 - padding, 0};
			}break;
			
			
			case GUI_Build_Direction::right_center:
			{
				offset = v2f{builder->last_element_dim.x / 2 + out_dimensions->x / 2 + padding, 0};
			}break;
			
			
			case GUI_Build_Direction::down_left:
			{
				v2f half_last_dim = builder->last_element_dim / 2;
				v2f half_out_dim = *out_dimensions / 2;
				offset = v2f{0, -half_last_dim.y - half_out_dim.y - padding};
			
				f32 left_shift = half_out_dim.x - half_last_dim.x;
				offset.x += left_shift;
			}break;
			
			
			case GUI_Build_Direction::down_right:
			{
				v2f half_last_dim = builder->last_element_dim / 2;
				v2f half_out_dim = *out_dimensions / 2;
				offset = v2f{0, -half_last_dim.y - half_out_dim.y - padding};
			
				f32 right_shift = half_last_dim.x - half_out_dim.x;
				offset.x += right_shift;
			}break;
			
			
			case GUI_Build_Direction::up_left:
			{
				v2f half_last_dim = builder->last_element_dim / 2;
				v2f half_out_dim = *out_dimensions / 2;
				offset = v2f{0, half_last_dim.y + half_out_dim.y + padding};
			
				f32 left_shift = half_out_dim.x - half_last_dim.x;
				offset.x += left_shift;
			}break;
			
			
			case GUI_Build_Direction::up_right:
			{
				v2f half_last_dim = builder->last_element_dim / 2;
				v2f half_out_dim = *out_dimensions / 2;
				offset = v2f{0, half_last_dim.y + half_out_dim.y + padding};
			
				f32 right_shift = half_last_dim.x - half_out_dim.x;
				offset.x += right_shift;
			}break;
			
			
			case GUI_Build_Direction::left_top:
			{
				v2f half_last_dim = builder->last_element_dim / 2;
				v2f half_out_dim = *out_dimensions / 2;
				offset = v2f{-half_last_dim.x - half_out_dim.x - padding, 0};
			
				f32 up_shift = half_last_dim.y - half_out_dim.y;
				offset.y += up_shift;
			}break;
			
			
			case GUI_Build_Direction::left_bottom:
			{
				v2f half_last_dim = builder->last_element_dim / 2;
				v2f half_out_dim = *out_dimensions / 2;
				offset = v2f{-half_last_dim.x - half_out_dim.x - padding, 0};
			
				f32 up_shift = half_out_dim.y - half_last_dim.y;
				offset.y += up_shift;
			}break;
			
			
			case GUI_Build_Direction::right_top:
			{
				v2f half_last_dim = builder->last_element_dim / 2;
				v2f half_out_dim = *out_dimensions / 2;
				offset = v2f{half_last_dim.x + half_out_dim.x + padding, 0};
			
				f32 up_shift = half_last_dim.y - half_out_dim.y;
				offset.y += up_shift;
			}break;
			
			
			case GUI_Build_Direction::right_bottom:
			{
				v2f half_last_dim = builder->last_element_dim / 2;
				v2f half_out_dim = *out_dimensions / 2;
				offset = v2f{half_last_dim.x + half_out_dim.x + padding, 0};
			
				f32 up_shift = half_out_dim.y - half_last_dim.y;
				offset.y += up_shift;
			}break;
		}
		
		*out_position = builder->last_element_pos + offset;
	}
	else
	{
		v2f half_dim = *out_dimensions / 2;
		*out_position = *in_position;
		
		switch(builder->anchor)
		{
			case GUI_Anchor::top:
			{
				out_position->y -= half_dim.y;
			}break;
			
			case GUI_Anchor::bottom:
			{
				out_position->y += half_dim.y;
			}break;
			
			case GUI_Anchor::left:
			{
				out_position->x += half_dim.x;
			}break;
			
			case GUI_Anchor::right:
			{
				out_position->x -= half_dim.x;
			}break;
			
			case GUI_Anchor::top_left:
			{
				out_position->y -= half_dim.y;
				out_position->x += half_dim.x;
			}break;
			
			case GUI_Anchor::top_right:
			{
				out_position->y -= half_dim.y;
				out_position->x -= half_dim.x;
			}break;
			
			case GUI_Anchor::bottom_left:
			{
				out_position->y += half_dim.y;
				out_position->x += half_dim.x;
			}break;
			
			case GUI_Anchor::bottom_right:
			{
				out_position->y += half_dim.y;
				out_position->x -= half_dim.x;
			}break;
		}
	}
	
	builder->last_element_pos = *out_position;
	builder->last_element_dim = *out_dimensions;
}


static inline GUI_Static_Widget_Header* GUI_Builder_Create_Static_Header(
	GUI_Builder* builder,
	GUI_Static_Widget_Header* header, 
	GUI_Static_Widget_Type type,
	u32 widget_size,
	v2f* position,
	v2f* dimensions)
{
	*header = GUI_Static_Widget_Header();
	header->type = type;
	header->theme = builder->theme;
	
	v2f dummy = {};
	
	GUI_Builder_Calc_Dim_And_Pos(
		builder, 
		&header->position, 
		&dummy, 
		position, 
		dimensions);
	
	
	builder->static_widget_memory_count += widget_size;
	builder->static_widget_count += 1;
	
	return header;
}


static inline GUI_Add_Result GUI_Builder_Create_Header(
	GUI_Builder* builder,
	void* widget,
	u32 widget_size,
	GUI_Widget_Type type,
	v2f* position,
	v2f* dimensions,
	u32 extra_memory = 0)
{
	u32 total_size = widget_size + sizeof(GUI_Widget_Header) + extra_memory;
	GUI_Widget_Header* header = (GUI_Widget_Header*)(builder->allocator->push(total_size));
	Mem_Copy(header + 1, widget, widget_size);
	
	header->type = type;
	header->theme = builder->theme;
	
	GUI_Builder_Calc_Dim_And_Pos(
		builder, 
		&header->position, 
		&header->dimensions, 
		position, 
		dimensions);
	
	
	// Builder tracking thigs:
	GUI_Add_Result result = {header, builder->widget_count};
	if(builder->widget_count == 0)
		builder->first_element = result;
	
	builder->widget_memory_count += GUI_Get_Widget_Size(header);
	builder->last_element = result;
	builder->widget_count += 1;
	
	return result;
}


static GUI_Add_Result GUI_Builder_Create_Button(
	GUI_Builder* builder, 
	GUI_Button* widget, 
	v2f* position,
	v2f* dimensions)
{
	Assert(builder);
	Assert(widget);
	
	GUI_Widget_Type t = GUI_Widget_Type::button;
	return GUI_Builder_Create_Header(builder, widget, sizeof(*widget), t, position, dimensions);
}


static GUI_Add_Result GUI_Builder_Create_List_Button(
	GUI_Builder* builder, 
	GUI_List_Button* widget, 
	v2f* position,
	v2f* dimensions,
	GUI_List_Button_Element** out_element_array)
{
	Assert(builder);
	Assert(widget);
	Assert(widget->list_element_count > 1);
	
	u32 extra_memory = sizeof(GUI_List_Button_Element) * widget->list_element_count;
	u32 widget_size = sizeof(*widget);
	
	GUI_Widget_Type t = GUI_Widget_Type::list_button;
	GUI_Add_Result result = GUI_Builder_Create_Header(
		builder, 
		widget, 
		widget_size, 
		t, 
		position, 
		dimensions, 
		extra_memory);
	
	*out_element_array = (GUI_List_Button_Element*)((u8*)result.widget + widget_size + sizeof(GUI_Widget_Header));
	
	return result;
}


static GUI_Add_Result GUI_Builder_Create_Slider(
	GUI_Builder* builder, 
	GUI_Slider* widget,
	v2f* position, 
	v2f* dimensions)
{
	Assert(builder);
	Assert(widget);
	
	GUI_Widget_Type t = GUI_Widget_Type::slider;
	return GUI_Builder_Create_Header(builder, widget, sizeof(*widget), t, position, dimensions);
}


static GUI_Add_Result GUI_Builder_Create_Checkbox(
	GUI_Builder* builder, 
	GUI_Checkbox* widget,
	v2f* position, 
	v2f* dimensions)
{
	Assert(builder);
	Assert(widget);
	
	GUI_Widget_Type t = GUI_Widget_Type::checkbox;
	return GUI_Builder_Create_Header(builder, widget, sizeof(*widget), t, position, dimensions);
}


static GUI_Add_Result GUI_Builder_Create_Key_Listener(GUI_Builder* builder, GUI_Key_Listener* widget)
{
	Assert(builder);
	Assert(widget);
	
	GUI_Widget_Type t = GUI_Widget_Type::key_listener;
	return GUI_Builder_Create_Header(builder, widget, sizeof(*widget), t, 0, 0);
}


static GUI_Add_Result GUI_Builder_Create_Input_Field(
	GUI_Builder* builder, 
	GUI_Input_Field* widget,
	v2f* position,
	v2f* dimensions)
{
	Assert(builder);
	Assert(widget);
	
	GUI_Widget_Type t = GUI_Widget_Type::input_field;
	
	return GUI_Builder_Create_Header(builder, widget, sizeof(*widget), t, position, dimensions);
}


static GUI_Static_Widget_Header* GUI_Builder_Create_Text(
	GUI_Builder* builder, 
	GUI_Text* static_widget, 
	v2f* position)
{
	Assert(builder);
	Assert(static_widget);
	Assert(static_widget->text);

	// Text doens't have dimensions in the regular sense, but it's bounding box is calculated here.
	// It's used for auto position of elements and mouse selection of text elements.
	
	Font* font = &builder->theme->font;
	
	u32 text_lenght = Null_Terminated_Buffer_Lenght(static_widget->text);
	v2f dim = 
		{
			(f32)(text_lenght * static_widget->text_scale.x * font->char_width), 
			(f32)(static_widget->text_scale.y * font->char_height)
		};
	
	u32 static_widget_size = sizeof(*static_widget) + sizeof(GUI_Static_Widget_Header);
	GUI_Static_Widget_Header* header = (GUI_Static_Widget_Header*)(builder->allocator->push(static_widget_size));
	*((GUI_Text*)(header + 1)) = *static_widget;
	
	GUI_Static_Widget_Type t = GUI_Static_Widget_Type::text;
	return GUI_Builder_Create_Static_Header(builder, header, t, static_widget_size, position, &dim);
}


static inline void GUI_Builder_Push_Placement(GUI_Builder* builder)
{
	Assert(builder);
	Assert(builder->placement_stack_count < Array_Lenght(builder->placement_stack));
	
	builder->placement_stack[builder->placement_stack_count].pos = builder->last_element_pos;
	builder->placement_stack[builder->placement_stack_count].dim = builder->last_element_dim;
	builder->placement_stack[builder->placement_stack_count].bl = builder->build_direction;
	
	builder->placement_stack_count += 1;
}


static inline void GUI_Builder_Pop_Placement(GUI_Builder* builder)
{
	Assert(builder);
	Assert(builder->placement_stack_count > 0);
	
	builder->placement_stack_count -= 1;
	
	builder->last_element_pos = builder->placement_stack[builder->placement_stack_count].pos;
	builder->last_element_dim = builder->placement_stack[builder->placement_stack_count].dim;
	builder->build_direction = builder->placement_stack[builder->placement_stack_count].bl;
}


static inline v2f GUI_Builder_Fit_Button_Dimensions_To_Text(GUI_Builder* builder, GUI_Button* button)
{
	GUI_Theme* theme = builder->theme;
	
	u32 text_width = Null_Terminated_Buffer_Lenght(button->text);
	
	u32 w = theme->font.char_width;
	u32 h = theme->font.char_height;
	
	v2f result = v2f{
			button->text_scale.x * w * (f32)text_width + theme->outline_thickness * 3,
			button->text_scale.y * h + theme->outline_thickness * 3};
			
	return result;
}


static inline v2f GUI_Builder_Fit_List_Button_Dimensions_To_Text(GUI_Builder* builder, GUI_List_Button* button)
{
	GUI_Theme* theme = builder->theme;
	
	u32 text_width = Null_Terminated_Buffer_Lenght(button->text);
	
	u32 w = theme->font.char_width;
	u32 h = theme->font.char_height;
	
	v2f result = v2f{
			button->text_scale.x * w * (f32)text_width + theme->outline_thickness * 3,
			button->text_scale.y * h + theme->outline_thickness * 3};
			
	return result;
}


static inline Rect GUI_Get_List_Button_Open_Rect(GUI_Widget_Header* header)
{
	f32 half_dim_y = header->dimensions.y / 2;
	
	f32 l0 = header->position.y + half_dim_y;
	
	GUI_List_Button* lb = (GUI_List_Button*)(header + 1);
	
	f32 l1 = header->position.y + (half_dim_y * lb->list_element_count);
	
	v2f center = v2f{header->position.x, header->position.y - (l1 - l0)};
	
	v2f new_dim = v2f{header->dimensions.x, header->dimensions.y * lb->list_element_count};
	
	Rect result = Create_Rect_Center(center, new_dim);
	return result;
}


static inline v2f GUI_Calc_Centered_Text_Position(char* text, v2f scale, v2f position, Font* font)
{
	f32 magic_offset = 1.0f;
	
	u32 text_lenght = Null_Terminated_Buffer_Lenght(text);
	v2f result = position;
	result.x -= text_lenght * scale.x * font->char_width / 2;
	result.y -= (scale.y * magic_offset) * font->char_height / 2;
	
	return result;
}


static inline Action* GUI_Get_Action(Action* actions, GUI_Menu_Actions action)
{
	return (actions + (u32)action);
}


static inline GUI_Widget_Header* GUI_Get_Selected_Header(GUI_Frame* frame)
{
	if(frame->selected_idx < frame->widget_count)
	{
		return frame->random_access_table[frame->selected_idx];
	}
	
	return 0;
}


static inline bool GUI_Is_Menu_Up(GUI_Handler* handler)
{
	return handler->active_frame.memory;
}


static void GUI_Push_Frame(
	GUI_Handler* handler,
	GUI_Builder* builder,
	Platform_Calltable* platform, 
	General_Allocator* mem_arena,
	void(*on_back_action)(),
	void(*on_frame_close)())
{
	Assert(handler);
	Assert(builder);
	Assert(platform);
	Assert(mem_arena);
	
	GUI_Frame* frame = &handler->active_frame;
	
	GUI_Frame* prev_frame = 0;
	
	// There is a menu, it needs to be pushed forwards in the chain.
	if(frame->widget_count + frame->static_widget_count > 0)
	{
		prev_frame = mem_arena->push<GUI_Frame>();
		*prev_frame = *frame;
	}
	
	// there is no previous menu, show cursor.
	else
	{
		platform->Set_Flag(App_Flags::cursor_is_visible, true);
	}
	
	*frame = GUI_Frame();
	frame->prev_frame = prev_frame;


	u32 frame_size = sizeof(*frame->random_access_table) * builder->widget_count;
	frame_size += builder->widget_memory_count + builder->static_widget_memory_count;
	
	// Setup the memory section pointers	
	frame->memory = mem_arena->push(frame_size);
	
	frame->random_access_table = (GUI_Widget_Header**)frame->memory;
	frame->widgets = (GUI_Widget_Header*)(frame->random_access_table + builder->widget_count);
	frame->static_widgets = 
		(GUI_Static_Widget_Header*)((u8*)frame->widgets) + builder->widget_memory_count;
	
	
	frame->ld = builder->link_direction;
	
	// Copy over the data.
	u8* read_cursor = (u8*)builder->allocator_state_copy.next_free;
	u8* widget_write_cursor = (u8*)frame->widgets;
	u8* static_widget_write_cursor = (u8*)frame->static_widgets;
	u32 widget_size;
	for(u16 i = 0; i < builder->widget_count + builder->static_widget_count; ++i, read_cursor += widget_size)
	{
		widget_size	= GUI_Get_Widget_Size(read_cursor);
		void* source = read_cursor;
		void* dest = 0; 
		
		// Widget
		if(*((u16*)read_cursor) < (u16)GUI_Widget_Type::COUNT)
		{
			dest = widget_write_cursor;
			widget_write_cursor += widget_size;
			
			GUI_Widget_Header* target = (GUI_Widget_Header*)dest;
			*(frame->random_access_table + frame->widget_count) = target;
			frame->widget_count += 1;
		}
		
		// Static Widget
		else
		{
			dest = static_widget_write_cursor;
			static_widget_write_cursor += widget_size;
			frame->static_widget_count += 1;
		}
		
		Mem_Copy(dest, source, widget_size);
	}
	
	Assert(frame->widget_count == builder->widget_count);
	Assert(frame->static_widget_count == builder->static_widget_count);
	
	*builder->allocator = builder->allocator_state_copy;
	*builder = {};
	
	frame->selected_idx = builder->selected_idx;
}

   
static void GUI_Pop_Frame(GUI_Handler* handler, Platform_Calltable* platform, General_Allocator* mem_arena)
{
	Assert(handler);
	GUI_Frame* active_frame = &handler->active_frame;
	
	Assert(active_frame->memory);
	
	if(active_frame->on_frame_close)
		active_frame->on_frame_close();
	
	mem_arena->free(active_frame->memory);
	
	GUI_Frame* prev_frame = active_frame->prev_frame;
	if(prev_frame)
	{    
		*active_frame = *prev_frame;
		mem_arena->free(prev_frame);
	}
	
	// This is the only frame.
	else
	{
		*active_frame = GUI_Frame();
		platform->Set_Flag(App_Flags::cursor_is_visible, false);
	}
}


static inline bool GUI_Default_Mouse_On_Element_Test(GUI_Handler* handler, 
	Rect element_rect, 
	v2i cursor_position, 
	u32 idx)
{
	if(Is_Point_Inside_Rect(v2i::Cast<f32>(cursor_position), element_rect))
	{
		handler->selection_state.cursor_on_selection = true;
		if(idx != handler->active_frame.selected_idx)
		{
			handler->selection_state.element = {};
			handler->active_frame.selected_idx = idx;
		}
		
		return true;
	}
	
	return false;
}


static inline bool GUI_Default_Mouse_On_Element_Test(GUI_Handler* handler, 
	GUI_Widget_Header* header, 
	v2i cursor_position, 
	u32 idx)
{
	Rect element_rect = Create_Rect_Center(header->position, header->dimensions);
	return GUI_Default_Mouse_On_Element_Test(handler, element_rect, cursor_position, idx);
}


static void GUI_Handle_Mouse_Input(GUI_Handler* handler, Platform_Calltable* platform)
{
	v2i cursor_position = platform->Get_Cursor_Position();
	
	GUI_Frame* active_frame = &handler->active_frame;
	
	bool mouse_is_pressed = GUI_Get_Action(handler->actions, GUI_Menu_Actions::mouse)->Is_Pressed();
	bool mouse_down = GUI_Get_Action(handler->actions, GUI_Menu_Actions::mouse)->Is_Down();
	
	if(handler->last_cursor_position != cursor_position || mouse_is_pressed)
	{
		handler->last_cursor_position = cursor_position;
		
		handler->selection_state.cursor_on_selection = false;
		
		// Loop starts from the selected idx and iterates though to the end, 
		// then from idx 0 up to the selected idx.
		
		u32 i, j;
		for(i = 0, j = active_frame->selected_idx; i < active_frame->widget_count; ++i, ++j)
		{
			if(j >= active_frame->widget_count)
				j = 0;
			
			GUI_Widget_Header* header = active_frame->random_access_table[j];
			
			switch(header->type)
			{
				case GUI_Widget_Type::list_button:
				{
					if(handler->selection_state.element.list_button.is_open)
					{
						Rect element_rect = GUI_Get_List_Button_Open_Rect(header);
					
						GUI_List_Button* list_button = (GUI_List_Button*)(header + 1);
						
						if(GUI_Default_Mouse_On_Element_Test(handler, element_rect, cursor_position, j))
						{
							u32 selection = handler->selection_state.element.list_button.selected_idx;
							f32 level = header->position.y - header->dimensions.y / 2;
							for(u32 y = 0; y < list_button->list_element_count; ++y)
							{
								if(cursor_position.y > level)
								{
									handler->selection_state.element.list_button.selected_idx = y;
									break;
								}
								level -= header->dimensions.y;
							}
							
							if(selection != handler->selection_state.element.list_button.selected_idx)
								handler->selection_state.element.list_button.is_pressed = false;
						
							return;
						}
					}
					else
					{
						if(GUI_Default_Mouse_On_Element_Test(handler, header, cursor_position, j))
							return;
					}
				}break;
				
				case GUI_Widget_Type::slider:
				{
					if(GUI_Default_Mouse_On_Element_Test(handler, header, cursor_position, j))
					{
						bool slider_is_held = handler->selection_state.element.slider.is_clicked && mouse_down;
					
						if(mouse_is_pressed || slider_is_held)
						{
							handler->selection_state.element.slider.is_clicked = true;
							
							GUI_Slider* slider = (GUI_Slider*)(header + 1);
							
							GUI_Theme* theme = header->theme;
							
							v2f outline = 
								v2f{f32(theme->outline_thickness) * 2, f32(theme->outline_thickness) * 2};
							
							v2f internal_dim = header->dimensions - outline;
							v2f half_dim = internal_dim * 0.5f;
							f32 rel_cursor_x = 
								(v2i::Cast<f32>(cursor_position) - (header->position - half_dim)).x;
							
							f32 fill_percent = rel_cursor_x / internal_dim.x;
							f32 fill = (slider->max - slider->min) * fill_percent + slider->min;
							i32 steps = Round_To_Signed_Int32(fill / slider->step);
							
							slider->value = steps * slider->step;
							
							if(slider->on_value_change)
								slider->on_value_change(slider);
						}
						
						return;
					}
				}break;
				
				default:
				{
					if(GUI_Default_Mouse_On_Element_Test(handler, header, cursor_position, j))
						return;
				}
			}
		}
	}	
	
	bool mouse_released = GUI_Get_Action(handler->actions, GUI_Menu_Actions::mouse)->Is_Released();
	
	if(!handler->selection_state.cursor_on_selection && mouse_released)
	{
		handler->selection_state = {};
	}
}


static void GUI_Handle_Input(GUI_Handler* handler, Platform_Calltable* platform)
{
	Assert(handler);
	Assert(platform);
	
	Action* actions = &handler->actions[0];
	GUI_Frame* active_frame = &handler->active_frame;
	
	if((!GUI_Is_Menu_Up(handler)) || (!Is_Flag_Set(platform->Get_Flags(), (u32)App_Flags::is_focused)))
		return;
	
	
	Update_Actions(platform, actions, (u32)GUI_Menu_Actions::COUNT);
	
	GUI_Handle_Mouse_Input(handler, platform);
	
	bool mouse_is_pressed = GUI_Get_Action(actions, GUI_Menu_Actions::mouse)->Is_Pressed();
	bool mouse_is_released = GUI_Get_Action(actions, GUI_Menu_Actions::mouse)->Is_Released();
	
	bool select_action_pressed = 
		GUI_Get_Action(actions, GUI_Menu_Actions::enter)->Is_Pressed() ||
			(handler->selection_state.cursor_on_selection && mouse_is_pressed);
	
	
	bool select_action_released = 
		GUI_Get_Action(actions, GUI_Menu_Actions::enter)->Is_Released() ||
			(handler->selection_state.cursor_on_selection && mouse_is_released);
	
	// Set to false in the widget control in order to disable/highjack the up down controlls.
	bool allow_kc_selection_change = true;
	bool allow_back_action = true;
	
	GUI_Widget_Header* selected_header = GUI_Get_Selected_Header(active_frame);
	if(selected_header)
	{
		void* selected_widget = (selected_header + 1);
		
		// Keyboard/Controller selection interaction.
		switch(selected_header->type)
		{
			case GUI_Widget_Type::button:
			{
				GUI_Button* button = (GUI_Button*)(selected_widget);
				
				if(select_action_released)
				{
					if(handler->selection_state.element.button.is_pressed)
					{
						handler->selection_state.element.button.is_pressed = false;
						if(button->on_click)
						{
							button->on_click(button->event_id);
							return;
						}
					}
				}
				else if(select_action_pressed)
					handler->selection_state.element.button.is_pressed = true;
				
				
			}break;
			
			
			case GUI_Widget_Type::list_button:
			{
				GUI_List_Button* list_button = (GUI_List_Button*)(selected_widget);
				
				if(handler->selection_state.element.list_button.is_open)
				{
					allow_back_action = false;
					allow_kc_selection_change = false;
					
					if(GUI_Get_Action(actions, GUI_Menu_Actions::back)->Is_Pressed())
						handler->selection_state.element.list_button = {};
					
					u32* idx = &handler->selection_state.element.list_button.selected_idx;
					if(GUI_Get_Action(actions, GUI_Menu_Actions::up)->Is_Pressed())
					{
						*idx -= 1;
						if(*idx >= list_button->list_element_count)
							*idx = list_button->list_element_count - 1;
						
						handler->selection_state.element.list_button.is_pressed = false;
					}
					
					if(GUI_Get_Action(actions, GUI_Menu_Actions::down)->Is_Pressed())
					{
						*idx += 1;
						if(*idx >= list_button->list_element_count)
							*idx = 0;
						
						handler->selection_state.element.list_button.is_pressed = false;
					}
					
					if(select_action_released)
					{
						if(handler->selection_state.element.button.is_pressed)
						{
							u32 selection = handler->selection_state.element.list_button.selected_idx;
							handler->selection_state.element = {};
							if(list_button->on_select)
							{
								list_button->on_select(list_button->event_id, selection);
								return;
							}
						}
					}
					else if(select_action_pressed)
						handler->selection_state.element.list_button.is_pressed = true;
				}
				else
				{
					if(select_action_released)
					{
						if(handler->selection_state.element.button.is_pressed)
						{
							handler->selection_state.element.list_button.is_pressed = false;
							handler->selection_state.element.list_button.is_open = true;
						}
					}
					else if(select_action_pressed)
						handler->selection_state.element.list_button.is_pressed = true;
					
				}
				
			}break;
			
			
			case GUI_Widget_Type::slider:
			{
				GUI_Slider* slider = (GUI_Slider*)(selected_widget);
				
				Action* left = GUI_Get_Action(actions, GUI_Menu_Actions::left);
				Action* right = GUI_Get_Action(actions, GUI_Menu_Actions::right); 
				
				if(left->Is_Pressed() || right->Is_Pressed() || (!left->Is_Down() && !right->Is_Down()))
				{
					handler->selection_state.element.slider.accum_action_time = 0;
					handler->selection_state.element.slider.full_action_time = 0;
					handler->selection_state.element.slider.last_time_stamp = 0;
				}
				
				f64 time = platform->Get_Time_Stamp();
				f32 time_dif = f32(time - handler->selection_state.element.slider.last_time_stamp);
				
				if(handler->selection_state.element.slider.last_time_stamp > 0 && time_dif > 0)
				{
					handler->selection_state.element.slider.accum_action_time += time_dif;
					handler->selection_state.element.slider.full_action_time += time_dif;
					
					f32 t = slider->action_tick_rate;
					
					if(slider->action_speed_up_time > 0)
					{
						f32 full_action_time = handler->selection_state.element.slider.full_action_time;
						f32 t1 = full_action_time / slider->action_speed_up_time;
						f32 t2 = 1.f + t1;
						t /= t2;
					}
					
					while(handler->selection_state.element.slider.accum_action_time >= t)
					{
						handler->selection_state.element.slider.accum_action_time -= t;
						
						if(GUI_Get_Action(actions, GUI_Menu_Actions::left)->Is_Down())
						{
							slider->value -= slider->step;
							if(slider->value < slider->min)
								slider->value = slider->min;
							
							if(slider->on_value_change)
								slider->on_value_change(slider);
						}
						
						if(GUI_Get_Action(actions, GUI_Menu_Actions::right)->Is_Down())
						{	
							slider->value += slider->step;
							if(slider->value > slider->max)
								slider->value = slider->max;
							
							if(slider->on_value_change)
								slider->on_value_change(slider);
						}				
					}				
				}
				
				handler->selection_state.element.slider.last_time_stamp = time;
			}break;
			
			
			case GUI_Widget_Type::checkbox:
			{
				GUI_Checkbox* checkbox = (GUI_Checkbox*)(selected_widget);
				
				if(select_action_released)
				{
					if(handler->selection_state.element.checkbox.is_pressed)
					{
						handler->selection_state.element.checkbox.is_pressed = false;
						checkbox->is_checked = !checkbox->is_checked;
						if(checkbox->on_value_change)
							checkbox->on_value_change(checkbox);					
					}
				}
				else if(select_action_pressed)
					handler->selection_state.element.checkbox.is_pressed = true;
				
			}break;
			
			
			case GUI_Widget_Type::key_listener:
			{
				GUI_Key_Listener* listener = (GUI_Key_Listener*)(selected_widget);
				
				if(listener->on_trigger)
				{
					Key_Code key = Key_Code::NONE;
					Button button = Button::NONE;
					
					for(u32 i = 0; i < (u32)Key_Code::COUNT; ++i)
					{
						Key_Code k = (Key_Code)i;
						
						if(platform->Get_Keyboard_Key_Down(k))
						{
							key = k;
							break;
						}
					}
					
					Controller_State controller = platform->Get_Controller_State(0);
					
					for(u32 i = 0; i < (u32)Button::BUTTON_COUNT; ++i)
					{
						Button b = (Button)i;
						
						if(controller.Get_Button_Down(b))
						{
							button = b;
							break;
						}
					}
					
					if(key != Key_Code::NONE || button != Button::NONE)
						listener->on_trigger(key, button, listener->action_array,listener->action_idx);
				}
				
			}break;
			
			
			case GUI_Widget_Type::input_field:
			{
				GUI_Input_Field* input_field = (GUI_Input_Field*)(selected_widget);
				
				GUI_Selection_State::Element::Input_Field* state = &handler->selection_state.element.input_field;
				
				if(state->is_active)
				{
					// CONSIDER: What about, rebound keys?
					// 	Should being in text input mode disable all regular keybinds?
					//	Probably...
					
					f64 time = platform->Get_Time_Stamp();
					state->draw_cursor = !(time > state->flicker_start_time) || (u64)time % 2;
					
					Char_Array typing_info = platform->Get_Typing_Information();
					if(typing_info.count > 0)
						state->flicker_start_time = time + GUI_Selection_State::Element::Input_Field::flicker_delay;
					
					for(u32 i = 0; i < typing_info.count; ++i)
					{
						char c = typing_info.buffer[i];
						
						if(c == '\b' || c == 127)
						{
							if(input_field->str->lenght > 0)
								input_field->str->pop_last();
						}
						else if(c >= 32 && c <= 127 && 
							(input_field->character_limit == U16_MAX 
							|| input_field->str->lenght < input_field->character_limit) &&
							(!input_field->character_check || 
							(input_field->character_check && input_field->character_check(input_field, c))))
						{
							*input_field->str += c;
						}
					}
				}
				else
				{
					if(select_action_released)
					{
						state->is_active = true;
						state->is_pressed = false;
					}
					else if(select_action_pressed)
						state->is_pressed = true;
				}
				
			}break;
		}
		
	}
	
	
	if(allow_back_action && 
		active_frame->on_back_action && 
		GUI_Get_Action(actions, GUI_Menu_Actions::back)->Is_Pressed())
	{
		active_frame->on_back_action();
		return;	
	}
	
	
	// Keyboard/Controller element selection.
	if(allow_kc_selection_change)
	{
		switch(active_frame->ld)
		{
			case GUI_Link_Direction::up:
			{
				if(GUI_Get_Action(actions, GUI_Menu_Actions::up)->Is_Pressed())
				{
					handler->selection_state = {};
					active_frame->selected_idx -= 1;
					
					// selected_idx is an unsigned value and will underflow,
					// so checking for < 0 does not work.
					if(active_frame->selected_idx >= active_frame->widget_count)
						active_frame->selected_idx = active_frame->widget_count - 1;					
				}	
				
				if(GUI_Get_Action(actions, GUI_Menu_Actions::down)->Is_Pressed())
				{
					handler->selection_state = {};
					active_frame->selected_idx += 1;
					
					if(active_frame->selected_idx >= active_frame->widget_count)
						active_frame->selected_idx = 0;
				}
				
			}break;
			
			
			case GUI_Link_Direction::down:
			{
				if(GUI_Get_Action(actions, GUI_Menu_Actions::up)->Is_Pressed())
				{
					
					handler->selection_state = {};
					active_frame->selected_idx += 1;
					
					if(active_frame->selected_idx >= active_frame->widget_count)
						active_frame->selected_idx = 0;
				}					
				
				if(GUI_Get_Action(actions, GUI_Menu_Actions::down)->Is_Pressed())
				{
					handler->selection_state = {};
					active_frame->selected_idx -= 1;
					
					// selected_idx is an unsigned value and will underflow,
					// so checking for < 0 does not work.
					if(active_frame->selected_idx >= active_frame->widget_count)
						active_frame->selected_idx = active_frame->widget_count - 1;
					
				}
				
			}break;
		}
		
	}
}


static void GUI_Draw_Widgets(GUI_Handler* handler, Canvas* canvas, Linear_Allocator* interim_mem)
{
	GUI_Frame* active_frame = &handler->active_frame;
	
	Linear_Allocator allocator_state_copy = *interim_mem;
	u32 defered_render_count = 0;
	GUI_Widget_Header** defered_render_list = (GUI_Widget_Header**)allocator_state_copy.next_free;
	
	{		
		u32 static_widget_size = 0;
		u8* header_cursor = (u8*)active_frame->static_widgets;
		for(u32 i = 0; i < active_frame->static_widget_count; ++i, header_cursor += static_widget_size)
		{
			static_widget_size = GUI_Get_Widget_Size(header_cursor);
			GUI_Static_Widget_Header* header = (GUI_Static_Widget_Header*)header_cursor;
			bool is_highlighted = 
				active_frame->selected_idx != U16_MAX && header->highlight_idx == active_frame->selected_idx;
			
			GUI_Theme* theme = header->theme;
			Font* font = &theme->font;
			
			switch(header->type)
			{
				case GUI_Static_Widget_Type::text:
				{
					GUI_Text* text = (GUI_Text*)(header + 1);
					
					v2f text_p = GUI_Calc_Centered_Text_Position(text->text, 
							text->text_scale, 
							header->position, 
							font);
					
					u32 color;
					if(text->is_title)
						color = theme->title_color;
					else
						color = is_highlighted? theme->selected_color : theme->outline_color;
					
					Draw_Text(canvas, (u8*)text->text, text_p, color, font, text->text_scale);
				}break;
			}
		}
	}
	
	for(u32 i = 0; i < active_frame->widget_count; ++i)
	{
		GUI_Widget_Header* header = active_frame->random_access_table[i];
		void* widget = header + 1;
		
		bool is_selected = i == active_frame->selected_idx;
		
		GUI_Theme* theme = header->theme;
		Font* font = &theme->font;
		
		switch(header->type)
		{
			case GUI_Widget_Type::button:
			{
				GUI_Button* button = (GUI_Button*)widget;
			
				u32 outline_color = is_selected? theme->selected_color : theme->outline_color;
				
				if(is_selected && handler->selection_state.element.button.is_pressed) 
					outline_color = theme->down_color;
				
				Rect button_rect = Create_Rect_Center(header->position, header->dimensions);
				
				Draw_Filled_Rect_With_Outline(
					canvas, 
					button_rect, 
					theme->background_color,
					theme->outline_thickness, 
					outline_color);
				
				if(button->text)
				{
					v2f text_p = GUI_Calc_Centered_Text_Position(button->text, 
						button->text_scale, 
						header->position, 
						font);
			
					Draw_Text(canvas, (u8*)button->text, text_p, outline_color, font, button->text_scale);
				}
			}break;
			
			
			case GUI_Widget_Type::list_button:
			{
				GUI_List_Button* list_button = (GUI_List_Button*)widget;
				if(!handler->selection_state.element.list_button.is_open || !is_selected)
				{
					u32 outline_color = is_selected? theme->selected_color : theme->outline_color;
					if(is_selected && handler->selection_state.element.list_button.is_pressed) 
						outline_color = theme->down_color;
					
					Rect button_rect = Create_Rect_Center(header->position, header->dimensions);
					
					Draw_Filled_Rect_With_Outline(
						canvas, 
						button_rect, 
						theme->background_color,
						theme->outline_thickness, 
						outline_color);
						
					if(list_button->text)
					{
						v2f text_p = GUI_Calc_Centered_Text_Position(list_button->text, 
							list_button->text_scale, 
							header->position, 
							font);
						
						Draw_Text(canvas, (u8*)list_button->text, text_p, outline_color, font, list_button->text_scale);
					}
				}
				else
				{
					*interim_mem->push<GUI_Widget_Header*>() = header;
					defered_render_count += 1;
				}
			
			}break;
			
			
			case GUI_Widget_Type::slider:
			{
				GUI_Slider* slider = (GUI_Slider*)widget;
				
				Rect slider_rect = Create_Rect_Center(header->position, header->dimensions);
				
				f32 fill = (slider->value - slider->min) / (slider->max - slider->min);
				u32 bar_color = is_selected? theme->selected_color : theme->outline_color;
				
				Draw_Percentile_Bar(
					canvas,
					slider_rect,
					theme->background_color,
					theme->outline_thickness,
					bar_color,
					bar_color,
					fill);
				
			}break;
			
			case GUI_Widget_Type::checkbox:
			{
				GUI_Checkbox* checkbox = (GUI_Checkbox*)widget;
				u32 outline_color = is_selected? theme->selected_color : theme->outline_color;
				
				if(is_selected && handler->selection_state.element.checkbox.is_pressed) 
					outline_color = theme->down_color;
				
				Rect rect = Create_Rect_Center(header->position, header->dimensions);
				
				Draw_Filled_Rect_With_Outline(
					canvas, 
					rect, 
					theme->background_color,
					theme->outline_thickness, 
					outline_color);
			
				if(checkbox->is_checked)
				{
					// half here on purpose
					rect = Create_Rect_Center(header->position, header->dimensions * 0.5f);
					Draw_Filled_Rect(canvas, rect, outline_color);
				}
				
			}break;
			
			case GUI_Widget_Type::input_field:
			{
				GUI_Input_Field* input_field = (GUI_Input_Field*)widget;
				
				GUI_Selection_State::Element::Input_Field* state = &handler->selection_state.element.input_field;
				
				u32 outline_color = is_selected? theme->selected_color : theme->outline_color;
				
				if(is_selected && state->is_pressed) 
					outline_color = theme->down_color;
				
				Rect rect = Create_Rect_Center(header->position, header->dimensions);
				
				Draw_Filled_Rect_With_Outline(
					canvas, 
					rect, 
					theme->background_color,
					theme->outline_thickness, 
					outline_color);
					
				
				if(input_field->str->buffer)
				{
					v2f text_p = header->position - header->dimensions / 2 + v2f{2,2};
					
					bool draw_cursor = is_selected && state->draw_cursor && state->is_active;
					
					if(draw_cursor)
					{
						v2f cursor_p = text_p;
						cursor_p.x += (input_field->str->lenght * (f32)font->char_width + 2);
						
						Draw_Vertical_Line(
							canvas, 
							cursor_p, 
							font->char_height * input_field->text_scale.y, 
							theme->outline_color);						
					}
					
					Draw_Text(
						canvas, 
						(u8*)input_field->str->buffer, 
						text_p, 
						outline_color, 
						font, 
						input_field->text_scale);
				}
				
			}break;
		}
	}
	
	for(u32 i = 0; i < defered_render_count; ++i)
	{
		GUI_Widget_Header* header = defered_render_list[i];
		void* widget = header + 1;
		
		bool is_selected = i == active_frame->selected_idx;
		GUI_Theme* theme = header->theme;
		Font* font = &theme->font;
		
		switch(header->type)
		{
			case GUI_Widget_Type::list_button:
			{
				GUI_List_Button* list_button = (GUI_List_Button*)widget;
				
				Rect lb_rect = GUI_Get_List_Button_Open_Rect(header);
					
				Draw_Filled_Rect_With_Outline(
					canvas, 
					lb_rect, 
					theme->background_color,
					theme->outline_thickness, 
					theme->selected_color);
				
				v2f draw_p = header->position;
				GUI_List_Button_Element* list_elements = (GUI_List_Button_Element*)(list_button + 1);
				f32 shrink_v = 0;
				v2f dim = v2f{header->dimensions.x - shrink_v, header->dimensions.y - shrink_v};
				
				for(u32 j = 0; j < list_button->list_element_count; ++j)
				{
					v2f text_p = GUI_Calc_Centered_Text_Position(list_elements->text, 
						list_button->text_scale, 
						draw_p, 
						font);
					
					u32 text_color;
					if(handler->selection_state.element.list_button.selected_idx == j)
					{
						u32 bg_color = theme->outline_color;
						text_color = theme->background_color;
						
						if(is_selected && handler->selection_state.element.list_button.is_pressed)
						{
							text_color = theme->down_color;
							bg_color = theme->background_color;
						}								
						
						Rect bg_rect = Create_Rect_Center(draw_p, dim);						
						Draw_Filled_Rect_With_Outline(canvas, bg_rect, bg_color, theme->outline_thickness, theme->selected_color);
					}
					else
						text_color = theme->outline_color;
					
					Draw_Text(canvas, (u8*)list_elements->text, text_p, text_color, font, list_button->text_scale);
					
					draw_p.y -= header->dimensions.y;
					++list_elements;
					
				}
			}break;
			
			default:
				Terminate;
		}
	}
	
	*interim_mem = allocator_state_copy;
}
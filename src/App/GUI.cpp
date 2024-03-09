
#pragma once

static inline bool GUI_Is_Context_Ready(GUI_Context* context)
{
	bool result = context->flags & GUI_Context_Flags::context_ready;
	return result;
}


static inline void GUI_Reset_Context(GUI_Context* context)
{
	// Make sure context is not reset between begin and end.
	Assert(!GUI_Is_Context_Ready(context));
	
	f32 dynamic_slider_girth = context->dynamic_slider_girth;
	*context = GUI_Context();
	
	context->dynamic_slider_girth = dynamic_slider_girth;
}


static Rect GUI_Get_Bounds_In_Pixel_Space(GUI_Context* context)
{
	Rect result = context->bounds_rel_anchor_base;
	result.min -= context->anchor_base;
	result.max -= context->anchor_base;
	
	return result;
}


static inline void GUI_Push_Layout(GUI_Context* context)
{
	bool can_push = context->layout_stack_count < Array_Lenght(context->layout_stack);
	Assert(can_push);
	
	if(can_push)
	{
		context->layout_stack[context->layout_stack_count] = context->layout;
		context->layout_stack_count += 1;
	}
}


static inline void GUI_Pop_Layout(GUI_Context* context)
{
	bool can_pop = context->layout_stack_count > 0;
	Assert(can_pop);
	
	if(can_pop)
	{
		context->layout_stack_count -= 1;
		context->layout = context->layout_stack[context->layout_stack_count];
	}
}


static inline void GUI_Set_Default_Menu_Actions(Action* actions)
{
	Assert(actions);
	
	*(actions + GUI_Menu_Actions::mouse) 	= Make_Action(Key_Code::MOUSE_LEFT, Button::NONE);
	*(actions + GUI_Menu_Actions::up) 		= Make_Action(Key_Code::UP, Button::DPAD_UP);
	*(actions + GUI_Menu_Actions::down) 	= Make_Action(Key_Code::DOWN, Button::DPAD_DOWN);
	*(actions + GUI_Menu_Actions::left) 	= Make_Action(Key_Code::LEFT, Button::DPAD_LEFT);
	*(actions + GUI_Menu_Actions::right) 	= Make_Action(Key_Code::RIGHT, Button::DPAD_RIGHT);
	*(actions + GUI_Menu_Actions::enter) 	= Make_Action(Key_Code::ENTER, Button::BUT_A);
	*(actions + GUI_Menu_Actions::back) 	= Make_Action(Key_Code::ESC, Button::BUT_X);
}


static inline void GUI_One_Time_Skip_Padding(GUI_Context* context)
{
	context->layout.one_time_skip_padding = true;
}


static inline  bool GUI_Accelerated_Tick(
	GUI_Input_Acceleration_Behavior* p,
	f64 time,
	f64* input_start_time,
	f64* next_input_time)
{
	Assert(p->input_delay_time > 0);
	Assert(p->input_speed_up_time >= 0);
	
	if(time < *next_input_time)
		return false;
	
	if(*input_start_time == 0)
	{
		*input_start_time = time;
		*next_input_time = time + p->input_delay_time;
		
		return true;
	}
	else 
	{
		f64 s = p->input_speed_up_time > 0? p->input_speed_up_time : 1.0;
		
		f64 hold_duration = *next_input_time - *input_start_time;
		f64 t = Min(1.0 + (hold_duration / s), Max(1.0, p->max_speed_up_factor));
		f64 t2 = *next_input_time + (p->input_delay_time / t);
		
		*next_input_time = t2;
	
		return true;
	}
}


static inline GUI_Highlight GUI_Highlight_Nothing()
{
	return {};
}


static inline GUI_Highlight GUI_Highlight_Next(GUI_Context* context, i32 num_elements)
{
	return { context->widget_count, num_elements };
}


static inline GUI_Highlight GUI_Highlight_Next(GUI_Context* context)
{
	return { context->widget_count, 1 };
}


static inline GUI_Highlight GUI_Highlight_Prev(GUI_Context* context)
{
	return { context->widget_count - 1, 1 };
}


static inline v2f GUI_Tight_Fit_Text(char* text, v2f text_scale, Font* font)
{
	v2f result = v2f
	{
		text_scale.x * f32(font->char_width) * f32(Null_Terminated_Buffer_Lenght(text)), 
		text_scale.y * f32(font->char_height)
	};
	
	return result;
}


static inline void GUI_Update_Bounds(GUI_Context* context, Rect rect)
{
	if(rect.min.x < context->bounds_rel_anchor_base.min.x)
		context->bounds_rel_anchor_base.min.x = rect.min.x;
	
	if(rect.min.y < context->bounds_rel_anchor_base.min.y)
		context->bounds_rel_anchor_base.min.y = rect.min.y;
		
	if(rect.max.x > context->bounds_rel_anchor_base.max.x)
		context->bounds_rel_anchor_base.max.x = rect.max.x;
	
	if(rect.max.y > context->bounds_rel_anchor_base.max.y)
		context->bounds_rel_anchor_base.max.y = rect.max.y;
}


static inline GUI_Placement GUI_Get_Placement(GUI_Context* context, v2f* dim, v2f* pos)
{
	GUI_Layout* layout = &context->layout;
	
	GUI_Placement result;
	
	v2f last_element_dim = layout->last_element_dim;
	
	// Auto positioning.
	// NOTE: Dimension effects positioning, so it's handeled first.
	result.dim = (dim)? *dim : last_element_dim;
	
	f32 padding;
	if(layout->one_time_skip_padding)
	{
		padding = 0;
		layout->one_time_skip_padding = false;
	}
	else
	{
		padding = layout->theme->padding;
	}
	
	
	if(!pos)
	{
		Assert(last_element_dim.x * last_element_dim.y > 0);
		
		v2f offset;
		switch(layout->build_direction)
		{
			case GUI_Build_Direction::up_center:
			{
				offset = v2f{0, last_element_dim.y / 2 + result.dim.y / 2 + padding};
			}break;
			
			
			case GUI_Build_Direction::down_center:
			{
				offset = v2f{0, -last_element_dim.y / 2 - result.dim.y / 2 - padding};
			}break;
			
			
			case GUI_Build_Direction::left_center:
			{
				offset = v2f{-last_element_dim.x / 2 - result.dim.x / 2 - padding, 0};
			}break;
			
			
			case GUI_Build_Direction::right_center:
			{
				offset = v2f{last_element_dim.x / 2 + result.dim.x / 2 + padding, 0};
			}break;
			
			
			case GUI_Build_Direction::down_left:
			{
				v2f half_last_dim = last_element_dim / 2;
				v2f half_out_dim = result.dim / 2;
				offset = v2f{0, -half_last_dim.y - half_out_dim.y - padding};
			
				f32 left_shift = half_out_dim.x - half_last_dim.x;
				offset.x += left_shift;
			}break;
			
			
			case GUI_Build_Direction::down_right:
			{
				v2f half_last_dim = last_element_dim / 2;
				v2f half_out_dim = result.dim / 2;
				offset = v2f{0, -half_last_dim.y - half_out_dim.y - padding};
			
				f32 right_shift = half_last_dim.x - half_out_dim.x;
				offset.x += right_shift;
			}break;
			
			
			case GUI_Build_Direction::up_left:
			{
				v2f half_last_dim = last_element_dim / 2;
				v2f half_out_dim = result.dim / 2;
				offset = v2f{0, half_last_dim.y + half_out_dim.y + padding};
			
				f32 left_shift = half_out_dim.x - half_last_dim.x;
				offset.x += left_shift;
			}break;
			
			
			case GUI_Build_Direction::up_right:
			{
				v2f half_last_dim = last_element_dim / 2;
				v2f half_out_dim = result.dim / 2;
				offset = v2f{0, half_last_dim.y + half_out_dim.y + padding};
			
				f32 right_shift = half_last_dim.x - half_out_dim.x;
				offset.x += right_shift;
			}break;
			
			
			case GUI_Build_Direction::left_top:
			{
				v2f half_last_dim = last_element_dim / 2;
				v2f half_out_dim = result.dim / 2;
				offset = v2f{-half_last_dim.x - half_out_dim.x - padding, 0};
			
				f32 up_shift = half_last_dim.y - half_out_dim.y;
				offset.y += up_shift;
			}break;
			
			
			case GUI_Build_Direction::left_bottom:
			{
				v2f half_last_dim = last_element_dim / 2;
				v2f half_out_dim = result.dim / 2;
				offset = v2f{-half_last_dim.x - half_out_dim.x - padding, 0};
			
				f32 up_shift = half_out_dim.y - half_last_dim.y;
				offset.y += up_shift;
			}break;
			
			
			case GUI_Build_Direction::right_top:
			{
				v2f half_last_dim = last_element_dim / 2;
				v2f half_out_dim = result.dim / 2;
				offset = v2f{half_last_dim.x + half_out_dim.x + padding, 0};
			
				f32 up_shift = half_last_dim.y - half_out_dim.y;
				offset.y += up_shift;
			}break;
			
			
			case GUI_Build_Direction::right_bottom:
			{
				v2f half_last_dim = last_element_dim / 2;
				v2f half_out_dim = result.dim / 2;
				offset = v2f{half_last_dim.x + half_out_dim.x + padding, 0};
			
				f32 up_shift = half_out_dim.y - half_last_dim.y;
				offset.y += up_shift;
			}break;
		}
		
		result.pos = layout->last_element_pos + offset;
	}
	else
	{
		GUI_Anchor anchor = layout->anchor;
		
		v2f p;
		if(pos == &GUI_AUTO_TOP_CENTER)
		{
			pos = &p;
			
			anchor = GUI_Anchor::top;
			p = {f32(context->canvas->dim.x / 2), f32(context->canvas->dim.y - padding)};
		}
		else if(pos == &GUI_AUTO_TOP_RIGHT)
		{
			pos = &p;
			
			anchor = GUI_Anchor::top_right;
			p = v2f{f32(context->canvas->dim.x - padding), f32(context->canvas->dim.y - padding)};
		}
		else if(pos == &GUI_AUTO_TOP_LEFT)
		{
			pos = &p;
			
			anchor = GUI_Anchor::top_left;
			p = {f32(padding), f32(context->canvas->dim.y - padding)};
		}
		else if(pos == &GUI_AUTO_MIDDLE_RIGHT)
		{
			pos = &p;
			
			anchor = GUI_Anchor::right;
			p = {f32(context->canvas->dim.x - padding), f32(context->canvas->dim.y / 2)};
		}
		else if(pos == &GUI_AUTO_MIDDLE_LEFT)
		{
			pos = &p;
			
			anchor = GUI_Anchor::left;
			p = {f32(padding), f32(context->canvas->dim.y / 2)};
		}
		else if(pos == &GUI_AUTO_BOTTOM_CENTER)
		{
			pos = &p;
			
			anchor = GUI_Anchor::bottom;
			p = {f32(context->canvas->dim.x / 2), padding};
		}
		else if(pos == &GUI_AUTO_BOTTOM_RIGHT)
		{
			pos = &p;
			
			anchor = GUI_Anchor::bottom_right;
			p = {f32(context->canvas->dim.x - padding), padding};
		}
		else if(pos == &GUI_AUTO_BOTTOM_LEFT)
		{
			pos = &p;
			
			anchor = GUI_Anchor::bottom_left;
			p = {padding, padding};
		}
		else if(pos == &GUI_AUTO_MIDDLE)
		{
			pos = &p;
			
			anchor = GUI_Anchor::center;
			p = {f32(context->canvas->dim.x / 2), f32(context->canvas->dim.y / 2)};
		}
		
		v2f half_dim = result.dim / 2;
		result.pos = context->anchor_base + *pos;
		
		switch(anchor)
		{
			case GUI_Anchor::top:
			{
				result.pos.y -= half_dim.y;
			}break;
			
			case GUI_Anchor::bottom:
			{
				result.pos.y += half_dim.y;
			}break;
			
			case GUI_Anchor::left:
			{
				result.pos.x += half_dim.x;
			}break;
			
			case GUI_Anchor::right:
			{
				result.pos.x -= half_dim.x;
			}break;
			
			case GUI_Anchor::top_left:
			{
				result.pos.y -= half_dim.y;
				result.pos.x += half_dim.x;
			}break;
			
			case GUI_Anchor::top_right:
			{
				result.pos.y -= half_dim.y;
				result.pos.x -= half_dim.x;
			}break;
			
			case GUI_Anchor::bottom_left:
			{
				result.pos.y += half_dim.y;
				result.pos.x += half_dim.x;
			}break;
			
			case GUI_Anchor::bottom_right:
			{
				result.pos.y += half_dim.y;
				result.pos.x -= half_dim.x;
			}break;
		}
	}
	
	layout->last_element_pos = result.pos;
	layout->last_element_dim = result.dim;
	
	result.rect = Create_Rect_Center(result.pos, result.dim);

	GUI_Update_Bounds(context, result.rect);

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


static inline void GUI_Reset_Selection_State(GUI_Context* context)
{
	u32 flag_mask = GUI_Context_Flags::cursor_mask_enabled | GUI_Context_Flags::cursor_mask_validation;
	Inverse_Bit_Mask(&context->flags, flag_mask);
	
	context->defered_render = GUI_Defered_Render_Type::none;
	context->selection_state = {};
	context->selected_id = 0;
}


static inline void GUI_Begin_Context(
	GUI_Context* context,
	Platform_Calltable* platform,
	Canvas* canvas,
	Action* actions,
	GUI_Theme* theme,
	v2i canvas_pos = v2i{0, 0},
	GUI_Anchor anchor = GUI_Anchor::top_left,
	GUI_Build_Direction build_direction = GUI_Build_Direction::down_left,
	GUI_Link_Direction::Type ld = GUI_Link_Direction::up)
{
	Assert(canvas);
	Assert(theme);
	Assert(context->widget_count == 0);
	Assert(!GUI_Is_Context_Ready(context));
	Assert(platform);
	Assert(actions);
	
	context->layout = {};
	
	context->actions = actions;
	context->platform = platform;
	context->canvas = canvas;
	context->layout.theme = theme;
	
	context->layout.build_direction = build_direction;
	context->layout.anchor = anchor;
	
	context->canvas_pos = canvas_pos;
	context->layout.last_element_pos = context->anchor_base;
	context->layout_stack_count = 0;
	context->selected_element_dim = {};
	context->selected_element_pos = {};
	context->bounds_rel_anchor_base = { {F32_MAX, F32_MAX}, {-F32_MAX, -F32_MAX} };
	
	Inverse_Bit_Mask(&context->flags, GUI_Context_Flags::cursor_mask_validation);
	
	context->flags |= GUI_Context_Flags::context_ready;
	
	if(Is_Flag_Set(context->platform->Get_Flags(), (u32)App_Flags::is_focused))
	{
		if(context->flags & GUI_Context_Flags::disable_kc_navigation)
		{
			Inverse_Bit_Mask(&context->flags, GUI_Context_Flags::disable_kc_navigation);
		}
		else if(Bit_Not_Set(context->flags, GUI_Context_Flags::ignore_selection))
		{
			if(actions[GUI_Menu_Actions::up].Is_Pressed())
			{
				context->selected_index += -1 * ld;
				GUI_Reset_Selection_State(context);
			}
			
			if(actions[GUI_Menu_Actions::down].Is_Pressed())
			{
				context->selected_index += 1 * ld;
				GUI_Reset_Selection_State(context);
			}			
		}
		
		context->cursor_position = context->platform->Get_Cursor_Position() - context->canvas_pos;
		context->cursor_fpos = v2i::Cast<f32>(context->cursor_position);
	}
	
	context->canvas_rect = Create_Rect_Min_Dim(v2f{0}, v2u::Cast<f32>(context->canvas->dim));
}


static inline void GUI_End_Context(GUI_Context* context)
{
	Assert(context);
	Assert(context->layout_stack_count == 0);
	
	
	// Sometimes you have to rendering some things at the end in order to insure that,
	// draw order is correct.
	// TODO: That said, this is a temprory placeholder, that carrise manny issues.
	// For one the char*[] that is passed into the Do function, has still be valid,
	// when ending the context here.
	switch(context->defered_render)
	{
		case GUI_Defered_Render_Type::dropdown_button:
		{
			GUI_Dropdown_Button_State* state = &context->selection_state.dropdown_button;
			GUI_Theme* theme = state->theme;
			
			Font* font = &theme->font;

			Draw_Filled_Rect_With_Outline(
				context->canvas, 
				state->open_rect, 
				theme->background_color,
				theme->outline_thickness, 
				theme->selected_color);
			
			v2f pos = state->pos;
			
			for(u32 i = 0; i < state->element_count; ++i)
			{
				char* text = state->element_names[i];
				Color text_color = theme->outline_color;
				if(state->selected_element_idx == i)
				{
					Color bg_color = theme->outline_color;
					text_color = theme->background_color;
					
					if(state->is_pressed_down)
					{
						text_color = theme->down_color;
						bg_color = theme->background_color;
					}								
					
					Rect bg_rect = Create_Rect_Center(pos, state->dim);						
					
					Draw_Filled_Rect_With_Outline(
						context->canvas, 
						bg_rect, bg_color, 
						theme->outline_thickness, 
						theme->selected_color);
				}
				
				v2f text_p = GUI_Calc_Centered_Text_Position(text, state->text_scale, pos, font);
				Draw_Text(context->canvas, (u8*)text, text_p, text_color, font, state->text_scale);
				
				pos.y -= state->dim.y;
			}
		}break;
	}
	
	context->defered_render = GUI_Defered_Render_Type::none;
	
	if(Bit_Not_Set(context->flags, GUI_Context_Flags::cursor_mask_validation))
	{
		Inverse_Bit_Mask(&context->flags, GUI_Context_Flags::cursor_mask_enabled);
		context->cursor_mask_area = {};
	}
	
	
	// Selection wrapping.
	if(Bit_Not_Set(context->flags, GUI_Context_Flags::ignore_selection) && 
		Bit_Not_Set(context->flags, GUI_Context_Flags::disable_wrapping))
	{
		if(context->widget_count > 0 &&
			context->selected_index > context->widget_count - 1 &&
			context->widget_count < context->last_widget_count)
		{
			context->selected_index = context->widget_count - 1;
		}
		
		if(context->selected_index < 0)
		{
			context->selected_index = context->widget_count - 1;
			GUI_Reset_Selection_State(context);
		}
		
		if(context->selected_index >= context->widget_count)
		{
			context->selected_index = 0;		
			GUI_Reset_Selection_State(context);
		}
	}
	
	context->last_widget_count = context->widget_count;
	Inverse_Bit_Mask(&context->flags, GUI_Context_Flags::disable_wrapping);
	
	// GUI scrolling
	if(context->flags & GUI_Context_Flags::enable_dynamic_sliders)
	{
		f32 padding = context->layout.theme->padding;
		
		v2f canvas_dim = v2u::Cast<f32>(context->canvas->dim);
		
		f32 canvas_height = canvas_dim.y;
		f32 canvas_width = canvas_dim.x;
		f32 canvas_bottom = 0;
		
		Rect bounds = GUI_Get_Bounds_In_Pixel_Space(context);
		
		// Recovery points if sliderds schange effective canvas size.
		Rect bounds_recovery = bounds;
		
		f32 y_factor = 0;
		{
			if(bounds.min.y >= 0)
				bounds.min.y = 0;
			else
				bounds.min.y -= padding;
			
			if(bounds.max.y < canvas_height)
				bounds.max.y = canvas_height;
			
			else
			{
				bounds.max.y += padding;
				y_factor = bounds.max.y - canvas_height;
			}			
		}
		
		f32 x_factor = 0;
		{
			if(bounds.max.x < canvas_width)
				bounds.max.x = canvas_width;
			
			else
				bounds.max.x += padding;
			
			if(bounds.min.x >= 0)
				bounds.min.x = 0;
			
			else
			{
				bounds.min.x -= padding;
				x_factor = 0 - bounds.min.x;
			}
		}
		
		v2f selected_element_half_dim = context->selected_element_dim / 2;
		
		// NOTE: Done up here as the window sliders can cause selected_element records to be modified.
		f32 selected_element_max_y = context->selected_element_pos.y + selected_element_half_dim.y;
		f32 selected_element_min_y = context->selected_element_pos.y - selected_element_half_dim.y;	
		f32 selected_element_max_x = context->selected_element_pos.x + selected_element_half_dim.x;
		f32 selected_element_min_x = context->selected_element_pos.x - selected_element_half_dim.x;	
		
		bool selected_element_is_window_slider = false;
		bool shift_down = context->platform->Get_Keyboard_Key_Down(Key_Code::LSHIFT) || 
			context->platform->Get_Keyboard_Key_Down(Key_Code::LSHIFT);
		
		// Dynamic sliders!
		{		
			f32 slider_girth = context->dynamic_slider_girth;
			f32 gui_height = bounds.max.y - bounds.min.y;
			
			bool enable_vertical_slider = gui_height > canvas_height && 
				context->canvas->dim.x > slider_girth &&
				context->canvas->dim.y > context->layout.theme->outline_thickness * 2;
			
			if(enable_vertical_slider)
			{
				canvas_width -= slider_girth;
				
				// Recover max.x and recalc.
				bounds.max.x = bounds_recovery.max.x;
				if(bounds.max.x < canvas_width)
					bounds.max.x = canvas_width;
				else
					bounds.max.x += padding;
			}
			
			f32 gui_width = bounds.max.x - bounds.min.x;
			bool enable_horizontal_slider = gui_width > canvas_width && 
				context->canvas->dim.y > slider_girth &&
				context->canvas->dim.x > context->layout.theme->outline_thickness * 2;
			
			if(enable_horizontal_slider)
			{
				f32 canvas_bottom_and_height = canvas_height;
				canvas_bottom = slider_girth;
				canvas_height -= slider_girth;
				
				// Recover min/max.y and recalc.	
				bounds.max.y = bounds_recovery.max.y;
				bounds.min.y = bounds_recovery.min.y;
				{
					if(bounds.min.y >= canvas_bottom)
						bounds.min.y = canvas_bottom;
					else
					{
						//if(bounds.min.y > 0)
						//	bounds.min.y = 0;
						
						bounds.min.y -= padding;
					}
					
					if(bounds.max.y < canvas_bottom_and_height)
						bounds.max.y = canvas_bottom_and_height;
					
					else
					{
						bounds.max.y += padding;
						y_factor = bounds.max.y - canvas_height;
					}
				}
				
				gui_height = bounds.max.y - bounds.min.y;
				
				if(!enable_vertical_slider)
				{
					enable_vertical_slider = gui_height > canvas_height && 
						context->canvas->dim.x > slider_girth &&
						context->canvas->dim.y > context->layout.theme->outline_thickness * 2;
					
					if(enable_vertical_slider)
					{
						canvas_width -= slider_girth;
						
						// Recover max.x and recalc.
						bounds.max.x = bounds_recovery.max.x;
						if(bounds.max.x < canvas_width)
							bounds.max.x = canvas_width;
						else
							bounds.max.x += padding;
					}
				}
			}
			
			if(enable_vertical_slider && enable_horizontal_slider)
			{
				v2f min = v2f{canvas_dim.x - slider_girth, 0};
				v2f dim = v2f{0} + slider_girth;
				
				Color color = context->layout.theme->outline_color;
				Draw_Filled_Rect(context->canvas, Create_Rect_Min_Dim(min, dim), color);
			}
			
			f32 mouse_scroll;
			if(Bit_Not_Set(context->flags, GUI_Context_Flags::disable_mouse_scroll) &&
				Is_Point_Inside_Rect(context->cursor_fpos, context->canvas_rect) &&
				Is_Flag_Set(context->platform->Get_Flags(), (u32)App_Flags::is_focused))
			{
				mouse_scroll = context->platform->Get_Scroll_Wheel_Delta();
			}
			else
				mouse_scroll = 0;
			
			
			v2f anchor_base = context->anchor_base;
			context->anchor_base = {};
			
			if(enable_vertical_slider)
			{
				context->layout.anchor = GUI_Anchor::top_right;
				
				f32 slider_max = (gui_height / canvas_height) - 1;
				v2f slider_dim = {slider_girth, canvas_height};
				f32 slider_value = (anchor_base.y + y_factor) / canvas_height;
				
				GUI_One_Time_Skip_Padding(context);
				GUI_Do_Handle_Slider(
					context, 
					&GUI_AUTO_TOP_RIGHT, 
					&slider_dim, 
					&slider_value,
					slider_max, 
					0,
					GUI_Cardinal_Direction::up_down);
				
				f32 local_scroll_v = (!shift_down)? mouse_scroll : 0;
				f32 scroll_delta = local_scroll_v / canvas_height * GUI_MOUSE_SCROLL_SPEED;
				slider_value -= scroll_delta;
				
				if(context->flags & GUI_Context_Flags::maxout_vertical_slider)
					slider_value = slider_max;
				
				slider_value = Clamp_Zero_To_Max(slider_value, slider_max);
	
				anchor_base.y = (slider_value * canvas_height) - y_factor;
				
				// Selects the slider when using mouse scroll.
				if(local_scroll_v != 0)
					context->selected_index = context->widget_count - 1;
				
				
				if(Bit_Not_Set(context->flags, GUI_Context_Flags::ignore_selection) && 
					context->selected_index == context->widget_count - 1)
				{
					selected_element_is_window_slider = true;
					context->flags |= GUI_Context_Flags::disable_kc_navigation;
					context->flags |= GUI_Context_Flags::disable_wrapping;
					
					v2f pos = context->layout.last_element_pos;
					v2f dim = context->layout.last_element_dim;
					Rect window_slider_rect;
					
					// NOTE: This is an unsafe union access!
					if(context->selection_state.slider.is_held_down)
						window_slider_rect = Create_Rect_Min_Dim(v2f{0}, canvas_dim);
					
					else
					{
						window_slider_rect = Create_Rect_Center(pos, dim);
					}
					
					if(Is_Point_Inside_Rect(context->cursor_fpos, window_slider_rect))
					{
						context->flags |= GUI_Context_Flags::cursor_mask_validation;
						context->flags |= GUI_Context_Flags::cursor_mask_enabled;
	
						context->cursor_mask_area = window_slider_rect;
					}
				}
			}
			else
			{
				anchor_base.y = 0;
			}
			
			if(enable_horizontal_slider)
			{
				context->layout.anchor = GUI_Anchor::bottom_left;
				
				f32 slider_max = (gui_width / canvas_width) - 1;
				v2f slider_dim = {canvas_width, slider_girth};
				f32 slider_value = (-(anchor_base.x - x_factor)) / canvas_width;
				
				GUI_One_Time_Skip_Padding(context);
				GUI_Do_Handle_Slider(
					context, 
					&GUI_AUTO_BOTTOM_LEFT, 
					&slider_dim, 
					&slider_value,
					slider_max, 
					0,
					GUI_Cardinal_Direction::left_right);
				
				f32 local_scroll_v = (shift_down)? mouse_scroll : 0;
				f32 scroll_delta = local_scroll_v / canvas_width * GUI_MOUSE_SCROLL_SPEED;
				slider_value -= scroll_delta;
				
				if(context->flags & GUI_Context_Flags::maxout_horizontal_slider)
					slider_value = slider_max;
				
				slider_value = Clamp_Zero_To_Max(slider_value, slider_max);
				
				anchor_base.x = -(slider_value * canvas_width) + x_factor;
				
				// Selects the slider when using mouse scroll.
				if(local_scroll_v != 0)
					context->selected_index = context->widget_count - 1;
				
				if(Bit_Not_Set(context->flags, GUI_Context_Flags::ignore_selection) && 
					context->selected_index == context->widget_count - 1)
				{
					selected_element_is_window_slider = true;
					context->flags |= GUI_Context_Flags::disable_kc_navigation;
					context->flags |= GUI_Context_Flags::disable_wrapping;
					
					v2f pos = context->layout.last_element_pos;
					v2f dim = context->layout.last_element_dim;
					Rect window_slider_rect;
					
					// NOTE: This is an unsafe union access!
					if(context->selection_state.slider.is_held_down)
						window_slider_rect = Create_Rect_Min_Dim(v2f{0}, canvas_dim);
					
					else
					{
						window_slider_rect = Create_Rect_Center(pos, dim);
					}
					
					if(Is_Point_Inside_Rect(context->cursor_fpos, window_slider_rect))
					{
						context->flags |= GUI_Context_Flags::cursor_mask_validation;
						context->flags |= GUI_Context_Flags::cursor_mask_enabled;
	
						context->cursor_mask_area = window_slider_rect;
					}
				}
			}
			else
			{
				anchor_base.x = 0;
			}
			
			context->anchor_base = anchor_base;
		}
		
		if(Bit_Not_Set(context->flags, GUI_Context_Flags::ignore_selection) 
			&& !selected_element_is_window_slider)
		{
			f32 true_canvas_height = canvas_dim.y;
		
			f32 padding_2 = padding * 2;
			
			f32 padded_e_height = selected_element_max_y - selected_element_min_y + padding_2;
			if(padded_e_height < canvas_height)
			{
				// Selection below the canvas.
				if(selected_element_min_y < canvas_bottom)
				{
					f32 dif = canvas_bottom - selected_element_min_y + padding;
					context->anchor_base.y += dif;
				}
				
				// Selection above the canvas.
				else if(selected_element_max_y > true_canvas_height)
				{
					f32 dif = true_canvas_height - selected_element_max_y - padding;
					context->anchor_base.y += dif;
				}
				
			}
			
			f32 padded_e_width 	= selected_element_max_x - selected_element_min_x + padding_2;
			if(padded_e_width < canvas_width)
			{				
				// Selection left the canvas.
				if(selected_element_min_x < 0)
				{
					f32 dif = 0 - selected_element_min_x + padding;
					context->anchor_base.x += dif;
				}
				
				// Selection right the canvas.
				else if(selected_element_max_x > canvas_width)
				{
					f32 dif = canvas_width - selected_element_max_x - padding;
					context->anchor_base.x += dif;
				}			
			}
		}
	}
	
	// Reset state.
	{
		context->last_cursor_position = context->cursor_position;
		
		u32 reset_mask = 
			GUI_Context_Flags::context_ready | 
			GUI_Context_Flags::maxout_horizontal_slider | 
			GUI_Context_Flags::maxout_vertical_slider;
		
		Inverse_Bit_Mask(&context->flags, reset_mask);
		context->widget_count = 0;
		context->canvas = 0;
	}
}


// Generates a non zero id based on the position, dimensions and some mangle factor provided
// by the calling code. Usually that would be the line number from the __LINE__ macro.
// There is really no gurantee that there wont be ID collissions. This really is just hoping
// the program is correct, where randomly it might not be.
// TODO: upgrade to use a 64 bite noise function to at least reduce the change of collissions.
static u32 GUI_Generate_ID(Rect rect, u32 mangle_factor)
{
	v2i np = v2f::Cast<i32>(Round(rect.min + rect.max));
	u32 id;
	do
	{
		id = 1 + mangle_factor + Noise_Squirrel3_2D(np, mangle_factor++);
	}while(!id);
	
	return id;
}


static inline bool GUI_Is_Element_Selected(GUI_Context* context, bool cursor_on_selection, u32 id)
{
	// TODO: Again re-think this.. just kind of awfull.
	bool result = false;
	
	// This is the selected element.
	if(Bit_Not_Set(context->flags, GUI_Context_Flags::ignore_selection) &&
		context->selected_index == context->widget_count)
	{
		// Buuut, it seems to be a different widget?
		if(id != context->selected_id)
		{
			GUI_Reset_Selection_State(context);
			context->selected_id = id;
		}
		
		context->selected_element_pos = context->layout.last_element_pos;
		context->selected_element_dim = context->layout.last_element_dim;
		
		result = true;
	}
	else if(cursor_on_selection && 
		(context->cursor_position != context->last_cursor_position || 
		context->actions[GUI_Menu_Actions::mouse].Is_Down()) &&
		Is_Point_Inside_Rect(context->cursor_fpos, context->canvas_rect) &&
		!((context->flags & GUI_Context_Flags::cursor_mask_enabled) && 
		Is_Point_Inside_Rect(context->cursor_fpos, context->cursor_mask_area))
	)
	{
		GUI_Reset_Selection_State(context);
		context->selected_index = context->widget_count;
		context->selected_id = id;
		
		context->selected_element_pos = context->layout.last_element_pos;
		context->selected_element_dim = context->layout.last_element_dim;		
		result = true;

		Inverse_Bit_Mask(&context->flags, GUI_Context_Flags::ignore_selection);
	}		

	context->widget_count += 1; // Suprising side effect! But better to do it here than forget to do it ouside.
	return result;
}


static inline bool GUI_On_Release_Action(
	GUI_Context* context, bool cursor_on_selection, bool* is_pressed_down)
{
	bool pressed = context->actions[GUI_Menu_Actions::enter].Is_Pressed() ||
		(cursor_on_selection && context->actions[GUI_Menu_Actions::mouse].Is_Pressed());
	
	bool released = context->actions[GUI_Menu_Actions::enter].Is_Released() ||
		(cursor_on_selection && context->actions[GUI_Menu_Actions::mouse].Is_Released());
	
	if(pressed)
	{
		*is_pressed_down = true;
	}
	else if(*is_pressed_down && released)
	{
		*is_pressed_down = false;
		return true;
	}
	else if(context->actions[GUI_Menu_Actions::mouse].Is_Up() && 
		context->actions[GUI_Menu_Actions::enter].Is_Up())
	{
		*is_pressed_down = false;
	}
	
	return false;
}


static bool GUI_Input_Field_Insert_Characters(
	Platform_Calltable* platform, 
	GUI_SL_Input_Field_State* state, 
	String* str,
	u32 character_limit,
	u32 wcp,
	bool (*character_check)(char*))
{
	bool(*Is_Allowed_Character)(char) = [](char c)
	{
		bool result = (c >= 32 && c <= 127) || c == -28 || c == -60 || c == -10 || c == -42;
		return result;
	};
	
	
	void(*Conditional_Erase_Selection)(GUI_SL_Input_Field_State*, String*) =
		[](GUI_SL_Input_Field_State* state, String* str)
	{
		if(state->text_select_mode)
		{
			u32 p0 = state->write_cursor_position;
			u32 p1 = state->text_select_start_point;
			
			if(p0 != p1)
			{
				if(p0 > p1)
					Swap(&p0, &p1);
				
				str->erase(p0, p1);
				
				state->write_cursor_position = p0;
			}
			
			state->text_select_mode = false;
		}
	};
	
	
	Char_Array typing_info = platform->Get_Typing_Information();
	
	bool result = false;
	
	
	for(u32 i = 0; i < typing_info.count; ++i)
	{
		char c = typing_info.buffer[i];
		
		switch(c)
		{
			case '\x1': // CTRL A (Select all)
			{
				state->write_cursor_position = str->lenght;
				state->text_select_start_point = 0;
				state->text_select_mode = true;
				
			}break;
			
			
			case '\x3': // CTRL C (Copy selection to clipboard)
			{
				if(state->text_select_mode)
				{
					u32 p0 = state->write_cursor_position;
					u32 p1 = state->text_select_start_point;
					
					if(p0 != p1)
					{
						if(p0 > p1)
							Swap(&p0, &p1);
						
						char* buffer_start = str->buffer + p0;
						u32 buffer_lenght = p1 - p0;
						
						platform->Set_Clipboard_Data_As_Text(buffer_start, buffer_lenght);
					}
				}
				
			}break;
			
			
			case '\x18': // CTRL X (Copy selection to clipboard, then delete selection)
			{
				if(state->text_select_mode)
				{
					u32 p0 = state->write_cursor_position;
					u32 p1 = state->text_select_start_point;
					
					if(p0 != p1)
					{
						if(p0 > p1)
							Swap(&p0, &p1);
						
						char* buffer_start = str->buffer + p0;
						u32 buffer_lenght = p1 - p0;
						
						platform->Set_Clipboard_Data_As_Text(buffer_start, buffer_lenght);

						str->erase(p0, p1);
						state->write_cursor_position = p0;
						state->text_select_mode = false;
					}
				}
				
			}break;
			
			
			case '\x16': // CTRL V (Paste from clipboard)
			{
				Conditional_Erase_Selection(state, str);
				
				char* clip_buffer = platform->Get_Clipboard_Data_As_Text();
				if(clip_buffer)
				{
					for(char cb = *clip_buffer; cb; ++clip_buffer, cb = *clip_buffer)
					{
						if(Is_Allowed_Character(cb) && 
							(character_limit == 0 || str->lenght < character_limit) &&
							(!character_check || (character_check && character_check(&cb))))
						{
							str->insert_at(state->write_cursor_position, *clip_buffer);
							state->write_cursor_position += 1;										
						}
					}
				}
				
			}break;
			
			
			case '\r': // ENTER hopefully, depends on the OS?
			{
				result = true;
				
				//GUI_Reset_Selection_State(context);
			}break;
			
			
			case '\b': // BACKSPACE
			{
				if(state->text_select_mode) 
				{
					Conditional_Erase_Selection(state, str);
					continue;
				}
				
				if(str->lenght > 0)
				{
					state->write_cursor_position -= 1;
					if(state->write_cursor_position > wcp)
						state->write_cursor_position = 0;
					
					str->remove_at(state->write_cursor_position);
				}
				
			}break;
			
			
			default:
			{
				if(Is_Allowed_Character(c) && 
					(character_limit == 0 || str->lenght < character_limit) &&
					(!character_check || (character_check && character_check(&c))))
				{
					Conditional_Erase_Selection(state, str);
					
					str->insert_at(state->write_cursor_position, c);
					state->write_cursor_position += 1;
				}
			}
		}
	}
	
	return result;
}


static void GUI_Do_Spacing(
	GUI_Context* context, 
	v2f* dim)
{
	Assert(GUI_Is_Context_Ready(context));
	
	Rect bounds = context->bounds_rel_anchor_base;
	
	GUI_One_Time_Skip_Padding(context);
	if(dim)
	{
		v2f d = *dim;
		if(d.x == 0)
			d.x = context->layout.last_element_dim.x;
		
		if(d.y == 0)
			d.y = context->layout.last_element_dim.y;
		
		GUI_Get_Placement(context, &d, 0);
	}
	else // AUTO case
	{		
		GUI_Get_Placement(context, dim, 0);
	}
	
	context->bounds_rel_anchor_base = bounds;
}


static void GUI_Do_Text(
	GUI_Context* context, 
	v2f* pos, 
	char* text,
	GUI_Highlight highlight = GUI_Highlight_Nothing(),
	v2f text_scale = GUI_DEFAULT_TEXT_SCALE,
	bool is_title = false)
{
	Assert(GUI_Is_Context_Ready(context));
	
	// --------------------------------------------------------------------------
	
	GUI_Theme* theme = context->layout.theme;
	
	v2f dim = GUI_Tight_Fit_Text(text, text_scale, &theme->font);
	
	GUI_Placement p = GUI_Get_Placement(context, &dim, pos);

	v2f text_p = GUI_Calc_Centered_Text_Position(text, text_scale, p.pos, &theme->font);
	
	// --------------------------------------------------------------------------
	
	if(Rects_Overlap(p.rect, context->canvas_rect))
	{
		bool is_highlighted = Bit_Not_Set(context->flags, GUI_Context_Flags::ignore_selection) && 
			highlight.highlight_count && context->selected_index >= highlight.idx && 
			context->selected_index < highlight.idx + highlight.highlight_count;
		
		Color color;
		if(is_title)
			color = theme->title_color;
		else
			color = is_highlighted? theme->selected_color : theme->text_color;
		
		Draw_Text(context->canvas, (u8*)text, text_p, color, &theme->font, text_scale);		
	}
}


static bool GUI_Do_Button(
	GUI_Context* context, 
	v2f* pos, 
	v2f* dim, 
	char* text, 
	v2f text_scale = GUI_DEFAULT_TEXT_SCALE)
{
	Assert(GUI_Is_Context_Ready(context));
	
	// --------------------------------------------------------------------------
	
	GUI_Theme* theme = context->layout.theme;
	
	if(dim == &GUI_AUTO_FIT)
	{
		*dim = GUI_Tight_Fit_Text(text, text_scale, &theme->font) + theme->padding;
	}
	
	GUI_Placement p = GUI_Get_Placement(context, dim, pos);
	
	
	u32 id = GUI_Generate_ID(p.rect, __LINE__);
	
	bool cursor_on_selection = Is_Point_Inside_Rect(context->cursor_fpos, p.rect);
	bool is_selected = GUI_Is_Element_Selected(context, cursor_on_selection, id);
	
	// --------------------------------------------------------------------------
	
	bool result = false;
	
	Color outline_color = outline_color = theme->outline_color;
	
	if(is_selected)
	{
		GUI_Button_State* state = &context->selection_state.button;
		
		// Handle input.
		if(GUI_On_Release_Action(context, cursor_on_selection, &state->is_pressed_down))
		{
			result = true;
		}
		
		if(state->is_pressed_down)
			outline_color = theme->down_color;
		else
			outline_color = theme->selected_color;
	}
	
	// Draw
	if(Rects_Overlap(p.rect, context->canvas_rect))
	{
		Draw_Filled_Rect_With_Outline(
			context->canvas, 
			p.rect, 
			theme->background_color,
			theme->outline_thickness, 
			outline_color);
		
		if(text)
		{
			v2f text_p = GUI_Calc_Centered_Text_Position(text, text_scale, p.pos, &theme->font);
			Draw_Text(context->canvas, (u8*)text, text_p, outline_color, &theme->font, text_scale);
		}
	}
	
	return result;
}


static bool GUI_Do_Fill_Slider(
	GUI_Context* context, 
	v2f* pos, 
	v2f* dim, 
	f32* value,
	f32 max = 1.f, 
	f32 min = 0.f, 
	f32 step = 0.01f,
	GUI_Input_Acceleration_Behavior inp_accel = GUI_Input_Acceleration_Behavior())
{
	Assert(GUI_Is_Context_Ready(context));
	
	Assert(step > 0);
	Assert(max > min);
	Assert(value);
	
	// --------------------------------------------------------------------------
	
	GUI_Placement p = GUI_Get_Placement(context, dim, pos);
	
	u32 id = GUI_Generate_ID(p.rect, __LINE__);
	
	bool cursor_on_selection = Is_Point_Inside_Rect(context->cursor_fpos, p.rect);
	bool is_selected = GUI_Is_Element_Selected(context, cursor_on_selection, id);
	
	GUI_Theme* theme = context->layout.theme;
	
	// --------------------------------------------------------------------------
	
	f32 pre_val = *value;
	
	*value = Min(*value, max);
	*value = Max(*value, min);
	
	Color bar_color; 
	
	if(is_selected)
	{
		GUI_Slider_State* state = &context->selection_state.slider;
		bar_color = theme->selected_color;		
		
		f64 time = context->platform->Get_Time_Stamp();
		
		// Handle mouse input -----------------
		if(context->actions[GUI_Menu_Actions::mouse].Is_Pressed() && cursor_on_selection)
			state->is_held_down = true;
		
		if(context->actions[GUI_Menu_Actions::mouse].Is_Up())
			state->is_held_down = false;
		
		if(state->is_held_down)
		{
			f32 internal_width = p.dim.x - (f32(theme->outline_thickness) * 2);
			f32 rel_cursor_x = Min(internal_width, Max(0.f, context->cursor_fpos.x - p.rect.min.x));
			
			f32 fill_percent = rel_cursor_x / internal_width;
			f32 fill = (max - min) * fill_percent;
			i32 steps = Round_To_Signed_Int32(fill / step);
			
			*value = min + steps * step;
		}
		
		
		// Handle input Keyboard/Controller.
		Action* left = context->actions + GUI_Menu_Actions::left;
		Action* right = context->actions + GUI_Menu_Actions::right; 
		
		if(left->Is_Pressed() || right->Is_Pressed())
		{
			state->input_start_time = 0;
			state->next_input_time = 0;
		}	
	
		while(left->Is_Down() &&
			GUI_Accelerated_Tick(&inp_accel, time, &state->input_start_time, &state->next_input_time))
		{
			*value = Max(*value - step, min);
		}
		
		while(right->Is_Down() &&
			GUI_Accelerated_Tick(&inp_accel, time, &state->input_start_time, &state->next_input_time))
		{		
			*value = Min(*value + step, max);
		}
	}
	else
	{
		bar_color = theme->outline_color;
	}
	
	// Draw
	if(Rects_Overlap(p.rect, context->canvas_rect))
	{
		f32 d = max - min;
		Assert(d > 0);
		
		f32 fill = 0;
		if(d != 0)
			fill = (*value - min) / d;
		
		Draw_Percentile_Bar(
			context->canvas,
			p.rect,
			theme->background_color,
			theme->outline_thickness,
			bar_color,
			bar_color,
			fill);		
	}
	
	return pre_val != *value;
}


static bool GUI_Do_Handle_Slider(
	GUI_Context* context, 
	v2f* pos, 
	v2f* dim, 
	f32* value,
	f32 max,
	f32 min,
	GUI_Cardinal_Direction cardinal_dir,
	f32 step_count,
	GUI_Input_Acceleration_Behavior inp_accel)
{
	Assert(GUI_Is_Context_Ready(context));
	
	static constexpr f32 min_handle_width = 3.f;
	
	f32 pre_val = *value;
	
	if(*value < min)
		*value = min;
	
	if(*value > max)
		*value = max;
	
	// --------------------------------------------------------------------------
	
	GUI_Placement p = GUI_Get_Placement(context, dim, pos);
	
	u32 id = GUI_Generate_ID(p.rect, __LINE__);
	
	bool cursor_on_selection = Is_Point_Inside_Rect(context->cursor_fpos, p.rect);
	bool is_selected = GUI_Is_Element_Selected(context, cursor_on_selection, id);
	
	GUI_Theme* theme = context->layout.theme;
	
	// --------------------------------------------------------------------------
	
	Color outline_color = theme->outline_color;
	Color handle_color = theme->selected_color;
	
	v2f internal_dim = p.dim - (f32(theme->outline_thickness) * 2);
	
	f32 range = max - min;
	f32 possibility_space = range + 1;
	v2f handle_pos = p.pos;
	
	f32 bar_internal_lenght = internal_dim.elements[u8(cardinal_dir)];
	
	if(step_count == 0)
		step_count = bar_internal_lenght;
	
	
	f32 handle_lenght = Max(bar_internal_lenght / possibility_space, min_handle_width);
	f32 half_handle_lenght = handle_lenght / 2;
	f32 slidable_lenght = bar_internal_lenght - handle_lenght;
	
	f32 divizor = possibility_space - 1;
	
	if(divizor == 0) // CONSIDER: Epsilon?
		divizor = 1;
	
	f32 directional_offset = 
		(-bar_internal_lenght / 2 + half_handle_lenght) + ((*value - min) * (slidable_lenght / divizor));
	
	v2f handle_dim;
	
	switch(cardinal_dir)
	{
		case GUI_Cardinal_Direction::left_right:
		{
			handle_dim = v2f{handle_lenght, internal_dim.y};
			handle_pos += v2f{directional_offset, 0};
		}break;
		
		
		case GUI_Cardinal_Direction::up_down:
		{
			handle_dim = v2f{internal_dim.x, handle_lenght};
			handle_pos += v2f{0, -directional_offset};
		}break;
	}
	
	Rect handle_rect = Create_Rect_Center(handle_pos, handle_dim);
	
	bool cursor_on_handle = Is_Point_Inside_Rect(context->cursor_fpos, handle_rect);
	if(is_selected)
	{
		if(cursor_on_handle)
		{
			handle_color = theme->title_color;
		}
		
		GUI_Slider_State* state = &context->selection_state.slider;
		
		outline_color = theme->selected_color;
		f32 step = possibility_space / step_count;
		
		// Handle mouse input -----------------
		if(context->actions[GUI_Menu_Actions::mouse].Is_Pressed() && cursor_on_selection)
		{
			if(cursor_on_handle)
				state->drag_offset = context->cursor_fpos - handle_pos;
			else
				state->drag_offset = v2f{0};
			
			state->is_held_down = true;
		}
		
		
		if(context->actions[GUI_Menu_Actions::mouse].Is_Up())
			state->is_held_down = false;
		
		
		if(state->is_held_down)
		{
			f32 cursor_pos_on_sliding_axis = context->cursor_fpos.elements[u8(cardinal_dir)];
			cursor_pos_on_sliding_axis -= state->drag_offset.elements[u8(cardinal_dir)];
			
			f32 y_flip = (1 - (f32(cardinal_dir) * 2));
			
			f32 rel_cursor = 
				cursor_pos_on_sliding_axis - (p.pos.elements[u8(cardinal_dir)] - (bar_internal_lenght / 2));
			
			rel_cursor -= half_handle_lenght * y_flip;
			
			rel_cursor = (f32(cardinal_dir) * bar_internal_lenght) + (rel_cursor * y_flip);
			
			
			f32 percent = rel_cursor / slidable_lenght;
			
			f32 v = (percent * range);
			f32 steps = Round(v / step);
			v = step * steps;
			
			v += min;
			v = Min(v, max);
			v = Max(v, min);
			
			*value = v;
		}
		
		f64 time = context->platform->Get_Time_Stamp();
		
		// Handle input Keyboard/Controller.
		Action* left = context->actions + GUI_Menu_Actions::left;
		Action* right = context->actions + GUI_Menu_Actions::right; 
		
		if(left->Is_Pressed() || right->Is_Pressed())
		{
			state->input_start_time = 0;
			state->next_input_time = 0;
		}	
		
		if(left->Is_Down())
		{
			handle_color = theme->title_color;
			while(GUI_Accelerated_Tick(&inp_accel, time, &state->input_start_time, &state->next_input_time))
			{
				f32 new_value = *value - step;
				*value = Min(max, Max(min, new_value));	
			}
		}

		
		if(right->Is_Down())
		{
			handle_color = theme->title_color;
			while(GUI_Accelerated_Tick(&inp_accel, time, &state->input_start_time, &state->next_input_time))
			{
				f32 new_value = *value + step;
				*value = Min(max, Max(min, new_value));
			}
		}
	}
	
	// Draw
	if(Rects_Overlap(p.rect, context->canvas_rect))
	{
		Draw_Filled_Rect_With_Outline(
			context->canvas, 
			p.rect, 
			theme->background_color,
			theme->outline_thickness, 
			outline_color);
		
		Draw_Filled_Rect(context->canvas, handle_rect, handle_color);		
	}
	
	return pre_val != *value;
}	


static bool GUI_Do_Checkbox(GUI_Context* context, v2f* pos, v2f* dim, bool* value)
{
	Assert(GUI_Is_Context_Ready(context));
	
	// --------------------------------------------------------------------------
	
	GUI_Placement p = GUI_Get_Placement(context, dim, pos);
	
	u32 id = GUI_Generate_ID(p.rect, __LINE__);
	
	bool cursor_on_selection = Is_Point_Inside_Rect(context->cursor_fpos, p.rect);
	bool is_selected = GUI_Is_Element_Selected(context, cursor_on_selection, id);
	
	GUI_Theme* theme = context->layout.theme;
	
	// --------------------------------------------------------------------------
	
	bool pre_val = *value;
	
	
	Color outline_color = theme->outline_color;
	if(is_selected)
	{
		GUI_Button_State* state = &context->selection_state.button;
		
		if(GUI_On_Release_Action(context, cursor_on_selection, &state->is_pressed_down))
		{
			*value = !(*value);
		}
		
		if(state->is_pressed_down)
			outline_color = theme->down_color;
		else
			outline_color = theme->selected_color;
	}
	
	// Draw
	{		
		Draw_Filled_Rect_With_Outline(
			context->canvas, 
			p.rect, 
			theme->background_color,
			theme->outline_thickness, 
			outline_color);

		if(*value)
		{
			// half here on purpose
			Rect rect = Create_Rect_Center(p.pos, p.dim * 0.5f);
			Draw_Filled_Rect(context->canvas, rect, outline_color);
		}		
	}
	
	return pre_val != *value;
}


static u32 GUI_Do_Dropdown_Button(
	GUI_Context* context, 
	v2f* pos,
	v2f* dim,
	char* text,
	u32 element_count,
	char** element_names,
	v2f text_scale = GUI_DEFAULT_TEXT_SCALE)
{
	Assert(GUI_Is_Context_Ready(context));
	
	Assert(element_count > 0);
	
	// --------------------------------------------------------------------------
	
	GUI_Theme* theme = context->layout.theme;
	
	if(dim == &GUI_AUTO_FIT)
	{
		u32 button_text_lenght = Null_Terminated_Buffer_Lenght(text);
		u32 longest_sub_lenght = Null_Terminated_Buffer_Lenght(element_names[0]);
		u32 longest_sub_idx = 0;
		
		for(u32 i = 1; i < element_count; ++i)
		{
			u32 text_lenght = Null_Terminated_Buffer_Lenght(element_names[i]);
			if(text_lenght > longest_sub_lenght)
			{
				longest_sub_lenght = text_lenght;
				longest_sub_idx = i;
			}
		}
		
		char* longest_text = (button_text_lenght >= longest_sub_lenght)? 
			text : element_names[longest_sub_idx];
		
		*dim = GUI_Tight_Fit_Text(longest_text, text_scale, &theme->font) + theme->padding;
	}
	
	
	GUI_Placement p = GUI_Get_Placement(context, dim, pos);
	
	Rect id_rect = {p.rect.min - context->anchor_base, p.rect.max - context->anchor_base};
	
	u32 id = GUI_Generate_ID(id_rect, __LINE__);
	
	bool cursor_on_selection = Is_Point_Inside_Rect(context->cursor_fpos, p.rect);
	bool is_selected = GUI_Is_Element_Selected(context, cursor_on_selection, id);
	
	// --------------------------------------------------------------------------
	
	Color outline_color = outline_color = theme->outline_color;
	
	u32 result = 0;
	
	f32 half_dim_y = p.dim.y / 2;
	v2f open_center = v2f{p.pos.x, p.pos.y - (half_dim_y * f32(element_count) - half_dim_y)};	
	v2f open_dim = v2f{p.dim.x, p.dim.y * f32(element_count)};
	
	Rect open_rect = Create_Rect_Center(open_center, open_dim);
	
	
	if(is_selected)
	{
		GUI_Dropdown_Button_State* state = &context->selection_state.dropdown_button;
		
		bool cursor_is_in_open_rect = Is_Point_Inside_Rect(context->cursor_fpos, open_rect);
		
		if(state->is_open)
		{
			if(context->actions[GUI_Menu_Actions::back].Is_Pressed() || 
				(context->actions[GUI_Menu_Actions::mouse].Is_Pressed() && !cursor_is_in_open_rect))
			{
				state->is_open = false;
				state->is_pressed_down = false;
			}
			else
			{
				GUI_Update_Bounds(context, open_rect);
				context->selected_element_pos = open_center;
				context->selected_element_dim = open_dim;
				
				context->flags |= GUI_Context_Flags::disable_kc_navigation;
				
				// KC controls
				if(context->actions[GUI_Menu_Actions::up].Is_Pressed())
				{
					if(state->selected_element_idx == 0)
						state->selected_element_idx = element_count - 1;
					else
						state->selected_element_idx -= 1;
					
					state->is_pressed_down = false;
				}
				
				
				if(context->actions[GUI_Menu_Actions::down].Is_Pressed())
				{
					if(state->selected_element_idx == element_count - 1)
						state->selected_element_idx = 0;
					else
						state->selected_element_idx += 1;
					
					state->is_pressed_down = false;
				}
				
				bool cursor_on_sub_selection = false;
				
				// Mouse controls
				if(cursor_is_in_open_rect)
				{
					u32 pre_selection = state->selected_element_idx;
					f32 level = p.pos.y - p.dim.y / 2;
					for(u32 i = 0; i < element_count; ++i)
					{
						if(f32(context->cursor_fpos.y) > level)
						{
							if(context->cursor_position != context->last_cursor_position || 
								context->actions[GUI_Menu_Actions::mouse].Is_Pressed())
							{
								state->selected_element_idx = i;
							}
							
							cursor_on_sub_selection = true;
							break;
						}
						level -= p.dim.y;
					}
					
					if(pre_selection != state->selected_element_idx)
						state->is_pressed_down = false;
				}
				
				
				if(GUI_On_Release_Action(context, cursor_on_sub_selection, &state->is_pressed_down))
				{
					state->is_open = false;
					result = state->selected_element_idx + 1;
				}
			}
		}
		
		else
		{
			if(state->is_pressed_down)
				outline_color = theme->down_color;
			else
				outline_color = theme->selected_color;
			
			if(GUI_On_Release_Action(context, cursor_on_selection, &state->is_pressed_down))
			{
				state->is_open = true;
				if(context->actions[GUI_Menu_Actions::mouse].Is_Released())
					state->selected_element_idx = 0;
			}
		}		
		
		// Re-check if the box is open or not. May have changed in the above code block.
		if(state->is_open)
		{
			context->defered_render = GUI_Defered_Render_Type::dropdown_button;
			
			context->cursor_mask_area = open_rect;
			context->flags |= GUI_Context_Flags::cursor_mask_enabled;
			context->flags |= GUI_Context_Flags::cursor_mask_validation;
			
			state->selected_element_idx = Min(state->selected_element_idx, element_count - 1);
			state->element_count = element_count;
			state->element_names = element_names;
			state->open_rect = open_rect;
			state->pos = p.pos;
			state->dim = p.dim;
			state->theme = theme;
			state->text_scale = text_scale;
			
			return 0; // Skip the rest of the function. Drawring is handled in the End_Context function.
		}
	}
	
	// Draw closed mode.
	if(Rects_Overlap(p.rect, context->canvas_rect))
	{
		Draw_Filled_Rect_With_Outline(
			context->canvas, 
			p.rect, 
			theme->background_color,
			theme->outline_thickness, 
			outline_color);
		
		if(text)
		{
			v2f text_p = GUI_Calc_Centered_Text_Position(text, text_scale, p.pos, &theme->font);
			Draw_Text(context->canvas, (u8*)text, text_p, outline_color, &theme->font, text_scale);
		}
	}
	
	return result;
}


static bool GUI_Do_SL_Input_Field(
	GUI_Context* context, 
	v2f* pos, 
	f32* width,
	String* str,
	u32 character_limit = 256,
	v2f text_scale = GUI_DEFAULT_TEXT_SCALE,
	bool (*character_check)(char*) = 0,
	GUI_Input_Acceleration_Behavior keyboard_acceleration = {0.2, 0.3, 30})
{
	Assert(GUI_Is_Context_Ready(context));

	Assert(str);
	
	static constexpr f32 min_handle_width = 3.f;
	
	GUI_Theme* theme = context->layout.theme;
	Font* font = &theme->font;
	
	f32 outline_thickness = f32(theme->outline_thickness);
	f32 char_width = f32(font->char_width) * text_scale.x;
	f32 char_height = f32(font->char_height) * text_scale.y;
	
	f32 bar_height = Ceil(char_height * 0.33f);
	
	v2f dim;
	{
		f32 _width = width? *width : context->layout.last_element_dim.x;
		f32 _height = char_height + bar_height + outline_thickness * 2;
		if((u32)_height % 2)
			_height += 1;
		
		dim = {_width, _height};		
	}
	
	// --------------------------------------------------------------------------
	
	GUI_Placement p = GUI_Get_Placement(context, &dim, pos);
	
	u32 id = GUI_Generate_ID(p.rect, __LINE__);
	

	bool cursor_on_selection = Is_Point_Inside_Rect(context->cursor_fpos, p.rect);
	bool is_selected = GUI_Is_Element_Selected(context, cursor_on_selection, id);
	
	
	// --------------------------------------------------------------------------
	
	bool result = false;
	
	Color outline_color = theme->outline_color;
	u32 view_offset = 0;
	
	v2f text_p = p.pos + v2f{1, -1} * outline_thickness + v2f{2,-2} +
		Hadamar_Product(v2f{-1, 1}, (p.dim / 2));
	text_p.y -= char_height;
	
	u32 view_limit; // Amount of charactes that can fit inside the text area.
	// Calculate view limit.
	{
		// w: dif between text_p and p.pos twice.
		f32 w = (text_p.x - p.rect.min.x) * 2;
		view_limit = (u32)Floor((p.dim.x - w) / char_width);	
	}
	
	// "Bar" is the slider area at the bottom.
	v2f bar_dim;
	v2f bar_center;
	Rect bar_rect;
	
	{
		f32 bar_offset_y = p.dim.y / 2 - bar_height / 2 - outline_thickness;
		bar_dim = { p.dim.x - outline_thickness * 2, bar_height };
		bar_center = p.pos - v2f{0, bar_offset_y};
		bar_rect = Create_Rect_Center(bar_center, bar_dim);
	}
	
	f32 f_lenght = f32(str->lenght) + 1.f;
	f32 handle_width = Max(bar_dim.x * view_limit / f_lenght, min_handle_width);
	
	bool draw_cursor = false;
	u32 write_cursor_position = 0;
	
	bool text_select_mode = false;
	u32 text_select_start_point = 0;
	
	if(is_selected)
	{
		GUI_SL_Input_Field_State* state = &context->selection_state.sl_input_field;
		
		draw_cursor = state->draw_cursor && state->is_active;
		
		// Handle input
		bool mouse_pressed_down = context->actions[GUI_Menu_Actions::mouse].Is_Pressed();
		bool mouse_is_down = context->actions[GUI_Menu_Actions::mouse].Is_Down();
		
		if(state->is_active)
		{
			if(context->actions[GUI_Menu_Actions::back].Is_Pressed() || 
				(!cursor_on_selection && mouse_pressed_down))
			{
				draw_cursor = false;
				state->is_active = false;
				state->view_offset = 0;
				
				Inverse_Bit_Mask(&context->flags, GUI_Context_Flags::cursor_mask_validation);
			}
			else
			{
				context->flags |= GUI_Context_Flags::cursor_mask_validation;
				context->flags |= GUI_Context_Flags::cursor_mask_enabled;	
				
				context->cursor_mask_area = context->canvas_rect;
				
				u32 wcp = state->write_cursor_position;
				
				if(GUI_Input_Field_Insert_Characters(
					context->platform, 
					state, 
					str, 
					character_limit, 
					wcp, 
					character_check))
				{
					GUI_Reset_Selection_State(context);
					result = true;
				}
				
				// NOTE: string lenght is a dependancy for f_lenght, thefore it's updated here,
				// now that the final lenght for the frame is known.
				f_lenght = f32(str->lenght) + 1.f;
				handle_width = Max(bar_dim.x * view_limit / f_lenght, min_handle_width);
				
				f64 time = context->platform->Get_Time_Stamp();
				state->draw_cursor = !(time > state->flicker_start_time) || (u64)time % 2;
				
				if(context->actions[GUI_Menu_Actions::left].Is_Pressed() ||
					context->actions[GUI_Menu_Actions::right].Is_Pressed())
				{
					state->input_start_time = 0;
					state->next_input_time = 0;
				}
				
				bool shift_down = context->platform->Get_Keyboard_Key_Down(Key_Code::LSHIFT) || 
					context->platform->Get_Keyboard_Key_Down(Key_Code::LSHIFT);
				
				// Arrowkey/controller input 
				while(context->actions[GUI_Menu_Actions::left].Is_Down() && GUI_Accelerated_Tick(
					&keyboard_acceleration, time, &state->input_start_time, &state->next_input_time))
				{
					bool pre_text_select = state->text_select_mode;
					
					if(shift_down)
					{
						if(!state->text_select_mode)
						{
							state->text_select_start_point = state->write_cursor_position;
							state->text_select_mode = true;
						}
					}
					else
						state->text_select_mode = false;
					
					if(pre_text_select && !state->text_select_mode)
						break;
					
					state->write_cursor_position -= 1;
					
					if(state->write_cursor_position > wcp)
					{
						if(context->actions[GUI_Menu_Actions::left].Is_Pressed())
							state->write_cursor_position = str->lenght;	
						
						else
							state->write_cursor_position = wcp;
					}
				}
				
				
				while(context->actions[GUI_Menu_Actions::right].Is_Down() && GUI_Accelerated_Tick(
					&keyboard_acceleration, time, &state->input_start_time, &state->next_input_time))
				{
					bool pre_text_select = state->text_select_mode;
					
					if(shift_down)
					{
						if(!state->text_select_mode)
						{
							state->text_select_start_point = state->write_cursor_position;
							state->text_select_mode = true;
						}
					}
					else
						state->text_select_mode = false;
					
					if(pre_text_select && !state->text_select_mode)
						break;
					
					state->write_cursor_position += 1;
					
					if(state->write_cursor_position > str->lenght)
					{
						if(context->actions[GUI_Menu_Actions::right].Is_Pressed())
							state->write_cursor_position = 0;
						else
							state->write_cursor_position = wcp;
					}
				}
				
				// mouse behavior with cursor and handle bar.
				if(str->lenght > 0)
				{
					bool mouse_y_on_text_area = (context->cursor_fpos.y >= text_p.y &&
						context->cursor_fpos.y < text_p.y + char_height);
					
					if(mouse_pressed_down && mouse_y_on_text_area)
					{
						if(shift_down)
						{
							if(!state->text_select_mode)
							{
								state->text_select_start_point = state->write_cursor_position;
								state->text_select_mode = true;
							}
						}
						else
							state->text_select_mode = false;
						
						f32 rel_cursor_x = context->cursor_fpos.x - text_p.x;
						if(rel_cursor_x < 0)
							rel_cursor_x = 0;
						
						u32 click_p = Max(u32(0), Min((u32)Round(rel_cursor_x / char_width), view_limit));
						
						
						state->write_cursor_position = Min(state->view_offset + click_p, str->lenght);
						
						state->click_p = click_p;
						state->mouse_press_down_point = state->write_cursor_position;
						state->mouse_hold_time = time + state->mouse_hold_delay;
					}
					
					
					if(mouse_is_down)
					{						
						if(state->mouse_hold_time != 0)
						{
							// mouse drag selection!
							
							f32 rel_cursor_x = context->cursor_fpos.x - text_p.x;
							if(rel_cursor_x < 0)
								rel_cursor_x = 0;
							
							u32 click_p = Max(u32(0), Min((u32)Round(rel_cursor_x / char_width), view_limit));
							
							if(time >= state->mouse_hold_time || click_p != state->click_p)
							{
								state->click_p = click_p;
								state->mouse_hold_time = time + state->mouse_hold_delay;
								state->write_cursor_position = Min(state->view_offset + click_p, str->lenght);
								state->text_select_start_point = state->mouse_press_down_point;
								state->text_select_mode = true;								
							}
						}
					}
					else
					{
						state->mouse_hold_time = 0;
					}
					
					
					if(str->lenght >= view_limit)
					{
						v2f shrink_bar_dim = bar_dim - v2f{handle_width / 2, 0};
						Rect shrink_bar = Create_Rect_Center(bar_center, shrink_bar_dim);	
						
						if(mouse_pressed_down && Is_Point_Inside_Rect(context->cursor_fpos, bar_rect))
						{
							state->handel_drag_mode = true;
							state->text_select_mode = false;
						}
						
						if(state->handel_drag_mode)
						{
							if(mouse_is_down)
							{
								state->flicker_start_time = time + state->flicker_delay;
								
								f32 rel_shrink_bar_x = context->cursor_fpos.x - shrink_bar.min.x;
								
								f32 percent = Clamp_Zero_To_One(rel_shrink_bar_x / shrink_bar_dim.x);
								
								// +1 here to allow scrolling 1 "slot" into the white space at the end.
								// (Lookahead)
								state->view_offset = (u32)Round(f32(str->lenght - view_limit + 1) * percent);
								
		
								if(state->write_cursor_position < state->view_offset + 1)
								{
									state->write_cursor_position = state->view_offset + 1;
								}
								
								else if(state->write_cursor_position > state->view_offset + view_limit - 1)
								{
									state->write_cursor_position = state->view_offset + view_limit - 1;
								}
							}
							else
								state->handel_drag_mode = false;
						}
					}
				}		
				
				text_select_mode = state->text_select_mode;
				text_select_start_point = state->text_select_start_point;
				
				if(wcp != state->write_cursor_position)
					state->flicker_start_time = time + state->flicker_delay;
				
				// View offset snapping! v2.0
				{
					if(str->lenght < view_limit || state->write_cursor_position == 0)
					{
						state->view_offset = 0;
					}
					
					else if(state->write_cursor_position == str->lenght)
					{
						state->view_offset = str->lenght - view_limit + 1;
					}
					
					else if(state->view_offset + view_limit > str->lenght + 1)
					{
						state->view_offset = str->lenght + 1 - view_limit;
					}
					
					else if(state->write_cursor_position > state->view_offset + view_limit - 1)
					{
						state->view_offset = state->write_cursor_position - view_limit + 1;
					}
					
					else if(state->write_cursor_position < state->view_offset + 1)
					{
						state->view_offset = state->write_cursor_position - 1;
					}
				}
			}
		}
		else
		{
			if(GUI_On_Release_Action(context, cursor_on_selection, &state->is_pressed_down))
			{
				// NOTE: Marikas idea!
				if(str->lenght > 0)
				{
					state->text_select_mode = true;
					state->text_select_start_point = 0;
				}
				
				state->is_active = true;
				state->write_cursor_position = str->lenght;
				state->flicker_start_time = context->platform->Get_Time_Stamp() + state->flicker_delay;
			}
		}
		
		view_offset = state->view_offset;
		
		write_cursor_position = state->write_cursor_position;
		
		if(state->is_pressed_down)
			outline_color = theme->down_color;
		else
			outline_color = theme->selected_color;
	}

	
	// Draw -------------------------------------------------------------------------------------------
	if(Rects_Overlap(p.rect, context->canvas_rect))
	{		
		Draw_Filled_Rect_With_Outline(
			context->canvas, 
			p.rect, 
			theme->background_color,
			theme->outline_thickness, 
			outline_color);
		
		// If the text can fit in the box, move it down so it's centered.
		if(str->lenght < view_limit)
			text_p.y = p.pos.y - char_height / 2;
		
		if(text_select_mode)
		{
			i32 p0 = (i32)write_cursor_position;
			i32 p1 = (i32)text_select_start_point;
			
			if(p1 != p0)
			{
				// p0 is forced to be less than p1.
				if(p0 > p1)
					Swap(&p0, &p1);
				
				p0 = Max(i32(0), i32(p0 - view_offset));
				p1 = Min(i32(view_limit), i32(p1 - view_offset));
				p1 -= p0;
				
				if(p1 != 0)
				{
					v2f selection_pos = {text_p.x + p0 * char_width, text_p.y};
					v2f selection_dim = {p1 * char_width, char_height};
					
					Rect select_rect = Create_Rect_Min_Dim(selection_pos, selection_dim);
					
					Draw_Filled_Rect(context->canvas, select_rect, theme->title_color);					
				}
			}
		}
		
		if(str->lenght > 0)
		{	
			u32 view_lenght = Min(str->lenght - view_offset, view_limit);
	
			Draw_Text(
				context->canvas, 
				Create_String_View(str, view_offset, view_lenght),
				text_p, 
				theme->text_color, 
				&theme->font, 
				text_scale);
		}
		
		if(draw_cursor)
		{
			v2f cursor_p = text_p;
			
			cursor_p.x += f32(write_cursor_position - view_offset) * char_width;
			
			Color cursor_color = (str->lenght < character_limit || character_limit == 0)? 
				theme->write_cursor_color : theme->write_cursor_limit_color;
			
			Draw_Vertical_Line(
				context->canvas, 
				cursor_p, 
				f32(font->char_height) * text_scale.y, 
				cursor_color);
		}
		
		// bar area.
		if(str->lenght >= view_limit)
		{
			Draw_Filled_Rect(context->canvas, bar_rect, theme->down_color);
			
			v2f handle_center = bar_center + v2f{-bar_dim.x / 2 + handle_width / 2, 0};
			
			f32 xoff = (f32)view_offset / f_lenght * bar_dim.x;
			handle_center.x += xoff;
			Rect handle_rect = Create_Rect_Center(handle_center, v2f{handle_width, bar_dim.y});
			
			Draw_Filled_Rect(context->canvas, handle_rect, theme->outline_color);
		}	
		
	}
	
	return result;
}


static void GUI_Do_ML_Input_Field(
	GUI_Context* context, 
	v2f* pos, 
	v2f* dim,
	String* str,
	u32 character_limit = 256,
	v2f text_scale = GUI_DEFAULT_TEXT_SCALE,
	bool (*character_check)(char*) = 0,
	GUI_Input_Acceleration_Behavior keyboard_acceleration = {0.2, 0.3, 30})
{
	Assert(GUI_Is_Context_Ready(context));
	
	// --------------------------------------------------------------------------
	
	GUI_Theme* theme = context->layout.theme;

	GUI_Placement p = GUI_Get_Placement(context, dim, pos);
	
	u32 id = GUI_Generate_ID(p.rect, __LINE__);
	
	bool cursor_on_selection = Is_Point_Inside_Rect(context->cursor_fpos, p.rect);
	bool is_selected = GUI_Is_Element_Selected(context, cursor_on_selection, id);
	
	// --------------------------------------------------------------------------
	
	Font* font = &theme->font;
	
	f32 outline_thickness = f32(theme->outline_thickness);
	f32 char_width = f32(font->char_width) * text_scale.x;
	f32 char_height = f32(font->char_height) * text_scale.y;
	
	v2f text_p = p.pos + v2f{1, -1} * outline_thickness + v2f{2,-2} +
		Hadamar_Product(v2f{-1, 1}, (p.dim / 2));
	
	text_p.y -= char_height;
	
	Color box_color = theme->background_color;
	Color text_color = theme->text_color;
	Color outline_color = theme->outline_color;
	
	u32 view_limit_x;
	// Calculate view limit.
	{
		// w: dif between text_p and p.pos twice.
		f32 w = (text_p.x - p.rect.min.x) * 2;
		view_limit_x = (u32)Floor((p.dim.x - w) / char_width);	
	}
	
	bool draw_cursor = false;
	u32 write_cursor_position = 0;
	
	if(is_selected)
	{
		GUI_SL_Input_Field_State* state = &context->selection_state.sl_input_field;
		
		if(state->is_active)
		{
			
			// Handle input
			bool mouse_pressed_down = context->actions[GUI_Menu_Actions::mouse].Is_Pressed();
			bool mouse_is_down = context->actions[GUI_Menu_Actions::mouse].Is_Down();
			
			if(context->actions[GUI_Menu_Actions::back].Is_Pressed() || 
				(!cursor_on_selection && mouse_pressed_down))
			{
				draw_cursor = false;
				state->is_active = false;
				state->view_offset = 0;
				
				Inverse_Bit_Mask(&context->flags, GUI_Context_Flags::cursor_mask_validation);
				
				
			}
			else
			{
				context->flags |= GUI_Context_Flags::cursor_mask_validation;
				context->flags |= GUI_Context_Flags::cursor_mask_enabled;	
				
				context->cursor_mask_area = context->canvas_rect;
				
				u32 wcp = state->write_cursor_position;
				
				if(GUI_Input_Field_Insert_Characters(
					context->platform, 
					state, 
					str, 
					character_limit, 
					wcp, 
					character_check))
				{
					str->insert_at(state->write_cursor_position, '\n');
					state->write_cursor_position += 1;
				}
				
			}
			
			draw_cursor = true;
			write_cursor_position = state->write_cursor_position;
		}
		else
		{
			if(GUI_On_Release_Action(context, cursor_on_selection, &state->is_pressed_down))
			{
				state->is_active = true;
				state->write_cursor_position = 0;
				state->flicker_start_time = context->platform->Get_Time_Stamp() + state->flicker_delay;
			}
			
		}
		
		if(state->is_pressed_down)
			outline_color = theme->down_color;
		else
			outline_color = theme->selected_color;
	}
	
	// Draw ---------------------------------------------------------------------
	if(Rects_Overlap(p.rect, context->canvas_rect))
	{	
		Draw_Filled_Rect_With_Outline(
			context->canvas, 
			p.rect, 
			box_color, 
			theme->outline_thickness, 
			outline_color);
		
		u32 line_start = 0;
		
		char* str_begin = str->buffer;
		char* str_end = str->buffer + str->lenght;
		int i = 0;
		for(char* c = str_begin; c < str_end; ++c)
		{
			u32 lenght = u32(c - str_begin) - line_start;
			
			if(*c == '\n' || lenght >= view_limit_x)
			{
				String_View view = {str->buffer + line_start, lenght};
				Draw_Text(context->canvas, view, text_p, text_color, font, text_scale);
				if(draw_cursor && write_cursor_position > line_start &&
					write_cursor_position < line_start + lenght)
				{
					draw_cursor = false;
					v2f cursor_p = text_p;
					cursor_p.x += f32(write_cursor_position - line_start) * char_width;
	
					Color cursor_color = (str->lenght < character_limit || character_limit == 0)? 
						theme->write_cursor_color : theme->write_cursor_limit_color;
					
					Draw_Vertical_Line(
						context->canvas, 
						cursor_p, 
						f32(font->char_height) * text_scale.y, 
						cursor_color);
				}
				
				
				text_p.y -= char_height;
				line_start += lenght + 1;
			}
			i++;
		}
		Assert(line_start <= str->lenght);
		
		if(line_start < str->lenght)
		{
			u32 lenght = u32(str->lenght - line_start);
			String_View view = {str->buffer + line_start, lenght};
			Draw_Text(context->canvas, view, text_p, text_color, font, text_scale);
			
			if(draw_cursor)
			{
				v2f cursor_p = text_p;
				cursor_p.x += f32(write_cursor_position - line_start) * char_width;

				Color cursor_color = (str->lenght < character_limit || character_limit == 0)? 
					theme->write_cursor_color : theme->write_cursor_limit_color;
				
				Draw_Vertical_Line(
					context->canvas, 
					cursor_p, 
					f32(font->char_height) * text_scale.y, 
					cursor_color);
			}
		}
	}
}


/*
if(draw_cursor)
{
	v2f cursor_p = text_p;
	
	cursor_p.x += f32(write_cursor_position - view_offset) * char_width;
	
	Color cursor_color = (str->lenght < character_limit || character_limit == 0)? 
		theme->write_cursor_color : theme->write_cursor_limit_color;
	
	Draw_Vertical_Line(
		context->canvas, 
		cursor_p, 
		f32(font->char_height) * text_scale.y, 
		cursor_color);
}
*/

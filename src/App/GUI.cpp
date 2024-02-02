
#pragma once


static inline v2f GUI_Get_Point_In_Placement_Space(
	GUI_Placement p, 
	v2f point, 
	GUI_Theme* theme)
{
	v2f internal_dim = p.dim - (v2f{} + f32(theme->outline_thickness)) * 2;
	
	f32 rel_cursor_x = Min(internal_dim.x, Max(0.f, point.x - p.rect.min.x));
	f32 rel_cursor_y = Min(internal_dim.y, Max(0.f, point.y - p.rect.min.y));
	
	v2f result = Hadamar_Division(v2f{rel_cursor_x, rel_cursor_y}, internal_dim); 
	
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


static inline void GUI_Set_Default_Menu_Actions(GUI_Context* context)
{
	Assert(context);
	
	Action* actions = &context->actions[0];
	
	*(actions + GUI_Menu_Actions::mouse) 	= Make_Action(Key_Code::MOUSE_LEFT, Button::NONE);
	*(actions + GUI_Menu_Actions::up) 		= Make_Action(Key_Code::UP, Button::DPAD_UP);
	*(actions + GUI_Menu_Actions::down) 	= Make_Action(Key_Code::DOWN, Button::DPAD_DOWN);
	*(actions + GUI_Menu_Actions::left) 	= Make_Action(Key_Code::LEFT, Button::DPAD_LEFT);
	*(actions + GUI_Menu_Actions::right) 	= Make_Action(Key_Code::RIGHT, Button::DPAD_RIGHT);
	*(actions + GUI_Menu_Actions::enter) 	= Make_Action(Key_Code::ENTER, Button::BUT_A);
	*(actions + GUI_Menu_Actions::back) 	= Make_Action(Key_Code::ESC, Button::BUT_X);
}


static inline Rect GUI_Get_Dropdown_Button_Open_Rect(v2f pos, v2f dim, u32 element_count)
{
	f32 half_dim_y = dim.y / 2;
	
	v2f center = v2f{pos.x, pos.y - (half_dim_y * f32(element_count) - half_dim_y)};
	
	v2f new_dim = v2f{dim.x, dim.y * f32(element_count)};
	
	Rect result = Create_Rect_Center(center, new_dim);
	return result;
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
			p = {f32(context->canvas->dim.x - padding), f32(context->canvas->dim.y - padding)};
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
		result.pos = *pos;
		
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
	context->cursor_mask_enabled = false;
	context->cursor_mask_validation = false;
	context->defered_render = GUI_Defered_Render_Type::none;
	context->selection_state = {};
	context->selected_id = 0;
}


static inline void GUI_Begin_Context(
	GUI_Context* context,  
	GUI_Anchor anchor,
	GUI_Build_Direction build_direction,
	GUI_Theme* theme,
	GUI_Link_Direction::Type ld = GUI_Link_Direction::up)
{
	Assert(context->widget_count == 0);
	
	context->layout = GUI_Layout();
	context->layout_stack_count = 0;
	context->layout.anchor = anchor;
	context->layout.build_direction = build_direction;
	context->layout.theme = theme;
	
	context->cursor_mask_validation = false;
	
	// Handle input.
	Action* actions = context->actions;
	Update_Actions(context->platform, actions, GUI_Menu_Actions::COUNT);
	
	if(Is_Flag_Set(context->platform->Get_Flags(), (u32)App_Flags::is_focused))
	{
		if(context->disable_kc_navigation)
		{
			context->disable_kc_navigation = false;
		}
		else
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
		
		context->cursor_position = context->platform->Get_Cursor_Position();
	}
}


static inline void GUI_End_Context(GUI_Context* context)
{
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
				u32 text_color;
				if(state->selected_element_idx == i)
				{
					u32 bg_color = theme->outline_color;
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
				else
					text_color = theme->outline_color;
				
				v2f text_p = GUI_Calc_Centered_Text_Position(text, state->text_scale, pos, font);
				Draw_Text(context->canvas, (u8*)text, text_p, text_color, font, state->text_scale);
				
				pos.y -= state->dim.y;
			}
		}break;
	}
	
	context->defered_render = GUI_Defered_Render_Type::none;
	
	if(!context->cursor_mask_validation)
	{
		context->cursor_mask_enabled = false;
		context->cursor_mask_area = {};
	}
	
	if(context->widget_count > 0 &&
		context->selected_index > context->widget_count - 1 &&
		context->widget_count < context->last_widget_count)
	{
		context->selected_index = context->widget_count - 1;
	}
	
	context->last_cursor_position = context->cursor_position;
	context->last_widget_count = context->widget_count;
	
	
	// Selection wrapping.
	{
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
	
	// Reset state.
	{
		context->widget_count = 0;	
		context->layout = {}; // Unnessery?
	}
}


// Generates a non zero id based on the position, dimensions and some mangle factor provided
// by the calling code. Usually that would be the line number from the __LINE__ macro.
// There is really no gurantee that there wont be ID collissions. This really is just hoping
// the program is correct, where randomly it might not be.
// TODO: upgrade to use a 64 bite nouse function to at least reduce the change of collissions.
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
	bool result = false;
	
	// This is the selected element.
	if(context->selected_index == context->widget_count)
	{
		// Buuut, it seems to be a different widget?
		if(id != context->selected_id)
		{
			GUI_Reset_Selection_State(context);
			context->selected_id = id;		
		}
		
		result = true;
	}
	else if(context->cursor_mask_enabled &&
		Is_Point_Inside_Rect(v2i::Cast<f32>(context->cursor_position), context->cursor_mask_area))
	{
		// Just a capture clause for cursor on a masked reagion.
	}
	// If the cursor has moved and it is on the element. Make it selected.
	else if((context->cursor_position != context->last_cursor_position && cursor_on_selection))
	{
		GUI_Reset_Selection_State(context);
		context->selected_index = context->widget_count;
		context->selected_id = id;
		
		result = true;
	}
	
	context->widget_count += 1; // Suprising side effect! But better to do it here than forget to do it ouside.
	return result;
}


static inline bool GUI_Selected_Element_Is_Pressed(GUI_Context* context, bool cursor_on_selection)
{
	bool result = context->actions[GUI_Menu_Actions::enter].Is_Pressed() ||
		(cursor_on_selection && context->actions[GUI_Menu_Actions::mouse].Is_Pressed());

	return result;
}


static inline bool GUI_Selected_Element_Is_Released(GUI_Context* context, bool cursor_on_selection)
{
	bool result = context->actions[GUI_Menu_Actions::enter].Is_Released() ||
		(cursor_on_selection && context->actions[GUI_Menu_Actions::mouse].Is_Released());

	return result;
}


static inline bool GUI_On_Release_Action(
	GUI_Context* context, bool cursor_on_selection, bool* is_pressed_down)
{
	if(GUI_Selected_Element_Is_Pressed(context, cursor_on_selection))
	{
		*is_pressed_down = true;
	}
	else if(*is_pressed_down && GUI_Selected_Element_Is_Released(context, cursor_on_selection))
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


static void GUI_Do_Spacing(
	GUI_Context* context, 
	v2f* dim)
{
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
}


static void GUI_Do_Text(
	GUI_Context* context, 
	v2f* pos, 
	char* text,
	GUI_Highlight highlight = GUI_Highlight_Nothing(),
	v2f text_scale = v2f{1.f, 1.f}, 
	bool is_title = false)
{
	// --------------------------------------------------------------------------
	
	GUI_Theme* theme = context->layout.theme;
	
	v2f dim = GUI_Tight_Fit_Text(text, text_scale, &theme->font);
	
	GUI_Placement p = GUI_Get_Placement(context, &dim, pos);

	v2f text_p = GUI_Calc_Centered_Text_Position(text, text_scale, p.pos, &theme->font);
	
	// --------------------------------------------------------------------------
	
	bool is_highlighted = highlight.highlight_count && 
		context->selected_index >= highlight.idx && 
		context->selected_index < highlight.idx + highlight.highlight_count;
	
	u32 color;
	if(is_title)
		color = theme->title_color;
	else
		color = is_highlighted? theme->selected_color : theme->outline_color;
	
	Draw_Text(context->canvas, (u8*)text, text_p, color, &theme->font, text_scale);
}





static bool GUI_Do_Button(
	GUI_Context* context, 
	v2f* pos, 
	v2f* dim, 
	char* text, 
	v2f text_scale = v2f{1, 1})
{
	// --------------------------------------------------------------------------
	
	GUI_Theme* theme = context->layout.theme;
	
	if(dim == &GUI_AUTO_FIT)
	{
		*dim = GUI_Tight_Fit_Text(text, text_scale, &theme->font) + theme->padding;
	}
	
	GUI_Placement p = GUI_Get_Placement(context, dim, pos);
	
	
	u32 id = GUI_Generate_ID(p.rect, __LINE__);
	
	bool cursor_on_selection = Is_Point_Inside_Rect(v2i::Cast<f32>(context->cursor_position), p.rect);
	bool is_selected = GUI_Is_Element_Selected(context, cursor_on_selection, id);
	
	// --------------------------------------------------------------------------
	
	bool result = false;
	
	u32 outline_color;
	
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
	else
	{
		outline_color = theme->outline_color;
	}
	
	// Draw
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
	Assert(step > 0);
	Assert(max > min);
	Assert(value);
	
	// --------------------------------------------------------------------------
	
	GUI_Placement p = GUI_Get_Placement(context, dim, pos);
	
	u32 id = GUI_Generate_ID(p.rect, __LINE__);
	
	v2f cursor_position = v2i::Cast<f32>(context->cursor_position);
	
	bool cursor_on_selection = Is_Point_Inside_Rect(cursor_position, p.rect);
	bool is_selected = GUI_Is_Element_Selected(context, cursor_on_selection, id);
	
	GUI_Theme* theme = context->layout.theme;
	
	// --------------------------------------------------------------------------
	
	f32 pre_val = *value;
	
	*value = Min(*value, max);
	*value = Max(*value, min);
	
	u32 bar_color; 
	
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
			f32 rel_cursor_x = Min(internal_width, Max(0.f, cursor_position.x - p.rect.min.x));
			
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


static bool GUI_Do_Checkbox(GUI_Context* context, v2f* pos, v2f* dim, bool* value)
{
	// --------------------------------------------------------------------------
	
	GUI_Placement p = GUI_Get_Placement(context, dim, pos);
	
	u32 id = GUI_Generate_ID(p.rect, __LINE__);
	
	v2f cursor_position = v2i::Cast<f32>(context->cursor_position);
	
	bool cursor_on_selection = Is_Point_Inside_Rect(cursor_position, p.rect);
	bool is_selected = GUI_Is_Element_Selected(context, cursor_on_selection, id);
	
	GUI_Theme* theme = context->layout.theme;
	
	// --------------------------------------------------------------------------
	
	bool pre_val = *value;
	
	
	u32 outline_color;
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
	else
	{
		outline_color = theme->outline_color;
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
	v2f text_scale = v2f{1.f, 1.f})
{
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
	u32 id = GUI_Generate_ID(p.rect, __LINE__);
	
	v2f cursor_position = v2i::Cast<f32>(context->cursor_position);
	
	bool cursor_on_selection = Is_Point_Inside_Rect(cursor_position, p.rect);
	bool is_selected = GUI_Is_Element_Selected(context, cursor_on_selection, id);
	
	// --------------------------------------------------------------------------
	
	u32 outline_color;
	
	u32 result = 0;
	
	Rect open_rect = GUI_Get_Dropdown_Button_Open_Rect(p.pos, p.dim, element_count);
	
	if(is_selected)
	{
		GUI_Dropdown_Button_State* state = &context->selection_state.dropdown_button;
		
		bool cursor_is_in_open_rect = Is_Point_Inside_Rect(v2i::Cast<f32>(context->cursor_position), open_rect);
		
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
				context->disable_kc_navigation = true;
				
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
						if(f32(context->cursor_position.y) > level)
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
			}			
		}		
		
		// Re-check if the box is open or not. May have changed in the above code block.
		if(state->is_open)
		{
			context->defered_render = GUI_Defered_Render_Type::dropdown_button;
			
			context->cursor_mask_area = open_rect;
			context->cursor_mask_enabled = true;
			context->cursor_mask_validation = true;
			
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
	else
	{
		outline_color = theme->outline_color;
	}
	
	// Draw closed mode.
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


//TODO: Should only take a width, as height is determined by the font and text scale.
static bool GUI_Do_SL_Input_Field(
	GUI_Context* context, 
	v2f* pos, 
	v2f* dim,
	String* str,
	u32 character_limit = 0,
	v2f text_scale = v2f{1.f, 1.f},
	bool (*character_check)(char*) = 0)
{
	static constexpr f32 min_handle_width = 3.f;
	
	// --------------------------------------------------------------------------
	
	GUI_Placement p = GUI_Get_Placement(context, dim, pos);
	
	u32 id = GUI_Generate_ID(p.rect, __LINE__);
	
	v2f cursor_position = v2i::Cast<f32>(context->cursor_position);
	
	bool cursor_on_selection = Is_Point_Inside_Rect(cursor_position, p.rect);
	bool is_selected = GUI_Is_Element_Selected(context, cursor_on_selection, id);
	
	GUI_Theme* theme = context->layout.theme;
	Font* font = &theme->font;
	
	// --------------------------------------------------------------------------
	
	u32 outline_color = theme->outline_color;
	u32 view_offset = 0;
	u32 view_limit; // Amount of charactes that can fit inside the text area.
	
	v2f text_p = p.pos + v2f{1, -1} * f32(theme->outline_thickness) + v2f{2,-2} +
		Hadamar_Product(v2f{-1, 1}, (p.dim / 2));
	text_p.y -= text_scale.y * f32(font->char_height);
	
	f32 char_width = f32(font->char_width) * text_scale.x;
	f32 char_height = f32(font->char_height) * text_scale.y;
	// Calculate view limit.
	{
		f32 w = (text_p.x - (p.pos - p.dim / 2).x) * 2;
		view_limit = (u32)Round_To_Signed_Int32((p.dim.x - w) / char_width);	
	}
	
	v2f bar_dim;
	v2f bar_center;
	Rect bar_rect;
	
	{
		f32 outline_thickness = f32(theme->outline_thickness);
		f32 bar_height = Ceil(font->char_height * text_scale.y) * 0.33f;
		f32 bar_offset_y = p.dim.y / 2 - bar_height / 2 - outline_thickness;
		bar_dim = { p.dim.x - outline_thickness * 2, bar_height };
		bar_center = p.pos - v2f{0, bar_offset_y};
		bar_rect = Create_Rect_Center(bar_center, bar_dim);
	}
	
	f32 f_lenght = f32(str->lenght) + 1.f;
	f32 handle_width = Max(bar_dim.x * view_limit / f_lenght, min_handle_width);
	
	bool draw_cursor = false; // I don't think state needs to keep track of this.
	u32 write_cursor_position = 0;
	
	bool text_select_mode = false;
	u32 text_select_start_point = 0;
	
	if(is_selected)
	{
		GUI_SL_Input_Field_State* state = &context->selection_state.sl_input_field;
		
		draw_cursor = state->draw_cursor && state->is_active;
		
		bool shift_down = context->platform->Get_Keyboard_Key_Down(Key_Code::LSHIFT) || 
			context->platform->Get_Keyboard_Key_Down(Key_Code::LSHIFT);
		
		// Handle input
		bool mouse_pressed_down = context->actions[GUI_Menu_Actions::mouse].Is_Pressed();
		
		if(state->is_active)
		{
			if(context->actions[GUI_Menu_Actions::back].Is_Pressed() || 
				(!cursor_on_selection && mouse_pressed_down))
			{
				draw_cursor = false;
				state->is_active = false;
				state->view_offset = 0;
				context->cursor_mask_validation = false;
				context->cursor_mask_enabled = false;
				context->cursor_position = v2i{-1, -1};
			}
			else
			{
				context->cursor_mask_validation = true;
				context->cursor_mask_enabled = true;
				context->cursor_mask_area = Create_Rect_Min_Dim(v2f{0,0}, v2u::Cast<f32>(context->canvas->dim));
				
				u32 wcp = state->write_cursor_position;
				
				Char_Array typing_info = context->platform->Get_Typing_Information();
	
				for(u32 i = 0; i < typing_info.count; ++i)
				{
					char c = typing_info.buffer[i];
					
					if(c == '\b'/*BACKSPACE*/)
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
							continue;
						}
						
						if(str->lenght > 0)
						{
							state->write_cursor_position -= 1;
							if(state->write_cursor_position > wcp)
								state->write_cursor_position = 0;
							
							str->remove_at(state->write_cursor_position);
						}
					}
					else if(c >= 32 && c <= 127 && 
						(character_limit == 0 || str->lenght < character_limit) &&
						(!character_check || (character_check && character_check(&c))))
					{
						if(state->text_select_mode) 
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
						}
						
						str->insert_at(state->write_cursor_position, c);
						state->write_cursor_position += 1;
					}
				}
				
				f_lenght = f32(str->lenght) + 1.f;
				handle_width = Max(bar_dim.x * view_limit / f_lenght, min_handle_width);
				
				f64 time = context->platform->Get_Time_Stamp();
				state->draw_cursor = !(time > state->flicker_start_time) || (u64)time % 2;
				
				//TODO: Provede a way for the user to set these.
				GUI_Input_Acceleration_Behavior accel;
				accel.input_speed_up_time = state->input_speed_up_time;
				accel.input_delay_time = state->input_delay;
				accel.max_speed_up_factor = state->max_speed_up_factor;
				
				Action* left = context->actions + GUI_Menu_Actions::left;
				Action* right = context->actions + GUI_Menu_Actions::right;
				
				if(left->Is_Pressed() || right->Is_Pressed())
				{
					state->input_start_time = 0;
					state->next_input_time = 0;
				}	
				
				// Arrowkey/controller input 
				while(left->Is_Down() &&
					GUI_Accelerated_Tick(&accel, time, &state->input_start_time, &state->next_input_time))
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
					
					
					state->write_cursor_position -= 1;
					
					if(state->write_cursor_position > wcp)
					{
						if(context->actions[GUI_Menu_Actions::left].Is_Pressed())
							state->write_cursor_position = str->lenght;	
						
						else
							state->write_cursor_position = wcp;
					}
				}
				
				
				while(right->Is_Down() &&
					GUI_Accelerated_Tick(&accel, time, &state->input_start_time, &state->next_input_time))
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
				if(str->lenght)
				{
					bool mouse_y_on_text_area = 
						(cursor_position.y >= text_p.y && cursor_position.y < text_p.y + char_height);
					
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
						
						f32 rel_cursor_x = cursor_position.x - text_p.x;
						u32 click_p = Max(u32(0), Min((u32)Round(rel_cursor_x / char_width), view_limit));
						
						state->write_cursor_position = Min(state->view_offset + click_p, str->lenght);
					}
					
					if(str->lenght >= view_limit)
					{
						v2f shrink_bar_dim = bar_dim - v2f{handle_width / 2, 0};
						Rect shrink_bar = Create_Rect_Center(bar_center, shrink_bar_dim);	
						
						if(mouse_pressed_down && Is_Point_Inside_Rect(cursor_position, bar_rect))
						{
							state->handel_drag_mode = true;
							state->text_select_mode = false;
						}
						
						if(state->handel_drag_mode)
						{
							if(context->actions[GUI_Menu_Actions::mouse].Is_Down())
							{
								state->flicker_start_time = time + state->flicker_delay;
								
								f32 rel_shrink_bar_x = cursor_position.x - shrink_bar.min.x;
								f32 percent = Clamp_Zero_To_One(rel_shrink_bar_x / shrink_bar_dim.x);
							
								state->view_offset = (u32)Round(f32(str->lenght - view_limit + 1) * percent);
	
								if(state->write_cursor_position < state->view_offset + 1)
									state->write_cursor_position = state->view_offset + 1;
								
								else if(state->write_cursor_position > state->view_offset + view_limit - 1)
									state->write_cursor_position = state->view_offset + view_limit - 1;
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
				
				
				// View offset handling.
				{
					// Yes, trying to wrap ones head around all of these conditionals is madness.
					// Therefore some helpfull comments! Hopefylly they aren't outdated!
					// Also the check order of these matters, for the logic it self and to prevent
					// tests that would underflow and crash if done in the wrong circumstance.
					
					bool underflow_protection = state->write_cursor_position > state->view_offset;
		
					// When the string can fit into a view, reset the view bar.
					if(str->lenght < view_limit)
					{
						state->view_offset = 0;
					}
					
					// Scroll the view leftwards when the right hand side of the view port is, farthen than
					// there is text to show. Or in other words prevent whitespace when looking past the string. 
					else if(state->write_cursor_position >= state->view_offset + 1 &&
						state->view_offset + view_limit + 1 > str->lenght)
					{
						state->view_offset = str->lenght - view_limit + 1;
					}
					
					// When the write cursor is beyond the right side of view, 
					// scroll the view to the right keep it inside.
					else if(underflow_protection &&
						state->write_cursor_position - state->view_offset > view_limit - 1)
					{
						state->view_offset = state->write_cursor_position - (view_limit - 1);
					}
					
					// Used as an underflow protection on top of the obvious.
					else if(state->write_cursor_position == 0)
					{
						state->view_offset = 0;
					}
					
					// When the write cursor is beyond the left side of view, 
					// scroll the view to the right left it inside.
					else if(state->write_cursor_position <= state->view_offset + 1)
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

	
	// Draw
	{		
		Draw_Filled_Rect_With_Outline(
			context->canvas, 
			p.rect, 
			theme->background_color,
			theme->outline_thickness, 
			outline_color);
		
		// If the text can fit in the box, move it down so it's centered.
		if(str->lenght < view_limit)
			text_p.y = p.pos.y - (font->char_height * text_scale.y) / 2;
		
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
			
				v2f selection_pos = {text_p.x + p0 * char_width, text_p.y};
				v2f selection_dim = {p1 * char_width, char_height};
				
				Rect select_rect = Create_Rect_Min_Dim(selection_pos, selection_dim);
				
				Draw_Filled_Rect(context->canvas, select_rect, theme->title_color);
			}
		}
		
		if(str->lenght > 0)
		{	
			u32 view_lenght = Min(str->lenght - view_offset, view_limit);
	
			Draw_Text(
				context->canvas, 
				Create_String_View(str, view_offset, view_lenght),
				text_p, 
				outline_color, 
				&theme->font, 
				text_scale);
		}
		
		// CONSIDER: Draw the cursor with different collor when at the maximum character limit.
		if(draw_cursor)
		{
			v2f cursor_p = text_p;
			
			cursor_p.x += f32(write_cursor_position - view_offset) * f32(font->char_width);
			
			Draw_Vertical_Line(
				context->canvas, 
				cursor_p, 
				f32(font->char_height) * text_scale.y, 
				theme->outline_color);					
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
	
	return false;
}
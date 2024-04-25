
#pragma once

#define SSE


static inline Color Pack_Color(v3f color)
{
	color.r = Min(255.f, Max(color.r, 0.f));
	color.g = Min(255.f, Max(color.g, 0.f));
	color.b = Min(255.f, Max(color.b, 0.f));
	
	return Make_Color(u8(color.r), u8(color.g), u8(color.b));
}


static inline v2f Get_Middle(Canvas* canvas)
{
	v2f result = v2u::Cast<f32>(canvas->dim) * 0.5f;
	return result;
}


static inline v3f Unpack_Color(Color color)
{
	v3f result;
	result.b = f32(color.blue);//f32(color & 0xFF);
	result.g = f32(color.green);//f32((color >> 8) & 0xFF);
	result.r = f32(color.red);//f32((color >> 16) & 0xFF);
	
	return result;
}


static inline v4f Unpack_Color_With_Alpha(Color color)
{
	v4f result;
	
	result.b = f32(color.blue);
	result.g = f32(color.green);
	result.r = f32(color.red);
	result.a = f32(color.alpha); //f32((color >> 24) & 0xFF);
	
	return result;
}


static inline bool Init_Canvas(Canvas* canvas, u32* buffer, v2u dim)
{
	Assert(canvas);
	//Assert(buffer); buffer can be null if the window size is 0.
	
	canvas->buffer = buffer;
	canvas->dim = dim;
	
	return buffer != 0;
}


static inline bool Is_Point_On_Canvas(Canvas* canvas, v2i p)
{
	Assert(canvas);
	
	v2u up = v2i::Cast<u32>(p);
	bool result = up.x < canvas->dim.x && up.y < canvas->dim.y;
	return result;
}


static inline bool Get_Pixel_Idx(Canvas* canvas, v2i p, u32* out_idx)
{
	Assert(canvas);
	
	v2u up = v2i::Cast<u32>(p);
	
	if(up.x < canvas->dim.x && up.y < canvas->dim.y)
	{
		u32 idx = Get_Pixel_Idx_HZ(canvas, p);
		if(out_idx)
			*out_idx = idx;
		
		return true;
	}
	
	return false;
}


static inline void Set_Pixel(Canvas* canvas, v2i p, Color color)
{
	Assert(canvas);
	
	u32 pixel_idx;
	if(Get_Pixel_Idx(canvas, p, &pixel_idx))
		*(canvas->buffer + pixel_idx) = color.as_u32;
	
}


static inline void Set_Pixel_Idx(Canvas* canvas, u32 idx, Color color)
{
	Assert(canvas);
	
	u32 canvas_pixel_count = canvas->dim.x * canvas->dim.y;
	
	if(idx < canvas_pixel_count)
		*(canvas->buffer + idx) = color.as_u32;
}


static inline u32 Get_Pixel_Idx_HZ(Canvas* canvas, v2i p)
{
	Assert(canvas);
	Assert(Is_Point_On_Canvas(canvas, p));
	u32 result = canvas->dim.x * p.y + p.x;
	
	return result;
}


static inline void Set_Pixel_HZ(Canvas* canvas, v2i p, Color color)
{
	Assert(canvas);
	u32 pixel_idx = Get_Pixel_Idx_HZ(canvas, p);
	
	*(canvas->buffer + pixel_idx) = color.as_u32;
}


static inline void Set_Pixel_Idx_HZ(Canvas* canvas, u32 idx, Color color)
{
	Assert(canvas);
	Assert(idx < canvas->dim.x * canvas->dim.y);
	*(canvas->buffer + idx) = color.as_u32;
}


static inline Color Get_Pixel_HZ(Canvas* canvas, v2i p)
{
	Assert(Is_Point_On_Canvas(canvas, p));
	u32 c = *((u32*)(canvas->buffer + Get_Pixel_Idx_HZ(canvas, p)));
	Color result = {c};
	return result;
}


// TODO: This doesn't work with sub canvases.
static void Clear_Canvas(Canvas* canvas, Color color)
{
	Start_Scope_Timer(clear_canvas);
	
	Assert(canvas);
	
	u32 canvas_area = canvas->dim.x * canvas->dim.y;
	
	for(u32 i = 0; i < canvas_area; ++i)
		Set_Pixel_Idx_HZ(canvas, i, color);
}


static inline void Blend_Pixel_With_Color(Canvas* canvas, v2i p, v3f color, f32 fraction)
{
	v3f buffer_color = Unpack_Color(Get_Pixel_HZ(canvas, p));
	v3f final_color = Lerp(buffer_color, color, fraction);
	
	Color c = Make_Color((u8)final_color.r, (u8)final_color.g, (u8)final_color.b);
	Set_Pixel_HZ(canvas, p, c);
}


static inline void Draw_Rect_Ribbon(
	Canvas* canvas, 
	Rect rect, 
	v2f ceil_min, 
	v2f floor_min, 
	v2f floor_max, 
	Color color32)
{
	v3f color = Unpack_Color(color32);
	
	// Left ribbon
	v2f inv_min_fraction = 1.f - (rect.min - floor_min);
	v2f max_fraction = rect.max - floor_max;
	
	bool has_inv_min_fraction_x = inv_min_fraction.x > 0 && inv_min_fraction.x < 1;
	bool has_inv_min_fraction_y = inv_min_fraction.y > 0 && inv_min_fraction.y < 1;
	bool has_max_fraction_x = max_fraction.x > 0 && max_fraction.x < 1;
	bool has_max_fraction_y = max_fraction.y > 0 && max_fraction.y < 1;
	
	if(has_inv_min_fraction_x)
		for(i32 y = (i32)ceil_min.y; y < (i32)floor_max.y; ++y)
			Blend_Pixel_With_Color(canvas, v2i{(i32)floor_min.x, y}, color, inv_min_fraction.x);
		
	// Bottom ribbon
	if(has_inv_min_fraction_y)
		for(i32 x = (i32)ceil_min.x; x < (i32)floor_max.x; ++x)
			Blend_Pixel_With_Color(canvas, v2i{x, (i32)floor_min.y}, color, inv_min_fraction.y);

	
	// Right ribbon
	if(has_max_fraction_x)
		for(i32 y = (i32)ceil_min.y; y < (i32)floor_max.y; ++y)
			Blend_Pixel_With_Color(canvas, v2i{(i32)floor_max.x, y}, color, max_fraction.x);
		
	
	// Top ribbon
	if(has_max_fraction_y)
		for(i32 x = (i32)ceil_min.x; x < (i32)floor_max.x; ++x)
			Blend_Pixel_With_Color(canvas, v2i{x, (i32)floor_max.y}, color, max_fraction.y);
		
	// Ribbon corners
	
	if(has_inv_min_fraction_x && has_inv_min_fraction_y)
	{
		v2i p = v2i{(i32)floor_min.x, (i32)floor_min.y};
		f32 subpixel_area = inv_min_fraction.x * inv_min_fraction.y;
		Blend_Pixel_With_Color(canvas, p, color, subpixel_area);
	}
	
	if(has_max_fraction_x && has_inv_min_fraction_y)
	{
		v2i p = v2i{(i32)floor_max.x, (i32)floor_min.y};
		f32 subpixel_area = max_fraction.x * inv_min_fraction.y;
		Blend_Pixel_With_Color(canvas, p, color, subpixel_area);
	}
	
	if(has_inv_min_fraction_x && has_max_fraction_y)
	{
		v2i p = v2i{(i32)floor_min.x, (i32)floor_max.y};
		f32 subpixel_area = inv_min_fraction.x * max_fraction.y;
		Blend_Pixel_With_Color(canvas, p, color, subpixel_area);
	}
	
	if(has_max_fraction_x && has_max_fraction_y)
	{
		v2i p = v2i{(i32)floor_max.x, (i32)floor_max.y};
		f32 subpixel_area = max_fraction.x * max_fraction.y;
		Blend_Pixel_With_Color(canvas, p, color, subpixel_area);
	}
}




//TODO: Call this something sane.
static inline bool Verify_Rect_(Canvas* canvas, Rect* rect)
{
	// Complitely off screen rects.
	{
		if(rect->min.x >= canvas->dim.x)
			return false;
		
		if(rect->min.y >= canvas->dim.y)
			return false;
		
		if(rect->max.x <= 0)
			return false;
		
		if(rect->max.y <= 0)
			return false;
	}
	
	// Partially on screen rects.
	{
		if(rect->min.x < 0)
			rect->min.x = 0;
		
		if(rect->min.y < 0)
			rect->min.y = 0;

		if(rect->max.x > canvas->dim.x)
			rect->max.x = (f32)canvas->dim.x;
		
		if(rect->max.y > canvas->dim.y)
			rect->max.y = (f32)canvas->dim.y;	
	}

	if(!Is_Rect_Valid(*rect))
		return false;
	
	return true;
}


static void Draw_Filled_Rect(Canvas* canvas, Rect rect, Color fill_color)
{
	Assert(canvas);
	//Start_Scope_Timer(render_filled_rect);

	if(Verify_Rect_(canvas, &rect))
	{
		v2f ceil_min = Ceil(rect.min); 
		v2f floor_min = Trunc(rect.min);
		v2f floor_max = Trunc(rect.max);
		
		// Shrinking internal Pixels.
		for(i32 y = (i32)ceil_min.y; y < (i32)floor_max.y; ++y)
			for(i32 x = (i32)ceil_min.x; x < (i32)floor_max.x; ++x)
				Set_Pixel_HZ(canvas, v2i{x, y}, fill_color);
		
		Draw_Rect_Ribbon(canvas, rect, ceil_min, floor_min, floor_max, fill_color);
	}	
}


static inline void Draw_Filled_Rect_Outline_Step(
	Canvas* canvas, 
	Rect rect, 
	f32 outlinef, 
	Color outline_color)
{
	v2f ceil_min = Ceil(rect.min); 
	v2f floor_min = Trunc(rect.min);
	v2f floor_max = Trunc(rect.max);
	
	// This looks funky, but trying to walk the memory in cache friendy way.
	
	// Bottom
	i32 y_max = (i32)Min(ceil_min.y + outlinef, (f32)canvas->dim.y - 1);
	for(i32 y = (i32)ceil_min.y; y < y_max; ++y)
		for(i32 x = (i32)ceil_min.x; x < (i32)floor_max.x; ++x)
			Set_Pixel_HZ(canvas, v2i{x, y}, outline_color);
	
	// Top
	i32 y_min = (i32)Max(floor_max.y - outlinef, 0.f);
	for(i32 y = y_min; y < (i32)floor_max.y; ++y)
		for(i32 x = (i32)ceil_min.x; x < (i32)floor_max.x; ++x)
			Set_Pixel_HZ(canvas, v2i{x, y}, outline_color);

	
	i32 x_min = (i32)Max(floor_max.x - outlinef, 0.f);
	i32 x_max = (i32)Min(ceil_min.x + outlinef, (f32)canvas->dim.x - 1);
	
	for(i32 y = y_max; y < y_min; ++y)
	{
		for(i32 x = (i32)ceil_min.x; x < x_max; ++x)
			Set_Pixel_HZ(canvas, v2i{x, y}, outline_color);
		
		for(i32 x = x_min; x < (i32)floor_max.x; ++x)
			Set_Pixel_HZ(canvas, v2i{x, y}, outline_color);
	}
	
	
	Draw_Rect_Ribbon(canvas, rect, ceil_min, floor_min, floor_max, outline_color);
}


static void Draw_Filled_Rect_With_Outline(
	Canvas* canvas, 
	Rect rect, 
	Color fill_color,
	u32 outline_thickness, 
	Color outline_color)
{
	//Start_Scope_Timer(render_filled_rect_with_outline);
	
	Assert(canvas);
	Assert(Is_Rect_Valid(rect));
	
	if(!outline_thickness) // CONSIDER: Assert instead?
	{
		Draw_Filled_Rect(canvas, rect, fill_color);
		return;
	}
	
	Rect rect2 = rect;
	
	if(!Verify_Rect_(canvas, &rect2))
		return;
	
	f32 outlinef = (f32)outline_thickness;
	Draw_Filled_Rect_Outline_Step(canvas, rect2, outlinef, outline_color);
	
	Rect internal = {rect.min + outlinef, rect.max - outlinef};
	
	Assert(Is_Rect_Valid(internal)); // CONSIDER: Just return out?
	
	Draw_Filled_Rect(canvas, internal, fill_color);
}


static void Draw_Percentile_Bar(
	Canvas* canvas, 
	Rect rect, 
	Color fill_color,
	u32 outline_thickness, 
	Color outline_color,
	Color bar_color,
	f32 fill_percent)
{
	Assert(Is_Rect_Valid(rect));
	
	fill_percent = Clamp_To_Barycentric(fill_percent);

	f32 outlinef = (f32)outline_thickness;
	
	{
		Rect rect2 = rect;
		
		if(outline_thickness && Verify_Rect_(canvas, &rect2))
		{
			Draw_Filled_Rect_Outline_Step(canvas, rect2, outlinef, outline_color);
		}
	}
	
	Rect internal = {rect.min + outlinef, rect.max - outlinef};
	f32 bar_width = internal.max.x - internal.min.x; 
	
	f32 x = internal.min.x + bar_width * fill_percent;
	
	Draw_Filled_Rect(canvas, internal, fill_color);
	
	if(fill_percent > 0)
	{
		Rect bar = {internal.min, {x, internal.max.y}};
		Draw_Filled_Rect(canvas, bar, bar_color);		
	}
}

// CONSIDER: Just delete these functions?
//TODO: Give this thing a real name.
static b32 Sample_Font(Font* font, i32 table_idx, u32 x, u32 y)
{
	i32 row = table_idx + (font->char_height - y);
	b32 is_pixel = (font->data_buffer[row] & 1 << x) > 0;
	
	return is_pixel;
}

// NOTE: Not used atm, logic was inlined.
static b32 Sample_Glyph(u8* glyph, u32 char_height, u32 x, u32 y)
{
	u32 row = char_height - y;
	b32 is_pixel = (*(glyph + row) & 1 << x) > 0;
	
	return is_pixel;
}


static void Draw_Text(
	Canvas* canvas,
	u8* text,
	v2f pos,
	Color color,
	Font* font,
	v2f scale)
{
	Assert(text);
	Assert(canvas);
	Assert(font);
	Assert(scale.x > 0);
	Assert(scale.y > 0);
	
	v2i cursor = {};

	v2f glyph_size = Hadamar_Product(v2f{f32(font->char_width), f32(font->char_height)}, scale);

	for(; *text; ++text, ++cursor.x)
	{
		u8 c = (*text);
		v2f p = Hadamar_Product(v2i::Cast<f32>(cursor), glyph_size) + pos;
		Draw_Glyph(canvas, p, scale, color, c, font);
	}
}


static void Draw_Text(
	Canvas* canvas,
	String_View text,
	v2f pos,
	Color color,
	Font* font,
	v2f scale)
{
	Assert(canvas);
	Assert(font);
	Assert(scale.x > 0);
	Assert(scale.y > 0);

	v2i cursor = {};

	v2f glyph_size = Hadamar_Product(v2f{f32(font->char_width), f32(font->char_height)}, scale);

	for(char* ptr = text.buffer; ptr < text.buffer + text.lenght; ++ptr, ++cursor.x)
	{
		u8 c = (*ptr);
		v2f p = Hadamar_Product(v2i::Cast<f32>(cursor), glyph_size) + pos;
		Draw_Glyph(canvas, p, scale, color, c, font);
	}
}

/*
u8 glyph = (*ptr);
i32 font_offset = (i32)glyph - 33;

if(glyph == '\n')
{
	cursor.x = -1;
	cursor.y += 1;
	continue;	
}

if(font_offset < 0 || font_offset > 93)
	continue;
*/


static void Draw_Glyph(
	Canvas* canvas,
	v2f pos,
	v2f scale,
	Color packed_color,
	char character,
	Font* font)
{	
	v2f floor_p = Floor(pos);
	
	v2f glyph_size = Hadamar_Product(v2f{f32(font->char_width), f32(font->char_height)}, scale);
	v2f ceil_p = Ceil(pos + glyph_size);
	v2i floor_int_p = v2f::Cast<i32>(floor_p);

	v2i max = {i32(ceil_p.x - floor_p.x), i32(ceil_p.y - floor_p.y)};

	// Internal pixels.
	i32 y_min = 1, y_max = max.y;
	i32 x_min = 1, x_max = max.x;
	
	i32 pixel_x_max = floor_int_p.x + x_max;
	if(pixel_x_max > i32(canvas->dim.x))
		x_max = x_max - (pixel_x_max - canvas->dim.x);

	i32 pixel_x_min = floor_int_p.x;
	if(pixel_x_min < 0)
		x_min = -pixel_x_min;	

	i32 pixel_y_max = floor_int_p.y + y_max;
	if(pixel_y_max > i32(canvas->dim.y))
		y_max = y_max - (pixel_y_max - canvas->dim.y);

	i32 pixel_y_min = floor_int_p.y;
	if(pixel_y_min < 0)
		y_min = -pixel_y_min;	

	// CONSIDER: Start timing after this point? Not much point in timing no work.
	if(y_min >= y_max || x_min >= x_max)
		return;
	
	Start_Scope_Timer(render_glyph);
	
	u8* glyph;
	if(character >= 33 && character <= 126)
	{
		static constexpr u32 font_offset = 33;
		glyph = font->data_buffer + ((character - font_offset) * font->char_height);
	}
	else
	{
		// This is fine for couple of supported special characters.
		switch(character)
		{
			case -28: //0xE4 -- ä
			{
				glyph = font->data_buffer_sc + (0 * font->char_height);
			}break;
			
			case -10: //0xF6 -- ö
			{
				glyph = font->data_buffer_sc + (1 * font->char_height);	
			}break;
			
			case -60: //0xC4 -- Ä
			{
				glyph = font->data_buffer_sc + (2 * font->char_height);
			}break;
			
			case -42: //0xD6 -- Ö
			{
				glyph = font->data_buffer_sc + (3 * font->char_height);
			}break;
			
			default:
				return;
		}
	}

	v3f unpacked_color = Unpack_Color(packed_color);
	
	v2f inv_scale = 1 / scale;
	v2f fraction = pos - floor_p;
	v2f inv_fraction = 1.f - fraction;
	
	f32 df = Componentwise_Mult(inv_fraction);
	f32 urf = Componentwise_Mult(fraction);
	f32 uf = Componentwise_Mult({inv_fraction.x, fraction.y});
	f32 rf = Componentwise_Mult({fraction.x, inv_fraction.y});
	
	#ifdef SSE // Going fast going wide.
	
	__m128i zeroes 		= _mm_set1_epi32(0);
	__m128i ones 		= _mm_set1_epi32(1);
	__m128 inv_scale_x 	= _mm_set1_ps(inv_scale.x);
	__m128 wide_df 		= _mm_set1_ps(df);
	__m128 wide_rf	 	= _mm_set1_ps(rf);
	__m128 wide_uf 		= _mm_set1_ps(uf);
	__m128 wide_urf 	= _mm_set1_ps(urf);
	
	for(i32 y = y_min; y < y_max; ++y)
	{ 
		u32 sample_y = u32(y * inv_scale.y);
		if(sample_y >= font->char_height)
			break;
		
		u32 sample_y_sub = u32((y - 1) * inv_scale.y);
		
		u32 row = (font->char_height - 1) - sample_y;
		u8 glyph_row = *(glyph + row);
		
		__m128i wide_glyph_row = _mm_set1_epi32(glyph_row);

		u32 row_sub = (font->char_height - 1) - sample_y_sub;
		u8 glyph_row_sub = *(glyph + row_sub);
		
		__m128i wide_glyph_row_sub = _mm_set1_epi32(glyph_row_sub);
		
		i32 pixel_y = floor_int_p.y + y;

		for(i32 x4 = x_min; x4 < x_max; x4 += 4)
		{
			f32 x0 = f32(x4);
			f32 x1 = f32(x4 + 1);
			f32 x2 = f32(x4 + 2);
			f32 x3 = f32(x4 + 3);
			
			f32 xs0 = f32(x4 - 1);
			
			#if 1
			
			__m128 fsample_x 		= _mm_set_ps(x3, x2, x1, x0);
			__m128 fsample_x_sub 	= _mm_set_ps(x2, x1, x0, xs0);	
			
			#else // Inverted load, kept here as a toggle as the effect is cool and maybe I'd like to use it at some point for demonic looking glyphs.
			
			__m128 fsample_x 		= _mm_set_ps(x0, x1, x2, x3);
			__m128 fsample_x_sub 	= _mm_set_ps(xs0, x0, x1, x2);
			
			#endif
			
			fsample_x 				= _mm_mul_ps(fsample_x, inv_scale_x);
			fsample_x_sub 			= _mm_mul_ps(fsample_x_sub, inv_scale_x);
			
			__m128i sample_x 		= _mm_cvttps_epi32(fsample_x); 						// Truncate f32 to u32.
			__m128i sample_x_sub 	= _mm_cvttps_epi32(fsample_x_sub);					// Truncate f32 to u32.
	
	
			__m128i sample_mask1 	= _mm_sllv_epi32(ones, sample_x);					// Shift left (variadic).
			__m128i sample_mask2 	= _mm_sllv_epi32(ones, sample_x_sub);				// Shift left (variadic).
	
	
			__m128i p_df 			= _mm_and_si128(wide_glyph_row, sample_mask1);		// Bitwise and.
			p_df 					= _mm_srlv_epi32(p_df, sample_x);					// Shift right (variadic).
			__m128 pixel_value_df	= _mm_cvtepi32_ps(p_df);							// Convert u32 to f32.
			
			
			__m128i p_rf 			= _mm_and_si128(wide_glyph_row, sample_mask2);		// Bitwise and.
			p_rf 					= _mm_srlv_epi32(p_rf, sample_x_sub);               // Shift right (variadic).
			__m128 pixel_value_rf	= _mm_cvtepi32_ps(p_rf);                            // Convert u32 to f32.
			
			
			__m128i p_uf 			= _mm_and_si128(wide_glyph_row_sub, sample_mask1);	// Bitwise and.
			p_uf 					= _mm_srlv_epi32(p_uf, sample_x);                   // Shift right (variadic).
			__m128 pixel_value_uf	= _mm_cvtepi32_ps(p_uf);                            // Convert u32 to f32.
			
			
			__m128i p_urf 			= _mm_and_si128(wide_glyph_row_sub, sample_mask2);	// Bitwise and.
			p_urf 					= _mm_srlv_epi32(p_urf, sample_x_sub);              // Shift right (variadic).
			__m128 pixel_value_urf	= _mm_cvtepi32_ps(p_urf);                           // Convert u32 to f32.
			
			
			__m128 tf[4] = {};
			tf[0] = _mm_mul_ps(wide_df, pixel_value_df);
			tf[1] = _mm_mul_ps(wide_rf, pixel_value_rf);
			tf[2] = _mm_mul_ps(wide_uf, pixel_value_uf);
			tf[3] = _mm_mul_ps(wide_urf, pixel_value_urf);
			
			__m128 color_sum_a = _mm_add_ps(tf[0], tf[1]);	
			__m128 color_sum_b = _mm_add_ps(tf[2], tf[3]);
			__m128 color_sum_t = _mm_add_ps(color_sum_a, color_sum_b);
			
			for(i32 p = 0; p < Array_Lenght(tf); ++p)
			{
				if(x4 + p >= x_max)
					break;
		
				f32 color_amount = (*((v4f*)&color_sum_t)).elements[p];
				if(color_amount > 0.01f)
				{
					v2i pixel_p = {floor_int_p.x + x4 + p, pixel_y};
					if(color_amount > 0.95f)
						Set_Pixel_HZ(canvas, pixel_p, packed_color);
					else
						Blend_Pixel_With_Color(canvas, pixel_p, unpacked_color, color_amount);
				}				
			}
		}
	}
	#else
		
	for(i32 y = y_min; y < y_max; ++y)
	{
		u32 sample_y = u32(y * inv_scale.y);
		if(sample_y >= font->char_height)
			break;
		
		u32 sample_y_sub = u32((y - 1) * inv_scale.y);
		
		u32 row = (font->char_height - 1) - sample_y;
		u8 glyph_row = *(glyph + row);
		
		u32 row_sub = (font->char_height - 1) - sample_y_sub;
		u8 glyph_row_sub = *(glyph + row_sub);
		
		i32 pixel_y = floor_int_p.y + y;
		for(i32 x = x_min; x < x_max; ++x)
		{
			u32 sample_x = u32(x * inv_scale.x);
			u32 sample_x_sub = u32((x - 1) * inv_scale.x);
			
			u32 left_shift1 = 1 << sample_x;
			u32 left_shift2 = 1 << sample_x_sub;
			
			v4f tf;
			tf.elements[0] = df  * 	((glyph_row 	& left_shift1) > 0);
			tf.elements[1] = rf  * 	((glyph_row 	& left_shift2) > 0);
			tf.elements[2] = uf  * 	((glyph_row_sub & left_shift1) > 0);
			tf.elements[3] = urf * 	((glyph_row_sub & left_shift2) > 0);
	
			f32 color_amount = Componentwise_Add(tf);
			if(color_amount > 0.01f)
			{
				v2i pixel_p = {floor_int_p.x + x, pixel_y};
				if(color_amount > 0.95f)
					Set_Pixel_HZ(canvas, pixel_p, packed_color);
				else
					Blend_Pixel_With_Color(canvas, pixel_p, unpacked_color, color_amount);
			}
		}
	}
	
	#endif
}


static void Draw_Vertical_Line(Canvas* canvas, v2f pos, f32 height, Color color)
{
	if(pos.x < 0 || pos.x >= canvas->dim.x)
		return;
	
	v2f floor_p = Floor(pos);
	v2f ceil_p = Ceil(pos);
	
	v2f fraction = pos - floor_p;
	v2f inv_fraction = 1 - fraction;
	
	v3f up_color = Unpack_Color(color);
	
	i32 y_start = (i32)floor_p.y;
	i32 y_end = (i32)(ceil_p.y + height) - 1;
	
	y_start = Max(y_start, 0);
	y_end = Min(y_end, i32(canvas->dim.y - 1));
	
	if(y_end <= y_start)
		return;
	
	if(fraction.x)
	{
		for(i32 y = y_start + 1; y <= y_end - 1; ++y)
		{
			Blend_Pixel_With_Color(canvas, v2i{i32(floor_p.x), y}, up_color, inv_fraction.x);
			Blend_Pixel_With_Color(canvas, v2i{i32(floor_p.x) + 1, y}, up_color, fraction.x);
		}
		
		if(fraction.y)
		{
			// Bottom pixels
			Blend_Pixel_With_Color(canvas, v2i{i32(floor_p.x), y_start}, up_color, inv_fraction.x * inv_fraction.y);
			Blend_Pixel_With_Color(canvas, v2i{i32(floor_p.x) + 1, y_start}, up_color, fraction.x * inv_fraction.y);
			
			// Bottom pixels
			Blend_Pixel_With_Color(canvas, v2i{i32(floor_p.x), y_end}, up_color, inv_fraction.x * fraction.y);
			Blend_Pixel_With_Color(canvas, v2i{i32(floor_p.x) + 1, y_end}, up_color, fraction.x * fraction.y);
		}
	}
	else if(fraction.y)
	{
		for(i32 y = y_start+ 1; y <= y_end - 1; ++y)
		{	
			Set_Pixel(canvas, v2i{i32(floor_p.x), y}, color);
		}
		
		Blend_Pixel_With_Color(canvas, v2i{i32(floor_p.x), y_start}, up_color, inv_fraction.y);
		Blend_Pixel_With_Color(canvas, v2i{i32(floor_p.x), y_end}, up_color, fraction.y);
	}
	else
	{
		for(i32 y = y_start; y <= y_end; ++y)
		{	
			Set_Pixel(canvas, v2i{i32(floor_p.x), y}, color);
		}
	}
}


static void Dim_Entire_Screen(Canvas* canvas, f32 s)
{
	Assert(s >= 0 && s < 1);
	Begin_Timing_Block(scale_pixel);
	
	#ifdef SSE
	u32 pixel_count = canvas->dim.x * canvas->dim.y;
	
	u32 mask_255 = 0xFF;
	
	__m128 wscale = _mm_set1_ps(s);
	__m128i wide_mask_255 = _mm_set1_epi32(mask_255);
	
	for(u32* pxl = canvas->buffer; pxl < canvas->buffer + pixel_count; pxl += 4)
	{
		__m128i wpck_pxl = _mm_load_si128((__m128i*)pxl);

		__m128i wc1 = _mm_and_si128(wpck_pxl, wide_mask_255);
		__m128i wc2 = _mm_and_si128(_mm_srli_epi32(wpck_pxl, 8), wide_mask_255);
		__m128i wc3 = _mm_and_si128(_mm_srli_epi32(wpck_pxl, 16), wide_mask_255);
		__m128i wc4 = _mm_and_si128(_mm_srli_epi32(wpck_pxl, 24), wide_mask_255);
		
		__m128 wc1_ps = _mm_cvtepi32_ps(wc1);
		__m128 wc2_ps = _mm_cvtepi32_ps(wc2);
		__m128 wc3_ps = _mm_cvtepi32_ps(wc3);
		__m128 wc4_ps = _mm_cvtepi32_ps(wc4);
		
		wc1_ps = _mm_mul_ps(wc1_ps, wscale);
		wc2_ps = _mm_mul_ps(wc2_ps, wscale);
		wc3_ps = _mm_mul_ps(wc3_ps, wscale);
		wc4_ps = _mm_mul_ps(wc4_ps, wscale);
		
		wc1 = _mm_cvttps_epi32(wc1_ps);
		wc2 = _mm_cvttps_epi32(wc2_ps);
		wc3 = _mm_cvttps_epi32(wc3_ps);
		wc4 = _mm_cvttps_epi32(wc4_ps);
		
		wc2 = _mm_slli_epi32(wc2, 8);
		wc3 = _mm_slli_epi32(wc3, 16);
		wc4 = _mm_slli_epi32(wc4, 24);
		
		__m128i wc12 = _mm_or_si128(wc1, wc2);
		__m128i wc34 = _mm_or_si128(wc3, wc4);
		
		__m128i result = _mm_or_si128(wc12, wc34);
		_mm_store_si128((__m128i*)pxl, result);
	}

	#else
	for(u32 y = 0; y < canvas->dim.y; ++y)
	{
		for(u32 x = 0; x < canvas->dim.x; ++x)
		{
			v2i p = v2i{i32(x), i32(y)};
			Color frame_buffer_color = Get_Pixel_HZ(canvas, p);
			
			v3f unpacked_color = Unpack_Color(frame_buffer_color);
			
			unpacked_color *= s;
			
			frame_buffer_color = Pack_Color(unpacked_color);
			
			Set_Pixel_HZ(canvas, p, frame_buffer_color);
		}
	}
	
	#endif
	
	End_Timing_Block(scale_pixel);
}
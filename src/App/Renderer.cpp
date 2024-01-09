
# pragma once


static inline v2f Get_Middle(Canvas* canvas)
{
	v2f result = v2u::Cast<f32>(canvas->dim) * 0.5f;
	return result;
}


static inline v3f Unpack_Color(u32 color)
{
	v3f result;
	result.b = f32(color & 0xFF);
	result.g = f32((color >> 8) & 0xFF);
	result.r = f32((color >> 16) & 0xFF);
	
	return result;
}


static inline v4f Unpack_Color_With_Alpha(u32 color)
{
	v4f result;
	result.b = f32(color & 0xFF);
	result.g = f32((color >> 8) & 0xFF);
	result.r = f32((color >> 16) & 0xFF);
	result.a = f32((color >> 24) & 0xFF);
	
	return result;
}


static inline void Init_Canvas(Canvas* canvas, u32* buffer, v2u dim)
{
	Assert(canvas);
	Assert(buffer);
	
	Assert(dim.x != 0);
	Assert(dim.y != 0);
	
	canvas->buffer = buffer;
	canvas->dim = dim;
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


static inline void Set_Pixel(Canvas* canvas, v2i p, u32 color)
{
	Assert(canvas);
	
	u32 pixel_idx;
	if(Get_Pixel_Idx(canvas, p, &pixel_idx))
		*(canvas->buffer + pixel_idx) = color;
	
}


static inline void Set_Pixel_Idx(Canvas* canvas, u32 idx, u32 color)
{
	Assert(canvas);
	
	u32 canvas_pixel_count = canvas->dim.x * canvas->dim.y;
	
	if(idx < canvas_pixel_count)
		*(canvas->buffer + idx) = color;
}


static inline u32 Get_Pixel_Idx_HZ(Canvas* canvas, v2i p)
{
	Assert(canvas);
	Assert(Is_Point_On_Canvas(canvas, p));
	u32 result = canvas->dim.x * p.y + p.x;
	
	return result;
}


static inline void Set_Pixel_HZ(Canvas* canvas, v2i p, u32 color)
{
	Assert(canvas);
	u32 pixel_idx = Get_Pixel_Idx_HZ(canvas, p);
	
	*(canvas->buffer + pixel_idx) = color;
}


static inline void Set_Pixel_Idx_HZ(Canvas* canvas, u32 idx, u32 color)
{
	Assert(canvas);
	*(canvas->buffer + idx) = color;
}


static inline u32 Get_Pixel_HZ(Canvas* canvas, v2i p)
{
	Assert(Is_Point_On_Canvas(canvas, p));
	u32 result = *((u32*)(canvas->buffer + Get_Pixel_Idx_HZ(canvas, p)));
	return result;
}


static void Clear_Canvas(Canvas* canvas, u32 color)
{
	Assert(canvas);
	
	u32 canvas_area = canvas->dim.x * canvas->dim.y;
	
	for(u32 i = 0; i < canvas_area; ++i)
		Set_Pixel_Idx_HZ(canvas, i, color);
}


static inline void Blend_Pixel_With_Color(Canvas* canvas, v2i p, v3f color, f32 fraction)
{
	v3f buffer_color = Unpack_Color(Get_Pixel_HZ(canvas, p));
	v3f final_color = Lerp(buffer_color, color, fraction);
	
	u32 c = Put_Color((u8)final_color.r, (u8)final_color.g, (u8)final_color.b);
	Set_Pixel_HZ(canvas, p, c);
}


static inline void Draw_Rect_Ribbon(
	Canvas* canvas, 
	Rect rect, 
	v2f ceil_min, 
	v2f floor_min, 
	v2f floor_max, 
	u32 color32)
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


static void Draw_Filled_Rect(Canvas* canvas, Rect rect, u32 fill_color)
{
	Assert(canvas);

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


static inline void Draw_Filled_Rect_Outline_Step(Canvas* canvas, Rect rect, f32 outlinef, u32 outline_color)
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
	u32 fill_color,
	u32 outline_thickness, 
	u32 outline_color)
{
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
	u32 fill_color,
	u32 outline_thickness, 
	u32 outline_color,
	u32 bar_color,
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


//TODO: Give this thing a real name and swap the param order so it takes x, y instead.
static b32 Sample_Font(Font* font, i32 table_idx, u32 x, u32 y)
{
	i32 row = table_idx + (font->char_height - y);
	b32 is_pixel = (font->data_buffer[row] & 1 << x) > 0;
	
	return is_pixel;
}


static void Draw_Text(
	Canvas* canvas,
	u8* text,
	v2f pos,
	u32 color,
	Font* font)
{
	Assert(canvas);
	Assert(font);
	
	v2f p = pos;
	
	v2f glyph_size = v2f{f32(font->char_width), f32(font->char_height)};

	v2f floor_p = Floor(p);
	
	v2f fraction = p - floor_p;
	v2f inv_fraction = 1.f - fraction;
	
	f32 df = Componentwise_Mult(inv_fraction);
	f32 urf = Componentwise_Mult(fraction);
	f32 uf = Componentwise_Mult({inv_fraction.x, fraction.y});
	f32 rf = Componentwise_Mult({fraction.x, inv_fraction.y});
	
	v3f c = Unpack_Color(color);
	
	for(; *text; ++text, p.x += glyph_size.x)
	{
		u8 glyph = (*text);
		i32 font_offset = (i32)glyph - 33;
		
		if(glyph == '\n')
		{
			p.x = pos.x - glyph_size.x;
			p.y -= glyph_size.y;
			continue;	
		}
		
		if(font_offset < 0 || font_offset > 93)
			continue;
		
		font_offset *= font->char_height;
		
		floor_p = Floor(p);
		
		for(u32 y = 1; y <= font->char_height; ++y)
			for(u32 x = 1; x <= font->char_width; ++x)
			{
				v2i pixel_p = v2f::Cast<i32>(floor_p);
				pixel_p.x += (i32)x;
				pixel_p.y += (i32)y;
				
				if(Is_Point_On_Canvas(canvas, pixel_p))
				{
					v4f tf;
					
					tf.elements[0] = df * Sample_Font(font, font_offset, x, y);
					tf.elements[1] = rf * Sample_Font(font, font_offset, x - 1, y);
					tf.elements[2] = uf * Sample_Font(font, font_offset, x, y - 1);
					tf.elements[3] = urf * Sample_Font(font, font_offset, x - 1, y - 1);
					
					f32 color_amount = Componentwise_Add(tf);
					
					if(color_amount > 0.01f)
					{
						if(color_amount > 0.95 && color_amount < 1.05)
							Set_Pixel_HZ(canvas, pixel_p, color);
						else
							Blend_Pixel_With_Color(canvas, pixel_p, c, color_amount);
					}
				}
			}
	}
}


static void Draw_Text(
	Canvas* canvas,
	u8* text,
	v2f pos,
	u32 color,
	Font* font,
	v2i scale)
{
	Draw_Text(
		canvas,
		text,
		pos,
		color,
		font,
		v2i::Cast<f32>(scale));
}


static void Draw_Text(
	Canvas* canvas,
	u8* text,
	v2f pos,
	u32 color,
	Font* font,
	v2f scale)
{
	Assert(canvas);
	Assert(font);
	Assert(scale.x > 0);
	Assert(scale.y > 0);
	
	#if 1
	if(scale == v2f{1.f, 1.f})
	{
		Draw_Text(canvas, text, pos, color, font);
		return;
	}
	#endif

	v2i cursor = {};

	v2f glyph_size = Hadamar(v2f{f32(font->char_width), f32(font->char_height)}, scale);

	for(; *text; ++text, ++cursor.x)
	{
		u8 glyph = (*text);
		i32 font_offset = (i32)glyph - 33;
		
		if(glyph == '\n')
		{
			cursor.x = -1;
			cursor.y += 1;
			continue;	
		}
		
		if(font_offset < 0 || font_offset > 93)
			continue;
		
		font_offset *= font->char_height;
		
		v2f p = Hadamar(v2i::Cast<f32>(cursor), glyph_size) + pos;
		Draw_Glypgh(canvas, p, scale, color, font_offset, font);
	}
}


static void Draw_Glypgh(
	Canvas* canvas,
	v2f pos,
	v2f scale,
	u32 packed_color,
	i32 font_offset,
	Font* font)
{
	v2f inv_scale = 1 / scale;
	v2f floor_p = Floor(pos);
	v2f fraction = pos - floor_p;
	v2f inv_fraction = 1.f - fraction;
	
	f32 df = Componentwise_Mult(inv_fraction);
	f32 urf = Componentwise_Mult(fraction);
	f32 uf = Componentwise_Mult({inv_fraction.x, fraction.y});
	f32 rf = Componentwise_Mult({fraction.x, inv_fraction.y});
	
	v2f glyph_size = Hadamar(v2f{f32(font->char_width), f32(font->char_height)}, scale);
	v2f ceil_p = Ceil(pos + glyph_size);
	v2i floor_int_p = v2f::Cast<i32>(floor_p);

	v2i max = {i32(ceil_p.x - floor_p.x), i32(ceil_p.y - floor_p.y)};

	v3f unpacked_color = Unpack_Color(packed_color);

	// Doing the left and right ribbon is "correct", 
	// but with this font those pixels are never set,
	// so disabling it doesn't effect text rendering.
	// AND with that, this code is unstested and,
	// therefore the correctness is unknown.
	#if 0
	// Bottom left pixel.
	{
		v2i pixel_p = {floor_int_p.x, floor_int_p.y};
		if(Is_Point_On_Canvas(canvas, pixel_p))
		{
			f32 color_amount = df * Sample_Font(font, font_offset, 0, 0);
			if(color_amount > 0.01)
			{
				if(color_amount > 0.95 && color_amount < 1.05)
					Set_Pixel_HZ(canvas, pixel_p, packed_color);
				else
					Blend_Pixel_With_Color(canvas, pixel_p, unpacked_color, color_amount);
			}
		}
	}
	
	// Bottom ribbon
	for(i32 x = 1; x <= max.x; ++x)
	{
		v2i pixel_p = {floor_int_p.x + x, floor_int_p.y};
		
		if(Is_Point_On_Canvas(canvas, pixel_p))
		{
			u32 sample_x = u32(x * inv_scale.x);
			u32 sample_x_sub = u32((x - 1) * inv_scale.x);
			
			v2f tf;
			tf.elements[0] = df * Sample_Font(font, font_offset, sample_x, 0);
			tf.elements[1] = uf * Sample_Font(font, font_offset, sample_x_sub, 0);
			
			f32 color_amount = Componentwise_Add(tf);
			if(color_amount > 0.01)
			{
				if(color_amount > 0.95 && color_amount < 1.05)
					Set_Pixel_HZ(canvas, pixel_p, packed_color);
				else
					Blend_Pixel_With_Color(canvas, pixel_p, unpacked_color, color_amount);
			}
		}
	}
	
	// Left ribbon.
	for(i32 y = 1; y <= max.y; ++y)
	{
		v2i pixel_p = {floor_int_p.x, floor_int_p.y + y};
		
		if(Is_Point_On_Canvas(canvas, pixel_p))
		{
			u32 sample_y = u32(y * inv_scale.y);
			u32 sample_y_sub = u32((y - 1) * inv_scale.y);
			
			v2f tf;
			tf.elements[0] = df * Sample_Font(font, font_offset, 0, sample_y);
			tf.elements[1] = uf * Sample_Font(font, font_offset, 0, sample_y_sub);
			
			f32 color_amount = Componentwise_Add(tf);
			if(color_amount > 0.01)
			{
				if(color_amount > 0.95 && color_amount < 1.05)
					Set_Pixel_HZ(canvas, pixel_p, packed_color);
				else
					Blend_Pixel_With_Color(canvas, pixel_p, unpacked_color, color_amount);
			}
		}
	}
	#endif
	
	// Internal pixels.
	for(i32 y = 1; y <= max.y; ++y)
		for(i32 x = 1; x <= max.x; ++x)
		{
			v2i pixel_p = {floor_int_p.x + x, floor_int_p.y + y};
			if(Is_Point_On_Canvas(canvas, pixel_p))
			{	
				u32 sample_x = u32(x * inv_scale.x);
				u32 sample_y = u32(y * inv_scale.y);
				u32 sample_x_sub = u32((x - 1) * inv_scale.x);
				u32 sample_y_sub = u32((y - 1) * inv_scale.y);
				
				v4f tf;
				tf.elements[0] = df * Sample_Font(font, font_offset, sample_x, sample_y);
				tf.elements[1] = rf * Sample_Font(font, font_offset, sample_x_sub, sample_y);
				tf.elements[2] = uf * Sample_Font(font, font_offset, sample_x, sample_y_sub);
				tf.elements[3] = urf * Sample_Font(font, font_offset, sample_x_sub, sample_y_sub);
				
				f32 color_amount = Componentwise_Add(tf);
				if(color_amount > 0.01)
				{
					if(color_amount > 0.95 && color_amount < 1.05)
						Set_Pixel_HZ(canvas, pixel_p, packed_color);
					else
						Blend_Pixel_With_Color(canvas, pixel_p, unpacked_color, color_amount);
				}
			}
		}
}


static void Draw_Vertical_Line(Canvas* canvas, v2f pos, f32 height, u32 color)
{
	v2f floor_p = Floor(pos);
	v2f ceil_p = Ceil(pos);
	
	v2f fraction = pos - floor_p;
	v2f inv_fraction = 1 - fraction;
	
	v3f up_color = Unpack_Color(color);
	
	i32 y_start = (i32)floor_p.y;
	i32 y_end = (i32)(ceil_p.y + height);
	
	if(fraction.x)
	{
		for(i32 y = y_start + 1; y <= i32(ceil_p.y + height - 1); ++y)
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
		for(i32 y = (i32)floor_p.y + 1; y <= ceil_p.y + height - 1; ++y)
		{	
			Set_Pixel(canvas, v2i{i32(floor_p.x), y}, color);
		}
		
		Blend_Pixel_With_Color(canvas, v2i{i32(floor_p.x), y_start}, up_color, inv_fraction.y);
		Blend_Pixel_With_Color(canvas, v2i{i32(floor_p.x), y_end}, up_color, fraction.y);
	}
	else
	{
		for(i32 y = (i32)floor_p.y; y <= ceil_p.y + height; ++y)
		{	
			Set_Pixel(canvas, v2i{i32(floor_p.x), y}, color);
		}
	}
}
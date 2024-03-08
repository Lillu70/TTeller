
# pragma once

struct Font
{
	u32 char_width = 0;
	u32 char_height = 0;
	u8* data_buffer = 0;
};


struct Canvas
{
	v2u dim = {};
	u32* buffer = 0;
};


static inline v2f Get_Middle(Canvas* canvas);


static inline v3f Unpack_Color(Color color);
static inline v4f Unpack_Color_With_Alpha(Color color);


static inline bool Init_Canvas(Canvas* canvas, u32* buffer, v2u dim);

// Does bounds checking.
static inline bool Get_Pixel_Idx(Canvas* canvas, v2i p, u32* out_idx);

static inline void Set_Pixel(Canvas* canvas, v2i p, Color color);
static inline void Set_Pixel_Idx(Canvas* canvas, u32 idx, Color color);


// Does not do bounds checking.
static inline u32 Get_Pixel_Idx_HZ(Canvas* canvas, v2i p);

static inline void Set_Pixel_HZ(Canvas* canvas, v2i p, Color color);
static inline void Set_Pixel_Idx_HZ(Canvas* canvas, u32 idx, Color color);


static inline void Blend_Pixel_With_Color(Canvas* canvas, v2i p, v3f color, f32 fraction);

static inline Color Get_Pixel_HZ(Canvas* canvas, v2i p);


static void Clear_Canvas(Canvas* canvas, Color color);


static void Draw_Vertical_Line(Canvas* canvas, v2f pos, f32 height, Color color);


static void Draw_Filled_Rect(Canvas* canvas, Rect rect, Color fill_color);


static void Draw_Filled_Rect_With_Outline(
	Canvas* canvas, 
	Rect rect,
	Color fill_color,
	u32 outline_thickness, 
	Color outline_color);


static void Draw_Percentile_Bar(
	Canvas* canvas, 
	Rect rect, 
	Color fill_color,
	u32 outline_thickness, 
	Color outline_color,
	Color bar_color,
	f32 fill_percent);


static void Draw_Text(
	Canvas* canvas,
	u8* text,
	v2f pos,
	Color color,
	Font* font);


static void Draw_Text(
	Canvas* canvas,
	u8* text,
	v2f pos,
	Color color,
	Font* font,
	v2i scale);


static void Draw_Text(
	Canvas* canvas,
	u8* text,
	v2f pos,
	Color color,
	Font* font,
	v2f scale);
	
	
static void Draw_Text(
	Canvas* canvas,
	String_View text,
	v2f pos,
	Color color,
	Font* font,
	v2f scale);	


static void Draw_Glyph(
	Canvas* canvas,
	v2f pos,
	v2f scale,
	Color color,
	i32 font_offset,
	Font* font);

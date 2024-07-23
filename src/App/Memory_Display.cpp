
#pragma once


static inline void Draw_Memory_Display(Paged_General_Allocator* pga, Canvas* canvas)
{
    Color background_color = Make_Color(80, 80, 80);
    Color free_color = Make_Color(80, 160, 80);
    Color used_color = Make_Color(160, 40, 40);
    
    Clear_Canvas(canvas, background_color);
    
    f32 strip_height = 25.f;
    f32 padding = 5.f;    
    v2f strip_dim = v2f{f32(canvas->dim.x) - 300, strip_height};
    
    for(u32 i = 0; i < pga->active_page_count; ++i)
    {
        v2f pos = v2f{50, strip_height + (strip_height + padding) * i};
        
        {
            Rect region = Create_Rect_Min_Dim(pos, strip_dim);
            Draw_Filled_Rect(canvas, region, used_color);
        }
        
        General_Allocator* alloc = pga->page_table + i;
        Free_Block* block = alloc->first_block;
        
        f32 w = f32(alloc->full_capacity);
        
        while(block)
        {
            u32 block_begin = u32((u8*)block - alloc->memory);
            u32 block_end = block_begin + block->size;
            
            // barry start and end
            f32 fb = f32(block_begin) / w;
            f32 fe = f32(block_end) / w;
            
            Assert(fb <= 1.f);
            Assert(fe <= 1.f);
            
            // start point
            v2f p = v2f{pos.x + fb * strip_dim.x, pos.y};
            
            v2f d = v2f{(fe - fb) * strip_dim.x, strip_dim.y};
            
            Rect region = Create_Rect_Min_Dim(p, d);
            Draw_Filled_Rect(canvas, region, free_color);
            
            block = block->next_block;
        }
    }
}
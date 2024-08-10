

// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#pragma once

static inline void* REALLY_BAD_REALLOC(void* p, u32 newsz)
{
    if(p)
    {
        void* result = s_allocator.push(newsz);
        u32 oldsz = *((u32*)p - 2);
        Mem_Copy(result, p, oldsz);
        s_allocator.free(p);
        return result;
    }
    return s_allocator.push(newsz);
}


static inline  void REALLY_BAD_FREE(void* p)
{
    if(p) 
        s_allocator.free(p);
}

#define STBI_FAILURE_USERMSG
#define STBI_NO_HDR
#define STBI_NO_GIF
#define STBI_NO_STDIO
#define STBI_MALLOC(sz)           s_allocator.push(u32(sz))
#define STBI_REALLOC(p,newsz)     REALLY_BAD_REALLOC(p, newsz)
#define STBI_FREE(p)              REALLY_BAD_FREE(p)

#define STBI_ASSERT(x)              Assert(x)
#define STB_IMAGE_IMPLEMENTATION
#include "..\Vendor\stb_image.h"


static bool Load_Image_Raw(
    Image* out_img, 
    char* path, 
    Platform_Calltable* platform)
{
    *out_img = {};
    
    s_scrach_buffer.clear();
    
    bool result = false;
    
    u32 buffer_size = 0;
    if(platform->Get_File_Size((const char*)path, &buffer_size))
    {
        if(s_scrach_buffer.capacity < buffer_size)
            s_scrach_buffer.init(platform, buffer_size);
        
        u8* file_buffer = (u8*)s_scrach_buffer.push(buffer_size);
        
        if(platform->Read_File(path, file_buffer, buffer_size))
        {
            i32 out_x;
            i32 out_y;
            i32 out_c;
            
            u8* img_buffer = (u8*)stbi_load_from_memory(
                file_buffer, 
                buffer_size, 
                &out_x, 
                &out_y, 
                &out_c, 
                sizeof(Color));
            
            if(img_buffer)
            {
                *out_img = {img_buffer, v2i{out_x, out_y}};
                result = true;
            }
        }
    }
    
    return result;
}


static bool Load_Image(
    Image* out_img, 
    char* path, 
    Platform_Calltable* platform)
{
    if(Load_Image_Raw(out_img, path, platform))
    {
        Convert_From_RGB_To_Color_And_Flip_Y(out_img);
        return true;
    }
    
    return false;
}
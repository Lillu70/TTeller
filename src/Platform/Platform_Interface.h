
#pragma once

// TODO: Make these pre-shifted.
// Add comon bit-field procedures into utility.
enum class App_Flags : u32
{
	is_running = 0,
	wants_to_exit,
	is_focused,
	is_fullscreen,
	cursor_is_visible,
	window_has_resized
};


namespace Create_Directory_Result
{
	enum T : u8
	{
		path_not_found = 0,
		success = 1,
		file_already_exists = 2,	
	};
}


struct Char_Array
{
	char buffer[8];
	u8 count;
};


struct Pixel_Buffer
{
	i32 width = 0, height = 0;
	u32* memory = 0;
};


struct Platform_Calltable
{
	u32  (*Get_Window_Width)() = 0;
	
	u32  (*Get_Window_Height)() = 0;
	
	Pixel_Buffer (*Get_Pixel_Buffer)() = 0;
	
	i32  (*Get_Pixel_Buffer_Width)() = 0;
	
	i32  (*Get_Pixel_Buffer_Height)() = 0;
	
	u32* (*Resize_Pixel_Buffer)(i32 width, i32 height) = 0;
	
	u32  (*Get_Flags)() = 0;
	
	void (*Set_Flag)(App_Flags flag, bool val) = 0;
	
	bool (*Get_Keyboard_Key_Down)(Key_Code) = 0;
	
	f32  (*Get_Scroll_Wheel_Delta)() = 0;
	
	Char_Array (*Get_Typing_Information)() = 0;
	
	Controller_State(*Get_Controller_State)(i32 controler_idx) = 0;
	
	v2i (*Get_Cursor_Position)() = 0;
	
	f64 (*Get_Time_Stamp)() = 0;
	
	u64 (*Get_CPU_Time_Stamp)() = 0;
	
	u64 (*Get_Frame_Count)() = 0;
	
	bool (*Write_File)(const char* path, u8* buffer, u32 buffer_size) = 0;
	
	bool (*Get_File_Size)(const char*, u32*) = 0;
	
	bool (*Read_File)(const char*, u8*, u32) = 0;
	
	Create_Directory_Result::T (*Create_Directory)(char* path) = 0;
	
	Dynamic_Array<String>* (*Search_Directory_For_Maching_Names)(char* file_name, Allocator_Shell* allocator) = 0;
	
	char* (*Get_Clipboard_Data_As_Text)() = 0;
	
	void  (*Set_Clipboard_Data_As_Text)(char*, u32) = 0;
	
	void* (*Allocate_Memory)(u32 amount, u32* out_amount) = 0;
	
	void  (*Free_Memory)(void* memory) = 0;
};
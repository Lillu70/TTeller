
#pragma once


enum class App_Flags
{
	is_running = 0,
	wants_to_exit,
	is_focused,
	is_fullscreen,
	cursor_is_visible,
};


struct Char_Array
{
	char buffer[8];
	u8 count;
};


struct Platform_Calltable
{
	u32  (*Get_Window_Width)() = 0;
	
	u32  (*Get_Window_Height)() = 0;
	
	u32* (*Get_Pixel_Buffer)() = 0;
	
	i32  (*Get_Pixel_Buffer_Width)() = 0;
	
	i32  (*Get_Pixel_Buffer_Height)() = 0;
	
	u32* (*Resize_Pixel_Buffer)(i32 width, i32 height) = 0;
	
	u32  (*Get_Flags)() = 0;
	
	void (*Set_Flag)(App_Flags flag, bool val) = 0;
	
	bool (*Get_Keyboard_Key_Down)(Key_Code) = 0;
	
	Char_Array (*Get_Typing_Information)() = 0;
	
	Controller_State(*Get_Controller_State)(i32 controler_idx) = 0;
	
	v2i (*Get_Cursor_Position)() = 0;
	
	f64 (*Get_Time_Stamp)() = 0;
	
	u64 (*Get_CPU_Time_Stamp)() = 0;
	
	u64 (*Get_Frame_Count)() = 0;
	
	bool (*Write_File)(const char*, u8*, u32) = 0;
	
	bool (*Get_File_Size)(const char*, u32*) = 0;
	
	bool (*Read_File)(const char*, u8*, u32) = 0;
};
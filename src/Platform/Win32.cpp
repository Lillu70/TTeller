
#pragma once

#include "Win32.h"


static Win32_App s_app;
static Win32_Bitmap s_bitmap;

typedef DWORD WINAPI x_input_get_state(DWORD dwUserIndex, XINPUT_STATE* state);
static x_input_get_state* XInputGetState_;
#define XInputGetState XInputGetState_

typedef DWORD WINAPI x_input_set_state(DWORD dwUserIndex, XINPUT_VIBRATION* vibrarion);
static x_input_set_state* XInputSetState_;
#define XInputSetState XInputSetState_


static LRESULT Win32_Message_Handler(HWND win_handle, UINT message, WPARAM wparam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (message)
	{
		case WM_CHAR:
		{
			char c = (char)wparam;
			
			u32 lenght = Array_Lenght(s_app.typing_information.buffer);
			if(s_app.typing_information.count < lenght)
			{
				s_app.typing_information.buffer[s_app.typing_information.count] = c;
				s_app.typing_information.count += 1;
			}
			
		}break;
		
		case WM_SIZING:
		case WM_SIZE:
		{
			RECT client_rect;
			GetClientRect(win_handle, &client_rect);
			
			s_app.window_width = client_rect.right - client_rect.left;
			s_app.window_height = client_rect.bottom - client_rect.top;
			s_app.force_update_surface = true;
		}break;
		
		
		case WM_DESTROY:
		{
			s_app.flags |= (1 << (u32)App_Flags::wants_to_exit);
		}break;
		
		
		case WM_CLOSE:
		{
			s_app.flags |= (1 << (u32)App_Flags::wants_to_exit);
		}break;
		
		
		case WM_ACTIVATEAPP:
		{
			BOOL active = (BOOL)wparam;
			
			u8 n = (u8)App_Flags::is_focused;
			s_app.flags = (s_app.flags & ~(1 << n)) | (active << n);
			s_app.force_update_surface = true;
		}break;
		
		
		case WM_SYSKEYDOWN:
		{
			s_app.force_update_surface = true;
			DefWindowProcA(win_handle, message, wparam, lParam);
		}break;
		
		
		
		case WM_SETCURSOR:
		{
			/*
			if(LOWORD(lParam) == HTCLIENT)
				SetCursor(NULL);
			*/
			result = DefWindowProcA(win_handle, message, wparam, lParam);
		}break;
		
		default:
		{
			result = DefWindowProcA(win_handle, message, wparam, lParam);
		}break;
	}

	return  result;
}


static u32* Win32_Resize_Pixel_Buffer(i32 new_width, i32 new_height)
{
	Assert(new_width >= 0);
	Assert(new_height >= 0);
	
	s_bitmap.height = new_height;
	s_bitmap.width = new_width;
	
	if (s_bitmap.memory)
		VirtualFree(s_bitmap.memory, 0, MEM_RELEASE);
	
	s_bitmap.info.bmiHeader.biWidth = new_width;
	s_bitmap.info.bmiHeader.biHeight = new_height;
	
	u64 pixel_count = (u64)new_width * new_height;
	u64 bitmap_memory_size = pixel_count * s_bitmap.bytes_per_pixel;
	
	s_bitmap.memory = (u32*)VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	
	return s_bitmap.memory;
}


static void Win32_Init(
	HINSTANCE instance, 
	u32 window_width, 
	u32 window_height, 
	u32 window_pos_x, 
	u32 window_pos_y, 
	const char* window_title,
	u32 app_memory_size)
{
	// Create window
	{
		s_app.instance = instance;
		WNDCLASSA win_class{};
		//window_class.hIcon = ;
		win_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		win_class.lpfnWndProc = Win32_Message_Handler;
		win_class.hInstance = instance;
		win_class.lpszClassName = "TW_CLASS";
		win_class.hCursor = LoadCursorA(0, (LPCSTR)IDC_CROSS);
		
		if (!RegisterClassA(&win_class))
		Terminate;
		
		
		//Windows behavior here is whacky.
		//https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa
		
		if (window_pos_x != CW_USEDEFAULT && window_pos_y == CW_USEDEFAULT) window_pos_y = 0;
		if (window_pos_y != CW_USEDEFAULT && window_pos_x == CW_USEDEFAULT) window_pos_x = 0;
		
		if (window_height != CW_USEDEFAULT && window_width == CW_USEDEFAULT) window_width = window_height;
		if (window_width  != CW_USEDEFAULT && window_height == CW_USEDEFAULT) window_height = window_width;
		
		DWORD win_style_flags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
		
		s_app.window = CreateWindowExA
		(
			0,
			win_class.lpszClassName,
			window_title, 
			win_style_flags,
			window_pos_x, 
			window_pos_y,
			window_width, 
			window_height,
			0, 
			0, 
			instance, 
			0
		);
		
		{            
			RECT window_rect;
			BOOL get_window_rect_result = GetWindowRect(s_app.window, &window_rect);
			
			RECT client_rect;
			BOOL get_client_rect_result = GetClientRect(s_app.window, &client_rect);
			
			i32 window_width = window_rect.right - window_rect.left;
			i32 window_height = window_rect.bottom - window_rect.top;
			
			i32 border_width = (window_width - client_rect.right);
			i32 border_height = (window_height - client_rect.bottom);
			
			u32 scale = 2;
			
			i32 new_width = window_width * scale + border_width;
			i32 new_height = window_height * scale + border_height;
			
			SetWindowPos(s_app.window, 0, window_rect.left, window_rect.top, new_width, new_height, 0);
		}
		
		
		if(!s_app.window)
			Terminate;
		
		s_app.dc = GetDC(s_app.window);
		
		
		s_app.flags |= 1 << (u32)App_Flags::is_running;
		s_app.flags |= 1 << (u32)App_Flags::is_focused;
		s_app.flags |= 1 << (u32)App_Flags::cursor_is_visible;
	
	}
	
	// Fill in bitmap info.
	{
		s_bitmap.info.bmiHeader.biSize = sizeof(s_bitmap.info.bmiHeader);
		s_bitmap.info.bmiHeader.biPlanes = 1;
		s_bitmap.info.bmiHeader.biBitCount = sizeof(i32) * 8;
		s_bitmap.info.bmiHeader.biCompression = BI_RGB;
		
		Win32_Resize_Pixel_Buffer(window_width, window_height);
	}
	
	
	// Try to load XInput
	{ 
		HMODULE XInput_lib;
		if ((XInput_lib = LoadLibraryA("xinput1_4.dll")) || (XInput_lib = LoadLibraryA("xinput1_3.dll")))
		{
			XInputGetState = (x_input_get_state*)GetProcAddress(XInput_lib, "XInputGetState");
			XInputSetState = (x_input_set_state*)GetProcAddress(XInput_lib, "XInputSetState");
		}
	}
	
	//Init time counters
	{
		LARGE_INTEGER perf_freg;
		QueryPerformanceFrequency(&perf_freg);
		s_app.timer_counter_freg = perf_freg.QuadPart;
		QueryPerformanceCounter(&s_app.last_time_counter);
	}
	
	s_app.game_state_memory = VirtualAlloc(0, app_memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}


static void Win32_Flush_Events()
{
	s_app.typing_information = {};
	
	// The message dispatch loop.
	{	
		MSG message;
		while (BOOL message_result = PeekMessage(&message, 0, 0, 0, PM_REMOVE) != 0)
		{
			if (message_result == -1) Terminate;
				TranslateMessage(&message);
				DispatchMessageA(&message);
		}
	}
	
	// Update controller state.
	{
		for (i32 i = 0; i < s_app.max_controllers; i++)
		{
			s_app.controller_state[i].m_prev = s_app.controller_state[i].m_curr;
			
			if (XInputGetState)
			{
				XINPUT_STATE xinput_state;
				f64 time_stamp = Win32_Get_Time_Stamp();
				if(time_stamp >= s_app.next_controler_test_time)
				{
					//TODO: Actually figure out the types and get rid of auto.
					auto result = XInputGetState(i, &xinput_state);
					if (result == ERROR_SUCCESS)
					{
						auto& pad = xinput_state.Gamepad;
						s_app.controller_state[i].m_curr.button_states = 0;
						for (u16 button_idx = 0; button_idx < (u16)Button::BUTTON_COUNT; ++button_idx)
							if (s_controller_map[button_idx] & pad.wButtons)
								s_app.controller_state[i].m_curr.button_states = s_app.controller_state[i].m_curr.button_states | (1 << button_idx);
						
						static constexpr u32 negative_max_range = 32768;
						static constexpr u32 positive_max_range = 32767;
						
						if (pad.sThumbLX < 0)
							s_app.controller_state[i].m_curr.l_thumb_x = (f32)pad.sThumbLX / negative_max_range;
						else
							s_app.controller_state[i].m_curr.l_thumb_x = (f32)pad.sThumbLX / positive_max_range;
						
						if (pad.sThumbLY < 0)
							s_app.controller_state[i].m_curr.l_thumb_y = (f32)pad.sThumbLY / negative_max_range;
						else
							s_app.controller_state[i].m_curr.l_thumb_y = (f32)pad.sThumbLY / positive_max_range;
						
						if (pad.sThumbRX < 0)
							s_app.controller_state[i].m_curr.r_thumb_x = (f32)pad.sThumbRX / negative_max_range;
						else
							s_app.controller_state[i].m_curr.r_thumb_x = (f32)pad.sThumbRX / positive_max_range;
						
						if (pad.sThumbRY < 0)
							s_app.controller_state[i].m_curr.r_thumb_y = (f32)pad.sThumbRY / negative_max_range;
						else
							s_app.controller_state[i].m_curr.r_thumb_y = (f32)pad.sThumbRY / positive_max_range;
						
						static constexpr u32 trigger_max_range = 255;
						
						s_app.controller_state[i].m_curr.l_trig = (f32)pad.bLeftTrigger / trigger_max_range;
						s_app.controller_state[i].m_curr.r_trig = (f32)pad.bRightTrigger / trigger_max_range;
					}
					else if(result == ERROR_DEVICE_NOT_CONNECTED)
					{
						s_app.controller_state[i] = {0};
						s_app.next_controler_test_time = time_stamp + 3;
					}
				
				}
			}
		}
	}
	
	s_app.frame_counter += 1;
}


static void Win32_Update_Surface(bool update_from_game)
{
	if(s_bitmap.memory && (update_from_game || s_app.force_update_surface))
	{
		s_app.force_update_surface = false;
		StretchDIBits
		(
			s_app.dc,
			0, 0,
			s_app.window_width,
			s_app.window_height,
			0, 0,
			s_bitmap.width,
			s_bitmap.height,
			s_bitmap.memory,
			&s_bitmap.info,
			DIB_RGB_COLORS,
			SRCCOPY
		);
	}
}


static void Win32_Set_Flag(App_Flags flag, bool value)
{
	u32 iflag = (u32)flag;
	
	switch(flag)
	{
		case App_Flags::is_running:
		{
			s_app.flags = (s_app.flags & ~(1 << iflag)) | (value << iflag);
		}break;
		
		
		case App_Flags::wants_to_exit:
		{
			s_app.flags = (s_app.flags & ~(1 << iflag)) | (value << iflag);
		}break;
		
		
		case App_Flags::is_focused:
		{
			//TODO: Make so this can be used to unfocus the window...
			// if that's even possible.
			Terminate;
		}break;
		
		case App_Flags::cursor_is_visible:
		{
			// TODO: Only hide the cursor in the client area of the window.
			bool cursor_visiblity = (s_app.flags & (1 << (u32)App_Flags::cursor_is_visible)) > 0;
			if(cursor_visiblity == value)
				return;
			
			s_app.flags ^= (1 << (u32)App_Flags::cursor_is_visible);
			
			i32 c = ShowCursor(value);
			int a = 0;
		}break;
		
		case App_Flags::is_fullscreen:
		{
			bool is_fullscreen = (s_app.flags & (1 << iflag)) > 0;
			if(is_fullscreen == value)
			return;
			
			s_app.force_update_surface = true;
			DWORD dwStyle = GetWindowLongA(s_app.window, GWL_STYLE);
			
			if(value) // Set fullscreen to true.
			{
				
				if (!GetWindowPlacement(s_app.window, &s_app.window_placement))
				Terminate;
				
				MONITORINFO mi = { sizeof(mi) };
				
				if (!GetMonitorInfoA(MonitorFromWindow(s_app.window, MONITOR_DEFAULTTOPRIMARY), &mi))
				Terminate;
				
				SetWindowLongA(s_app.window, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos
				(
					s_app.window,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOOWNERZORDER | SWP_FRAMECHANGED
				);
			}
			
			// Set fullscreen to false.
			else
			{
				SetWindowLongA(s_app.window, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
				SetWindowPlacement(s_app.window, &s_app.window_placement);
				SetWindowPos(s_app.window, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
			}
			
			s_app.flags ^= (1 << iflag);
		}break;
		
		default:
			//Unhandeled flag
			Terminate;
	}
}

static f64 Win32_Update_Frame_Time()
{
	//Update timing info.    
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);
	
	i64 counter_elapsed_time = current_time.QuadPart - s_app.last_time_counter.QuadPart;
	s_app.last_time_counter = current_time;	
	
	return f64(counter_elapsed_time) / s_app.timer_counter_freg;
}


static u32 Win32_Get_Window_Width()
{
	return s_app.window_width;
}


static u32 Win32_Get_Window_Height()
{
	return s_app.window_height;
}


static u32* Win32_Get_Pixel_Buffer()
{
	return s_bitmap.memory;
}


static i32 Win32_Get_Pixel_Buffer_Width()
{
	return s_bitmap.width;
}


static i32 Win32_Get_Pixel_Buffer_Height()
{
	return s_bitmap.height;
}


static u32 Win32_Get_Flags()
{
	return s_app.flags;
}


static Controller_State Win32_Get_Controller_State(i32 contoller_idx)
{
	Assert(contoller_idx >= 0 && contoller_idx < s_app.max_controllers);
	return s_app.controller_state[contoller_idx]; 
}


static f64 Win32_Get_Time_Stamp()
{
	//Update timing info.    
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);
	return f64(current_time.QuadPart) / s_app.timer_counter_freg;
}


static u64 Win32_Get_CPU_Time_Stamp()
{
	return __rdtsc();
}


static v2i Win32_Get_Cursor_Position()
{
	// TODO: cashe the border values.

	POINT cursor_p;
	BOOL get_cursor_pos_result = GetCursorPos(&cursor_p);
	
	RECT window_rect;
	BOOL get_window_rect_result = GetWindowRect(s_app.window, &window_rect);
	
	RECT client_rect;
	BOOL get_client_rect_result = GetClientRect(s_app.window, &client_rect);
	
	i32 window_width = window_rect.right - window_rect.left;
	i32 window_height = window_rect.bottom - window_rect.top;
	
	i32 border_width = (window_width - client_rect.right) / 2;
	i32 border_height = (window_height - client_rect.bottom) - border_width;
	
	// x and y are now relative to the top left of the windows client area.
	i32 x = cursor_p.x - (window_rect.left + border_width);
	i32 y = cursor_p.y - (window_rect.top + border_height);
	
	// Next they are converted to be relative to the bottom left. Increasing up and rightwards.
	y = (y - client_rect.bottom) * -1;
	
	// Next convert the screenspace coordinates into pixel space coordinates.
	x = i32(x * ((f32)s_bitmap.width / (f32)client_rect.right));
	y = i32(y * ((f32)s_bitmap.height / (f32)client_rect.bottom));
	
	return v2i{x, y};

}


static bool Win32_Get_Keyboard_Key_Down(Key_Code key_code)
{
	return (GetKeyState(s_keycode_map[(u32)key_code]) & (1 << 15)) > 0;
}


static u64 Win32_Get_Frame_Count()
{
	return s_app.frame_counter;
}


static bool Win32_Write_File(const char* file_name, u8* buffer, u32 bytes_to_write)
{
	HANDLE file = CreateFileA
	(
		file_name, 
		GENERIC_WRITE,
		0,
		0,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		0
	);
	
	if(file == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		return false;
	}
	
	DWORD bytes_written;
	BOOL write_succeeded = WriteFile
	(
		file,
		buffer,
		bytes_to_write,
		&bytes_written,
		0
	);
	
	Assert(bytes_to_write == bytes_written);
	
	CloseHandle(file);
	return write_succeeded;
}


static bool Win32_Get_File_Size(const char* file_name, u32* out_file_size)
{
	HANDLE file = CreateFileA
	(
		file_name, 
		GENERIC_READ,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0
	);
	
	if(file == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		*out_file_size = 0;
		return false;
	}
	
	*out_file_size = GetFileSize(file, 0);
	
	CloseHandle(file);
	return true;
}


static bool Win32_Read_File(const char* file_name, u8* out_buffer, u32 bytes_to_read)
{
	HANDLE file = CreateFileA
	(
		file_name, 
		GENERIC_READ,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0
	);
	
	if(file == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		return false;
	}
	
	DWORD bytes_read;
	BOOL file_read = ReadFile
	(
		file,
		out_buffer,
		bytes_to_read,
		&bytes_read,
		0
	);
	
	Assert(bytes_read == bytes_to_read);
	
	CloseHandle(file);
	return file_read;
}


static Char_Array Win32_Get_Typing_Information()
{
	return s_app.typing_information;
}


static Platform_Calltable Win32_Get_Calltable()
{
	Platform_Calltable ct = {};
	ct.Get_Window_Width = Win32_Get_Window_Width;
	ct.Get_Window_Height = Win32_Get_Window_Height;
	ct.Get_Pixel_Buffer = Win32_Get_Pixel_Buffer;
	ct.Get_Pixel_Buffer_Width = Win32_Get_Pixel_Buffer_Width;
	ct.Get_Pixel_Buffer_Height = Win32_Get_Pixel_Buffer_Height;
	ct.Resize_Pixel_Buffer = Win32_Resize_Pixel_Buffer;
	ct.Get_Flags = Win32_Get_Flags;
	ct.Set_Flag = Win32_Set_Flag;
	ct.Get_Keyboard_Key_Down = Win32_Get_Keyboard_Key_Down;
	ct.Get_Controller_State = Win32_Get_Controller_State;
	ct.Get_Time_Stamp = Win32_Get_Time_Stamp;
	ct.Get_CPU_Time_Stamp = Win32_Get_CPU_Time_Stamp;
	ct.Get_Cursor_Position = Win32_Get_Cursor_Position;
	ct.Get_Frame_Count = Win32_Get_Frame_Count;
	ct.Write_File = Win32_Write_File;
	ct.Get_File_Size = Win32_Get_File_Size;
	ct.Read_File = Win32_Read_File;
	ct.Get_Typing_Information = Win32_Get_Typing_Information;
	return ct;
}
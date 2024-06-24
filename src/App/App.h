
#pragma once

#include "..\Utility\Primitives.h"
#include "..\Utility\Assert.h"
#include "..\Platform\Input.h"
#include "..\Utility\Intrinsics.h"
#include "..\Utility\Bits.h"
#include "..\Utility\Allocator_Shell.h"
#include "..\Utility\Dynamic_Array.h"
#include "..\Utility\Vector_Ex.h"
#include "..\Utility\Maths.h"
#include "..\Utility\Utility.h"
#include "..\Utility\String.h"
#include "..\Utility\String_Ex.h"
#include "..\Platform\Platform_Interface.h"
#include "..\Utility\Instrumentation.h"
#include "..\Utility\Allocator.h"


static Allocator_Shell s_allocator;
static Paged_General_Allocator s_mem;

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

#define STBI_ASSERT(x)			  Assert(x)
#define STB_IMAGE_IMPLEMENTATION
#include "..\Vendor\stb_image.h"


#include "Terminus_Font.h"

#include "Renderer.h"
#include "Renderer.cpp"

#include "Action.h"

#include "GUI.h"
#include "GUI.cpp"

#include "Nalkapeli.h"
#include "Nalkapeli.cpp"

#include "Nalkapeli_GM.h"
#include "Nalkapeli_GM.cpp"

static Platform_Calltable s_platform;
static Canvas s_canvas;

#include "Menu_Core.h"
#include "Main_Menus.cpp"
#include "Editor_Menus.cpp"
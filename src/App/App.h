
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
static Linear_Allocator s_serialization_lalloc = {};

#include "Terminus_Font.h"

#include "Renderer.h"
#include "Renderer.cpp"

#include "Image_Loader.cpp"

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

#include "Memory_Display.cpp"

#pragma once

#include "..\Utility\Assert.h"
#include "..\Utility\Primitives.h"
#include "..\Utility\Vector_Ex.h"
#include "..\Utility\Maths.h"
#include "..\Utility\Utility.h"
#include "..\Utility\Allocator.h"
#include "..\Utility\String.h"
#include "..\Platform\Input.h"
#include "..\Platform\Platform_Interface.h"

#include "Renderer.h"
#include "Renderer.cpp"

#include "Action.h"

#include "GUI.h"
#include "GUI.cpp"

#include "Terminus_Font.h"

#define INTERIM_MEM_SIZE KiB * 32

static Platform_Calltable s_platform;

static General_Allocator s_mem;
static Linear_Allocator s_interim_mem;

static Canvas s_canvas;
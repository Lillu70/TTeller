
#pragma once

void Init_App(Platform_Calltable platform_calltable, void* app_memory, u32 app_memory_size);
void Update_App(f64 delta_time, bool* update_surface);

#pragma once


static inline u32 Put_Color(u8 r, u8 g, u8 b, u8 a = 0xff)
{
	return u32(a << 24) | u32(r << 16) | u32(g << 8) | u32(b << 0);
}


// Returns the lenght of the buffer, excluding the null terminator.
static u32 Null_Terminated_Buffer_Lenght(u8* buffer)
{
	u8* b;
	for(b = buffer; *b != 0; ++b){}
	return u32(b - buffer);
}


static inline u32 Null_Terminated_Buffer_Lenght(char* buffer)
{
	return Null_Terminated_Buffer_Lenght((u8*)buffer);
}


static inline b32 Is_Flag_Set(u32 field, u32 shift)
{
	return field & 1 << shift;
}


static inline void Mem_Copy(void* dest, void* source, u32 byte_count)
{
	Assert(dest);
	Assert(source);
	
	for(u32 i = 0; i < byte_count; ++i)
	{
		u8* sb = ((u8*)source) + i;
		u8* db = ((u8*)dest) + i;
		
		*db = *sb;
	}
}

static inline void Mem_Zero(void* mem, u32 byte_count)
{
	u8* mem_start = (u8*)mem;
	for(u8* p = mem_start; p < mem_start + byte_count; ++p)
		*p = 0;
}


static void Remove_Element_From_Packed_Array(void* array, u32* array_count, u32 element_size, u32 remove_idx)
{
	Assert(remove_idx < *array_count);
	for(u32 i = remove_idx; i < *array_count - 1; ++i)
	{
		u8* a = (u8*)array + (i * element_size);
		u8* b = a + element_size;
		Mem_Copy(a, b, element_size);
	}
	
	*array_count -= 1;
}


template<typename T>
static void Remove_Element_From_Packed_Array(T* array, u32* array_count, u32 remove_idx)
{
	Remove_Element_From_Packed_Array(array, array_count, sizeof(T), remove_idx);
}

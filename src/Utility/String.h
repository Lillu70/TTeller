#pragma once

// Capacity is always at least one more than lenght.
// Lenght is the character count, not including the null terminator character.
struct String
{
	char* buffer;
	u32 lenght;		// Character count of the string. Does not include the null terminator.
	u32 capacity; 	// The memory reserved for the string.
	
	Allocator_Shell* alloc;
	
	// CONSIDER: To member function or not? For some reason feels good here.
	void append_character(char c);
	void insert_at(u32 idx, char c);
	void remove_at(u32 idx);
	void erase(u32 from, u32 to);
	void pop_last();
	void free();
};


struct String_View
{
	char* buffer = 0;
	u32 lenght = 0;
};


String_View Create_String_View(String* str)
{
	Assert(str);
	Assert(str->buffer);
	Assert(str->lenght);
	
	String_View result = {str->buffer, str->lenght};
	
	return result;
}


String_View Create_String_View(char* c_str)
{
	Assert(c_str);
	String_View result = {c_str, Null_Terminated_Buffer_Lenght(c_str)};
	return result;
}


String_View Create_String_View(String* str, u32 start, u32 lenght)
{
	Assert(str);
	Assert(str->buffer);
	Assert(str->lenght);
	Assert(str->buffer + start + lenght <= str->buffer + str->lenght);
	
	String_View result = {str->buffer + start, lenght};
	
	return result;
}


static void String_Init_Alloc(String* str, u32 capacity)
{
	capacity = Max(str->alloc->min_alloc_size, capacity);
	str->capacity = capacity;
	
	str->buffer = (char*)str->alloc->push(capacity);
	Mem_Zero(str->buffer, str->capacity);
}


static void Init_String(String* str, Allocator_Shell* allocator, u32 capacity)
{
	Assert(allocator);
	
	str->lenght = 0;
	str->alloc = allocator;
	
	String_Init_Alloc(str, capacity);
}


static inline String Create_String(Allocator_Shell* allocator, u32 capacity)
{
	String result;
	Init_String(&result, allocator, capacity);
	
	return result;
}

static void Init_String(String* str, Allocator_Shell* allocator, char* c_str1, char* c_str2)
{
	Assert(allocator);
	
	u32 lenght1 = Null_Terminated_Buffer_Lenght(c_str1);
	u32 lenght2 = Null_Terminated_Buffer_Lenght(c_str2);

	str->lenght = lenght1 + lenght2;
	str->alloc = allocator;
	
	String_Init_Alloc(str, str->lenght+ 1);
	if(lenght1 > 0)
		Mem_Copy(str->buffer, c_str1, lenght1);
	
	if(lenght2 > 0)
		Mem_Copy(str->buffer + lenght1, c_str2, lenght2);
	
	str->buffer[str->lenght] = 0;
}


// Takes a null terminted C style string, as an argument. Lenght of it, determines capacity.
static void Init_String(String* str, Allocator_Shell* allocator, char* c_str)
{
	Assert(allocator);
	Assert(c_str);
	
	u32 lenght = Null_Terminated_Buffer_Lenght(c_str);
	
	str->lenght = lenght;
	str->alloc = allocator;
	
	String_Init_Alloc(str, lenght + 1);
	if(str->lenght > 0)
		Mem_Copy(str->buffer, c_str, lenght);
	
	str->buffer[lenght] = 0;
}


static inline String Create_String(Allocator_Shell* allocator, char* c_str)
{
	String result;
	Init_String(&result, allocator, c_str);
	
	return result;
}


static void Init_String(String* str, Allocator_Shell* allocator, char* c_str, u32 max_characters)
{
	Assert(allocator);
	Assert(c_str);
	
	u32 lenght = 0;
	char* c = c_str;
	for(; lenght < max_characters && *c != 0; ++lenght, ++c){}
	
	str->lenght = lenght;
	str->alloc = allocator;
	
	String_Init_Alloc(str, lenght + 1);
	if(str->lenght > 0)
		Mem_Copy(str->buffer, c_str, lenght);
	
	str->buffer[lenght] = 0;
}


void String::append_character(char c)
{
	if(capacity == 0)
	{
		Assert(lenght == 0);
		String_Init_Alloc(this, 2);
	}
	
	else if(lenght >= capacity - 1)
	{
		// CONSIDER: What kind of growth function to use here?
		capacity += 1;
		capacity = i32(capacity * 1.5f);
		char* mem = (char*)alloc->push(capacity);
		Mem_Zero(mem + lenght, capacity - lenght);
		if(buffer)
		{
			Mem_Copy(mem, buffer, lenght);
			alloc->free(buffer);			
		}
		buffer = mem;
	}
		
	*(buffer + lenght) = c;
	lenght += 1;
}


void String::pop_last()
{
	Assert(lenght > 0);
	buffer[lenght - 1] = 0;
	lenght -= 1;
}


void String::insert_at(u32 idx, char c)
{
	Assert(idx <= lenght);
	
	if(idx == lenght)
	{
		append_character(c);
	}
	else
	{
		// "append_character" Is used to grow the string if need be,
		// but it also increases the lenght by one. Normaly this is desired.
		// Problem is that "Insert_Element_Into_Packed_Array" also increases the character
		// count. We don't want to increase lenght twise so as a hack,
		// after appending lenght is reduced by one, so it ends up being tracked corretly.
		
		append_character(0);
		lenght -= 1; // :(
		
		// TODO: Make a Insert_Element_Into_Packed_Byte_Array function.
		// A version of the function that assumes size 1.
		Insert_Element_Into_Packed_Array(buffer, &c, &lenght, sizeof(*buffer), idx);
	}
}


void String::remove_at(u32 idx)
{
	Assert(idx <= lenght);
	
	if(idx == lenght)
		pop_last();
	else
	{
		// TODO: Make a Remove_Element_From_Packed_Byte_Array function.
		// A version of the function that assumes size 1.
		Remove_Element_From_Packed_Array(buffer, &lenght, sizeof(*buffer), idx);
	}
}


// "To" is exclusive.
void String::erase(u32 from, u32 to)
{
	for(u32 i = from; i < to; ++i)
		remove_at(from);
}


void String::free()
{
	if(buffer)
		alloc->free(buffer);

	buffer = 0;
	lenght = 0;
	capacity = 0;
}


static void operator += (String& str, char c)
{
	str.append_character(c);
}


static bool String_Compare(String* a, String* b)
{
	if(a->lenght != b->lenght)
		return false;
	
	char* aa = a->buffer;
	char* bb = b->buffer;
	
	u32 c = 0;
	while(*aa++ == *bb++)
	{
		if(++c == a->lenght)
			return true;
	}
	
	return false;
}


// Buffer size is assumed to be 11 or greater.
static char* U32_To_Char_Buffer(u8* buffer, u32 integer)
{
	// TODO: This works, but's odd and not very intuitive, so rethink and rework this.
	
	u32 buffer_size = 11;
	
	buffer[buffer_size - 1] = 0;
	
	u32 ascii_numeric_offset = 48;
	u32 last_non_zero = buffer_size - 2;
	for(u32 i = 0; i < buffer_size - 1; ++i)
	{
		u32 digit = 0;
		if(i > 0)
			digit = (u32)(integer / Pow32(10, i)) % 10;	
		else
			digit = integer % 10;
		u32 write_pos = buffer_size - 2 - i;
		if(digit)
			last_non_zero = write_pos;
		
		buffer[write_pos] = ascii_numeric_offset + digit;
	}
	
	return (char*)(buffer + last_non_zero);
}
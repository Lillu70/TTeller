#pragma once

// Capacity is always at least one more than lenght.
// Lenght is the character count, not including the null terminator character.
struct String
{
	char* buffer;
	u32 lenght;
	u32 capacity;
	
	General_Allocator* alloc;
	
	void append_character(char c);
	void pop_last();
};


static void String_Init_Alloc(String* str, u32 capacity)
{
	// NOTE: 2 then has to be the minimum for capacity ?
	
	capacity = Max(str->alloc->min_alloc_size, capacity);
	str->capacity = capacity;
	
	str->buffer = (char*)str->alloc->push(capacity);
	Mem_Zero(str->buffer, str->capacity);
}


static void Init_String(String* str, General_Allocator* allocator, u32 capacity)
{
	Assert(allocator);
	
	str->lenght = 0;
	str->capacity = capacity;
	str->alloc = allocator;
	
	if(capacity)
		String_Init_Alloc(str, capacity);
	else
		str->buffer = 0;
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

// BEWARE: when doing this to a pointer to a string. Dreference or you WILL fuck up the pointer.
static void operator += (String& str, char c)
{
	str.append_character(c);
}
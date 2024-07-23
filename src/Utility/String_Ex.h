
#pragma once

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


static u32 Convert_String_View_Into_U32(String_View view)
{
    u32 result = 0;
    u32 digit = view.lenght;
    
    char* begin = view.buffer;
    for(char* c = begin; c < begin + view.lenght; ++c)
    {
        u32 v = *c - u32('0');
        Assert(v < 10);
        
        v *= Pow32(10, --digit);
        result += v;
    }
    
    return result;
}
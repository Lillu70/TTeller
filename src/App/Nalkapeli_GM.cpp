
#pragma once

#pragma pack(push, 1)
struct Mark_Hash_Element
{
	String_View string;
	u32 key;
};
#pragma pack(pop)


struct Hash_Bucket_Header
{
	Hash_Bucket_Header* next_bucket;
	
	// --- methods ---
	Mark_Hash_Element* begin();
};


inline Mark_Hash_Element* Hash_Bucket_Header::begin()
{
	Mark_Hash_Element* result = (Mark_Hash_Element*)(this + 1);
	
	return result;
}


struct Hash_Table
{
	Hash_Bucket_Header* first_bucket;
	Hash_Bucket_Header* last_bucket;
	u32 _bucket_capacity;
	u32 unique_element_count;
	u32 bucket_count;
	
	// --- methods ---
	Hash_Bucket_Header* create_new_bucket(Allocator_Shell* allocator);
	bool push(String_View string, Allocator_Shell* allocator);
};


inline Hash_Bucket_Header* Hash_Table::create_new_bucket(Allocator_Shell* allocator)
{
	u32 element_size = sizeof(Mark_Hash_Element);
	u32 bucket_size = sizeof(Hash_Bucket_Header);
	u32 alloc_size = element_size * _bucket_capacity + bucket_size;
	
	Hash_Bucket_Header* result = (Hash_Bucket_Header*)allocator->push(alloc_size);
	Mem_Zero(result, alloc_size);
	
	bucket_count += 1;
	
	return result;
}


inline bool Hash_Table::push(String_View string, Allocator_Shell* allocator)
{
	u32 key = PCG_Hash(*string.buffer);
	{
		char* c;
		u32 i;
		for(c = string.buffer, i = 0; i < string.lenght; key += *c++, i++);
	}
	key = PCG_Hash(key);
	
	u32 slot = key % _bucket_capacity;
	
	u32 debug_insert_bucket = 0;
	
	bool result = false;
	
	Hash_Bucket_Header* bucket = first_bucket;
	while(bucket)
	{
		debug_insert_bucket++;
		
		Mark_Hash_Element* e = (Mark_Hash_Element*)(bucket + 1) + slot;
		
		// slot is empty, put it here!
		if(e->key == 0)
		{
			e->key = key;
			e->string = string;
			result = true;
			break;
		}
		
		// slot is occupied.
		else
		{			
			// check if content is matching.
			if(e->key == key && String_View_Compare(string, e->string))
			{
				result = false;
				break;
			}
		}
		
		bucket = bucket->next_bucket;
	}
	
	if(!bucket)
	{
		Hash_Bucket_Header* new_bucket = create_new_bucket(allocator);
		
		// Link the new bucket
		last_bucket->next_bucket = new_bucket;
		last_bucket = new_bucket;
		
		// Insert, no need for checking as this is an empty bucket.
		Mark_Hash_Element* e = (Mark_Hash_Element*)(new_bucket + 1) + slot;
		e->key = key;
		e->string = string;
		
		result = true;
	}
	
	if(result)
		unique_element_count += 1;
	
	return result;
}


Hash_Table Create_Hash_Table(Allocator_Shell* allocator, u32 bucket_capacity = 256)
{
	Hash_Table result = {};
	result._bucket_capacity = bucket_capacity;
	result.unique_element_count = 0;
	
	result.first_bucket = result.create_new_bucket(allocator);
	result.last_bucket = result.first_bucket;
	
	return result;
}










bool Load_Campaign_Into_Game_State_V3(
	Game_State* game_state,
	u8* buffer,
	u32 buffer_size,
	u32 read_head,
	Allocator_Shell* allocator,
	Platform_Calltable* platform)
{
	bool result = true;
	
	u32 read_head_record = read_head;
	
	Hash_Table unique_marks = Create_Hash_Table(allocator);
	
	u32 alloc_size = 0;
	
	// Memory requirement counting.
	{
		u32 total_unique_string_lenght = 0;
		u32 event_count = READ(u32);
		u32 day_event_count = READ(u32);
		
		alloc_size += sizeof(u32) * event_count; // Space for day and night event tables.
		alloc_size += sizeof(Event_Header) * event_count;
		
		
		Assert(day_event_count <= event_count);
		
		for(u32 e = 0; e < event_count; ++e)
		{
			// -- event name 
			u32 event_name_lenght = READ(u32);
			
			alloc_size += event_name_lenght;
			
			char* event_name_buffer = READ_TEXT(event_name_lenght);
			
			// -- global event reqs
			u32 global_req_count = READ(u32);
			
			alloc_size += sizeof(Global_Mark_Requirement_GM) * global_req_count;
			
			for(u32 gmr = 0; gmr < global_req_count; ++gmr)
			{
				Exists_Statement mark_exists = READ(Exists_Statement);
				i8 relation_target = READ(i8);
				Numerical_Relation numerical_relation = READ(Numerical_Relation);
				u32 mark_lenght = READ(u32);
				
				char* gmr_mark_buffer = READ_TEXT(mark_lenght);
				if(unique_marks.push({gmr_mark_buffer, mark_lenght}, allocator))
					total_unique_string_lenght += mark_lenght;
			}
			
			// -- global event cons
			u32 global_con_count = READ(u32);
			for(u32 gmc = 0; gmc < global_con_count; ++gmc)
			{
				i8 mark_duration = READ(i8);
				u32 mark_lenght = READ(u32);
				
				char* gmc_mark_buffer = READ_TEXT(mark_lenght);	
				if(unique_marks.push({gmc_mark_buffer, mark_lenght}, allocator))
					total_unique_string_lenght += mark_lenght;
			}
			
			
			// -- event text
			u32 event_text_lenght = READ(u32);
			
			alloc_size += event_text_lenght;
			
			char* event_text_buffer = READ_TEXT(event_text_lenght);
			
			// -- participents
			u32 participent_count = READ(u32);
			
			alloc_size += sizeof(Participant_Header) * participent_count * 2;
			
			for(u32 p = 0; p < participent_count; ++p)
			{			
				// Requirements block
				{
					u32 req_count = READ(u32);
					alloc_size += sizeof(Participation_Requirement_GM) * req_count;
					
					for(u32 req = 0; req < req_count; ++req)
					{
						Participation_Requirement_Type type = READ(Participation_Requirement_Type);
						
						Numerical_Relation numerical_relation = READ(Numerical_Relation);
						u16 relation_target = READ(u16);
						
						switch(type)
						{
							case Participation_Requirement_Type::character_stat:
							{
								Character_Stat::Stats stat_type = READ(Character_Stat::Stats);
							}break;
							
							case Participation_Requirement_Type::mark_item:
							case Participation_Requirement_Type::mark_personal:
							{
								u32 mark_lenght = READ(u32);
								char* mark_buffer = READ_TEXT(mark_lenght);
								
								if(unique_marks.push({mark_buffer, mark_lenght}, allocator))
									total_unique_string_lenght += mark_lenght;
								
								Exists_Statement mark_exists = READ(Exists_Statement);
							
							}break;
							
							default:
							{
								Terminate;
							}
						}
					}
				}
				
				// Consequenses block
				{
					u32 con_count = READ(u32);
					
					alloc_size += sizeof(Event_Consequens_GM) * con_count;
					
					for(u32 con = 0; con < con_count; ++con)
					{
						Event_Consequens_Type type = READ(Event_Consequens_Type);
						switch(type)
						{
							case Event_Consequens_Type::death:
							{
								u32 inherit_lenght = READ(u32);
								char* inherit_text = READ_TEXT(inherit_lenght);
							}break;
							
							case Event_Consequens_Type::gains_mark:
							case Event_Consequens_Type::loses_mark:
							{
								Mark_Type mark_type = READ(Mark_Type);
								i8 mark_duration = READ(i8);
								
								u32 mark_lenght = READ(u32);
								char* mark_text = READ_TEXT(mark_lenght);
								
								if(unique_marks.push({mark_text, mark_lenght}, allocator))
									total_unique_string_lenght += mark_lenght;
								
							}break;
							
							case Event_Consequens_Type::stat_change:
							{
								i8 stat_change_amount = READ(i8);
								Character_Stat::Stats stat = READ(Character_Stat::Stats);
							}break;
							
							
							default:
							{
								Terminate;
							}
						}
					}			
				}
			}
		}
		
		alloc_size += total_unique_string_lenght;
		alloc_size += unique_marks.unique_element_count * sizeof(u32);
	}
	
	
	
	return result;
}


bool Load_Campaign_Into_Game_State(
	Game_State* game_state,
	String* name,
	Allocator_Shell* allocator,
	Platform_Calltable* platform)
{
	
	String full_path = Create_Campaign_Full_Path(name, allocator);
	
	u32 buffer_size = 0;
	bool load_successful = false;
	if(platform->Get_File_Size(full_path.buffer, &buffer_size))
	{
		serialization_lalloc.clear();
		
		if(serialization_lalloc.get_free_capacity() < buffer_size)
			serialization_lalloc.init(platform, buffer_size);
		
		u8* buffer = (u8*)serialization_lalloc.push(buffer_size);
		u32 read_head = 0;
		
		*game_state = {};
		
		if(platform->Read_File(full_path.buffer, (u8*)buffer, buffer_size))
		{
			u32 version = READ(u32);
			if(version == 3)
			{
				load_successful = Load_Campaign_Into_Game_State_V3(
					game_state, 
					buffer, 
					buffer_size, 
					read_head, 
					allocator, 
					platform);
			}
		}
	}
	
	full_path.free();
	return load_successful;
}


Participation_Requirement_GM* Begin_Req_Array(Participant_Header* participant_header)
{
	return (Participation_Requirement_GM*)(participant_header + 1);
}


// Return a pointer pointing to first address after the end of the array.
Participation_Requirement_GM* End_Req_Array(Participant_Header* participant_header)
{
	return (Participation_Requirement_GM*)(participant_header + 1) + participant_header->count;
}


Event_Consequens_GM* Begin_Con_Array(Participant_Header* participant_header)
{
	return (Event_Consequens_GM*)(participant_header + 1);
}


// Return a pointer pointing to first address after the end of the array.
Event_Consequens_GM* End_Con_Array(Participant_Header* participant_header)
{
	return (Event_Consequens_GM*)(participant_header + 1) + participant_header->count;
}

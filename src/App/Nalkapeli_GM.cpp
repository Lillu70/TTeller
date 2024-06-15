
#pragma once

#pragma pack(push, 1)
struct Mark_Hash_Element
{
	String_View string;
	u32 table_slot;
};
#pragma pack(pop)


struct Mark_Hash_Bucket_Header
{
	Mark_Hash_Bucket_Header* next_bucket;
	
	// --- methods ---
	Mark_Hash_Element* begin();
};


inline Mark_Hash_Element* Mark_Hash_Bucket_Header::begin()
{
	Mark_Hash_Element* result = (Mark_Hash_Element*)(this + 1);
	
	return result;
}


struct Mark_Hash_Table
{
	Mark_Hash_Bucket_Header* first_bucket;
	Mark_Hash_Bucket_Header* last_bucket;
	u32 _bucket_capacity;
	u32 unique_element_count;
	u32 total_unique_string_lenght;
	u32 bucket_count;
	
	// --- methods ---
	Mark_Hash_Bucket_Header* create_new_bucket(Allocator_Shell* allocator);
	bool push(String_View string, Allocator_Shell* allocator);
};


inline Mark_Hash_Bucket_Header* Mark_Hash_Table::create_new_bucket(Allocator_Shell* allocator)
{
	u32 element_size = sizeof(Mark_Hash_Element);
	u32 bucket_size = sizeof(Mark_Hash_Bucket_Header);
	u32 alloc_size = element_size * _bucket_capacity + bucket_size;
	
	Mark_Hash_Bucket_Header* result = (Mark_Hash_Bucket_Header*)allocator->push(alloc_size);
	Mem_Zero(result, alloc_size);
	
	bucket_count += 1;
	
	return result;
}


inline bool Mark_Hash_Table::push(String_View string, Allocator_Shell* allocator)
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
	
	Mark_Hash_Bucket_Header* bucket = first_bucket;
	while(bucket)
	{
		debug_insert_bucket++;
		
		Mark_Hash_Element* e = (Mark_Hash_Element*)(bucket + 1) + slot;
		
		// slot is empty, put it here!
		if(e->string.buffer == 0)
		{
			e->string = string;
			result = true;
			break;
		}
		
		// slot is occupied.
		else
		{			
			// check if content is matching.
			if(String_View_Compare(string, e->string))
			{
				result = false;
				break;
			}
		}
		
		bucket = bucket->next_bucket;
	}
	
	if(!bucket)
	{
		Mark_Hash_Bucket_Header* new_bucket = create_new_bucket(allocator);
		
		// Link the new bucket
		last_bucket->next_bucket = new_bucket;
		last_bucket = new_bucket;
		
		// Insert, no need for checking as this is an empty bucket.
		Mark_Hash_Element* e = (Mark_Hash_Element*)(new_bucket + 1) + slot;
		e->string = string;
		
		result = true;
	}
	
	if(result)
	{
		unique_element_count += 1;
		total_unique_string_lenght += string.lenght + 1;
	}
	
	return result;
}


Mark_Hash_Table Create_Mark_Hash_Table(Allocator_Shell* allocator, u32 bucket_capacity = 256)
{
	Mark_Hash_Table result = {};
	result._bucket_capacity = bucket_capacity;
	result.unique_element_count = 0;
	
	result.first_bucket = result.create_new_bucket(allocator);
	result.last_bucket = result.first_bucket;
	
	return result;
}


void Free_Mark_Hash_Table(Mark_Hash_Table* ht, Allocator_Shell* allocator)
{
	Mark_Hash_Bucket_Header* bucket = ht->first_bucket;
	while(bucket)
	{
		Mark_Hash_Bucket_Header* next = bucket->next_bucket;
		allocator->free(bucket);
		
		bucket = next;
	}
}










bool Load_Campaign_Into_Game_State_V3(
	Game_State* game_state,
	u8* buffer,
	u32 buffer_size,
	u32 read_head,
	Allocator_Shell* allocator,
	Platform_Calltable* platform)
{
	*game_state = {};
	bool result = true;
	
	u32 read_head_record = read_head;
	
	Mark_Hash_Table marks = Create_Mark_Hash_Table(allocator, 64);
	
	u32 alloc_size = 0;
	u32 requirements_data_size = 0;
	
	// Memory requirement counting.
	{
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
			
			u32 global_req_size = sizeof(Global_Mark_Requirement_GM) * global_req_count;
			
			requirements_data_size += global_req_size;
			alloc_size += global_req_size;
			
			for(u32 gmr = 0; gmr < global_req_count; ++gmr)
			{
				Exists_Statement mark_exists = READ(Exists_Statement);
				i8 relation_target = READ(i8);
				Numerical_Relation numerical_relation = READ(Numerical_Relation);
				u32 mark_lenght = READ(u32);
				
				char* gmr_mark_buffer = READ_TEXT(mark_lenght);
				marks.push({gmr_mark_buffer, mark_lenght}, allocator);
			}
			
			// -- global event cons
			u32 global_con_count = READ(u32);
			for(u32 gmc = 0; gmc < global_con_count; ++gmc)
			{
				i8 mark_duration = READ(i8);
				u32 mark_lenght = READ(u32);
				
				char* gmc_mark_buffer = READ_TEXT(mark_lenght);	
				marks.push({gmc_mark_buffer, mark_lenght}, allocator);
			}
			
			
			// -- event text
			u32 event_text_lenght = READ(u32);
			
			alloc_size += event_text_lenght;
			
			char* event_text_buffer = READ_TEXT(event_text_lenght);
			
			// -- participents
			u32 participent_count = READ(u32);
			
			alloc_size += sizeof(Participant_Header) * participent_count * 2;
			
			requirements_data_size += sizeof(Participant_Header) * participent_count;
			
			for(u32 p = 0; p < participent_count; ++p)
			{			
				// Requirements block
				{
					u32 req_count = READ(u32);
					
					u32 participant_requirement_size = sizeof(Participation_Requirement_GM) * req_count;
					requirements_data_size += participant_requirement_size;
					alloc_size += participant_requirement_size;
					
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
								
								marks.push({mark_buffer, mark_lenght}, allocator);
								
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
								
								marks.push({mark_text, mark_lenght}, allocator);
								
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
		
		alloc_size += marks.total_unique_string_lenght;
		alloc_size += marks.unique_element_count * sizeof(u32);
	}
	
	game_state->memory = allocator->push(alloc_size);
	Linear_Allocator game_memory = Create_Linear_Allocator(game_state->memory, alloc_size);
	
	// Create mark data and table.
	{
		Linear_Allocator mark_data = Create_Linear_Allocator(
			game_memory.push(marks.total_unique_string_lenght), 
			marks.total_unique_string_lenght);
		
		game_state->mark_data = (char*)mark_data.memory;

	
		u32 mark_table_size = sizeof(u32) * marks.unique_element_count;
		Linear_Allocator mark_table = Create_Linear_Allocator(
			game_memory.push(mark_table_size), 
			mark_table_size);
		
		game_state->mark_table = Table{marks.unique_element_count, mark_table.memory};
		
		Mark_Hash_Bucket_Header* bucket = marks.first_bucket;
		u32 hit_count = 0;
		while(bucket)
		{
			Mark_Hash_Element* begin = (Mark_Hash_Element*)(bucket + 1);
			Mark_Hash_Element* end = begin + marks._bucket_capacity;
			
			for(Mark_Hash_Element* element = begin; element < end; ++element)
			{
				if(element->string.buffer)
				{
					element->table_slot = hit_count++;
					
					char* buffer = (char*)mark_data.push(element->string.lenght + 1);
					Mem_Copy(buffer, element->string.buffer, element->string.lenght);
					buffer[element->string.lenght] = 0;
					
					u32 offset = u32(buffer - game_state->mark_data);
					
					*mark_table.push<u32>() = offset;
				}
			}
			
			bucket = bucket->next_bucket;
		}
	}
	
	game_state->req_data = game_memory.push(requirements_data_size);
	
	#if 0 // Test Code!
	{
		Table mark_table = game_state->mark_table;
		char* mark_data = game_state->mark_data;
		
		for(u32 i = 0; i < mark_table.count; ++i)
		{
			char* buffer = mark_data + *((u32*)mark_table.memory + i);
			int a = 0;
		}
	}
	#endif
	
	// Second pass read
	read_head = read_head_record;
	{
		u32 event_count = READ(u32);
		u32 day_event_count = READ(u32);

		Assert(day_event_count <= event_count);
		
		for(u32 e = 0; e < event_count; ++e)
		{
			// -- event name 
			u32 event_name_lenght = READ(u32);
			char* event_name_buffer = READ_TEXT(event_name_lenght);
			
			// -- global event reqs
			u32 global_req_count = READ(u32);
			
			for(u32 gmr = 0; gmr < global_req_count; ++gmr)
			{
				Exists_Statement mark_exists = READ(Exists_Statement);
				i8 relation_target = READ(i8);
				Numerical_Relation numerical_relation = READ(Numerical_Relation);
				u32 mark_lenght = READ(u32);
				
				char* gmr_mark_buffer = READ_TEXT(mark_lenght);
			}
			
			// -- global event cons
			u32 global_con_count = READ(u32);
			for(u32 gmc = 0; gmc < global_con_count; ++gmc)
			{
				i8 mark_duration = READ(i8);
				u32 mark_lenght = READ(u32);
				
				char* gmc_mark_buffer = READ_TEXT(mark_lenght);	
			}
			
			
			// -- event text
			u32 event_text_lenght = READ(u32);
			char* event_text_buffer = READ_TEXT(event_text_lenght);
			
			// -- participents
			u32 participent_count = READ(u32);
			for(u32 p = 0; p < participent_count; ++p)
			{			
				// Requirements block
				{
					u32 req_count = READ(u32);
					
	
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
								Exists_Statement mark_exists = READ(Exists_Statement);
							}break;
						}
					}
				}
				
				// Consequenses block
				{
					u32 con_count = READ(u32);
					
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
							}break;
							
							case Event_Consequens_Type::stat_change:
							{
								i8 stat_change_amount = READ(i8);
								Character_Stat::Stats stat = READ(Character_Stat::Stats);
							}break;
						}
					}			
				}
			}
		}
	}
	
	
	Free_Mark_Hash_Table(&marks, allocator);
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

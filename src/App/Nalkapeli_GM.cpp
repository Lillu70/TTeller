
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
	
	Mark_Hash_Element* get(String_View string);
	u32 get_slot(String_View* string);
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


inline u32 Mark_Hash_Table::get_slot(String_View* string)
{
	u32 key = PCG_Hash(*string->buffer);
	{
		char* c;
		u32 i;
		for(c = string->buffer, i = 0; i < string->lenght; key += *c++, i++);
	}
	key = PCG_Hash(key);
	
	u32 result = key % _bucket_capacity;
	
	return result;
}


inline bool Mark_Hash_Table::push(String_View string, Allocator_Shell* allocator)
{
	Assert(string.buffer);
	
	u32 slot = get_slot(&string);
	
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


inline Mark_Hash_Element* Mark_Hash_Table::get(String_View string)
{
	u32 slot = get_slot(&string);
	
	u32 debug_insert_bucket = 0;
	
	Mark_Hash_Bucket_Header* bucket = first_bucket;
	
	Mark_Hash_Element* result = 0;
	
	while(bucket)
	{
		debug_insert_bucket++;
		
		Mark_Hash_Element* e = (Mark_Hash_Element*)(bucket + 1) + slot;
		if(e->string.buffer != 0 && String_View_Compare(string, e->string))
		{			
			result = e;
			break;
		}
		
		bucket = bucket->next_bucket;
	}
	
	Assert(result);
	
	return result;
}


static Mark_Hash_Table Create_Mark_Hash_Table(Allocator_Shell* allocator, u32 bucket_capacity = 256)
{
	Mark_Hash_Table result = {};
	result._bucket_capacity = bucket_capacity;
	result.unique_element_count = 0;
	
	result.first_bucket = result.create_new_bucket(allocator);
	result.last_bucket = result.first_bucket;
	
	return result;
}


static void Free_Mark_Hash_Table(Mark_Hash_Table* ht, Allocator_Shell* allocator)
{
	Mark_Hash_Bucket_Header* bucket = ht->first_bucket;
	while(bucket)
	{
		Mark_Hash_Bucket_Header* next = bucket->next_bucket;
		allocator->free(bucket);
		
		bucket = next;
	}
}


static Global_Mark_Requirement_GM Convert_To_GM(Global_Mark_Requirement* gmr, Mark_Hash_Table* marks)
{
	Global_Mark_Requirement_GM result;
	
	result.mark_exists = gmr->mark_exists;
	result.numerical_relation = gmr->numerical_relation;
	result.relation_target = gmr->relation_target;
	
	u32 slot = marks->get(Create_String_View(&gmr->mark))->table_slot;
	Assert(slot);
	
	result.mark_idx = slot - 1; 
	
	return result;
}


static Global_Mark_Consequence_GM Convert_To_GM(Global_Mark_Consequence* gmc, Mark_Hash_Table* marks)
{
	Global_Mark_Consequence_GM result;
	
	result.mark_duration = gmc->mark_duration;
	
	u32 slot = marks->get(Create_String_View(&gmc->mark))->table_slot;
	Assert(slot);
	
	result.mark_idx = slot - 1; 
	
	return result;
}



static Participation_Requirement_GM Convert_To_GM(Participation_Requirement* req, Mark_Hash_Table* marks)
{
	Participation_Requirement_GM result;
	result.type = req->type;
	result.numerical_relation = req->numerical_relation;
	
	switch(req->type)
	{
		case Participation_Requirement_Type::character_stat:
		{
			result.relation_target = i8(req->relation_target);
			result.stat_type = req->stat_type;
		}break;
		
		case Participation_Requirement_Type::mark_personal:
		case Participation_Requirement_Type::mark_item:
		{
			result.duration = i8(req->relation_target);
			result.mark_exists = req->mark_exists;
			
			u32 slot = marks->get(Create_String_View(&req->mark))->table_slot;
			Assert(slot);
			
			result.mark_idx = slot - 1;
			
		}break;
		
		default:
		{
			Terminate;
		}
	}
	
	return result;
}


static Event_Consequens_GM Convert_To_GM(Event_Consequens* con, Mark_Hash_Table* marks)
{
	Event_Consequens_GM result;
	result.type = con->type;
	
	switch(con->type)
	{
		case Event_Consequens_Type::death:
		{
			// TODO: Implement this!
			result.inheritance_target = 0;
		}break;
		
		case Event_Consequens_Type::stat_change:
		{
			result.change_amount = con->stat_change_amount;
			result.stat = con->stat;
		}break;
		
		case Event_Consequens_Type::gains_mark:
		{
			result.duration = con->mark_duration;
		}
		case Event_Consequens_Type::loses_mark:
		{
			result.mark_type = con->mark_type;
			
			u32 slot = marks->get(Create_String_View(&con->str))->table_slot;
			Assert(slot);
			
			result.mark_idx = slot - 1;
		}break;
	}
	
	return result;
}



static bool Convert_Editor_Campaign_Into_Game_Format(
	Game_State* game_state,
	Events_Container* event_container,
	Allocator_Shell* allocator)
{
	Assert(game_state);
	Assert(event_container);
	Assert(allocator);
	
	*game_state = {};
	bool result = true;
	
	u32 alloc_size = 0;
	u32 requirements_data_size = 0;
	u32 events_data_size = 0;
	
	Mark_Hash_Table marks = Create_Mark_Hash_Table(allocator, 64);
	
	// First figure out the memory required for the game state.
	{
		alloc_size += sizeof(u32) * event_container->events->count * 2;
		events_data_size += sizeof(Event_Header) * event_container->events->count;
		requirements_data_size += sizeof(Req_GM_Header) * event_container->events->count;
		
		for(Event_State* e = Begin(event_container->events); e < End(event_container->events); ++e)
		{
			u32 participant_header_size = sizeof(Participant_Header) * e->participents->count;
			
			events_data_size += participant_header_size;
			events_data_size += sizeof(Global_Mark_Consequence_GM) * e->global_mark_cons->count;
			events_data_size += e->name.lenght + 1;
			events_data_size +=	e->event_text.lenght + 1;
			
			requirements_data_size += participant_header_size;
			requirements_data_size += sizeof(Global_Mark_Requirement_GM) * e->global_mark_reqs->count;
			
			for(auto gmr = Begin(e->global_mark_reqs); gmr < End(e->global_mark_reqs); ++gmr)
				marks.push(String_View{gmr->mark.buffer, gmr->mark.lenght}, allocator);
			
			for(auto gmc = Begin(e->global_mark_cons); gmc < End(e->global_mark_cons); ++gmc)
				marks.push(String_View{gmc->mark.buffer, gmc->mark.lenght}, allocator);				
		
			for(Participent* p = Begin(e->participents); p < End(e->participents); ++p)
			{ 
				requirements_data_size += sizeof(Participation_Requirement_GM) * p->reqs->count;
				events_data_size += sizeof(Event_Consequens_GM) * p->cons->count;
				
				for(Participation_Requirement* req = Begin(p->reqs); req < End(p->reqs); ++req)
				{
					if(req->type == Participation_Requirement_Type::mark_item ||
						req->type ==  Participation_Requirement_Type::mark_personal)
					{
						marks.push(String_View{req->mark.buffer, req->mark.lenght}, allocator);
					}
				}
				
				for(Event_Consequens* con = Begin(p->cons); con < End(p->cons); ++con)
				{
					if(con->type == Event_Consequens_Type::gains_mark ||
						con->type ==  Event_Consequens_Type::loses_mark)
					{
						marks.push(String_View{con->str.buffer, con->str.lenght}, allocator);
					}
				}
			}
		}
	}
	
	alloc_size += requirements_data_size;					// req data
	alloc_size += events_data_size;							// events data
	
	alloc_size += sizeof(u32) * marks.unique_element_count; // mark table
	alloc_size += marks.total_unique_string_lenght;			// mark data
	
	game_state->memory = allocator->push(alloc_size);
	Linear_Allocator mem = Create_Linear_Allocator(game_state->memory, alloc_size);
	
	// Create mark data and table.
	{
		Linear_Allocator mark_data = Create_Linear_Allocator(
			mem.push(marks.total_unique_string_lenght), 
			marks.total_unique_string_lenght);
		
		game_state->mark_data = (char*)mark_data.memory;
	
		u32 mark_table_size = sizeof(u32) * marks.unique_element_count;
		Linear_Allocator mark_table = Create_Linear_Allocator(
			mem.push(mark_table_size), 
			mark_table_size);
		
		game_state->mark_table = Table{marks.unique_element_count, mark_table.memory};
		
		Mark_Hash_Bucket_Header* bucket = marks.first_bucket;
		u32 table_slot = 1;
		while(bucket)
		{
			Mark_Hash_Element* begin = (Mark_Hash_Element*)(bucket + 1);
			Mark_Hash_Element* end = begin + marks._bucket_capacity;
			
			for(Mark_Hash_Element* element = begin; element < end; ++element)
			{
				if(element->string.buffer)
				{
					element->table_slot = table_slot++;
					
					char* buffer = (char*)mark_data.push(element->string.lenght + 1);
					Mem_Copy(buffer, element->string.buffer, element->string.lenght + 1);
					
					u32 offset = u32(buffer - game_state->mark_data);
					
					*mark_table.push<u32>() = offset;
				}
			}
			
			bucket = bucket->next_bucket;
		}
		
		Assert(!mark_data.get_free_capacity());
	}
	
	game_state->req_data = (Req_GM_Header*)mem.push(requirements_data_size);
	Linear_Allocator req_data = Create_Linear_Allocator(game_state->req_data, requirements_data_size);
	
	u32 req_table_day_size = sizeof(u32) * event_container->day_event_count;
	game_state->req_table_day = Table{event_container->day_event_count, mem.push(req_table_day_size)};
	
	Linear_Allocator req_table_day 
		= Create_Linear_Allocator(game_state->req_table_day.memory, req_table_day_size);
	
	u32 night_event_count = event_container->events->count - event_container->day_event_count;
	u32 req_table_night_size = sizeof(u32) * night_event_count;
	game_state->req_table_night = Table{night_event_count, mem.push(req_table_night_size)};
	
	Linear_Allocator req_table_night 
		= Create_Linear_Allocator(game_state->req_table_night.memory,req_table_night_size);
	
	game_state->events_data = (Event_Header*)mem.push(events_data_size);
	Linear_Allocator events_data = Create_Linear_Allocator(game_state->events_data, events_data_size);
	
	game_state->event_table_day = Table{game_state->req_table_day.count, mem.push(req_table_day_size)};		
	Linear_Allocator event_table_day 
		= Create_Linear_Allocator(game_state->event_table_day.memory, req_table_day_size);
	
	
	game_state->event_table_night = Table{night_event_count, mem.push(req_table_night_size)};
	Linear_Allocator event_table_night
		= Create_Linear_Allocator(game_state->event_table_night.memory, req_table_night_size);

	Assert(!mem.get_free_capacity());
	
	// Slot in the data
	{
		u32 i = 0;
		for(Event_State* e = Begin(event_container->events); e < End(event_container->events); ++e, ++i)
		{
			Req_GM_Header* req_header = req_data.push<Req_GM_Header>();
			
			{
				u32 offset = u32((u8*)req_header - (u8*)game_state->req_data);
				if(i < event_container->day_event_count)
					*req_table_day.push<u32>() = offset;
				else
					*req_table_night.push<u32>() = offset;				
			}
			
			req_header->participant_count = e->participents->count;
			req_header->global_req_count = e->global_mark_reqs->count;
			
			for(auto gmr = Begin(e->global_mark_reqs); gmr < End(e->global_mark_reqs); ++gmr)
				*req_data.push<Global_Mark_Requirement_GM>() = Convert_To_GM(gmr, &marks);
			
			
			Event_Header* event_header = events_data.push<Event_Header>();
			
			{
				u32 offset = u32((u8*)event_header - (u8*)game_state->events_data);
				if(i < event_container->day_event_count)
					*event_table_day.push<u32>() = offset;
				else
					*event_table_night.push<u32>() = offset;				
			}
			
			event_header->event_name = String_View{(char*)events_data.push(e->name.lenght + 1), e->name.lenght};
			
			// name and text
			
			Mem_Copy(event_header->event_name.buffer, e->name.buffer, e->name.lenght + 1);
			event_header->event_text 
				= String_View{(char*)events_data.push(e->event_text.lenght + 1), e->event_text.lenght};
			Mem_Copy(event_header->event_text.buffer, e->event_text.buffer, e->event_text.lenght + 1);
			
			u32 event_size = e->name.lenght + e->event_text.lenght + 2;
			// ---
			
			event_header->global_con_count = e->global_mark_cons->count;
			for(auto gmc = Begin(e->global_mark_cons); gmc < End(e->global_mark_cons); ++gmc)
				*events_data.push<Global_Mark_Consequence_GM>() = Convert_To_GM(gmc, &marks);
			
			event_size += event_header->global_con_count * sizeof(Global_Mark_Consequence_GM);
			
			u32 req_count = 0;
			u32 con_count = 0;
			for(Participent* p = Begin(e->participents); p < End(e->participents); ++p)
			{
				Participant_Header* req_parti_header = req_data.push<Participant_Header>();
				
				u32 p_req_count = p->reqs->count;
				req_parti_header->count = p_req_count;
				req_count += p_req_count;
				
				Participant_Header* con_parti_header = events_data.push<Participant_Header>();
				
				u32 p_con_count = p->cons->count;
				con_parti_header->count = p_con_count;
				con_count += p_con_count;
				
				for(Participation_Requirement* req = Begin(p->reqs); req < End(p->reqs); ++req)
					*req_data.push<Participation_Requirement_GM>() = Convert_To_GM(req, &marks);
				
				for(Event_Consequens* con = Begin(p->cons); con < End(p->cons); ++con)
					*events_data.push<Event_Consequens_GM>() = Convert_To_GM(con, &marks);
			}
			
			req_header->req_count = req_count;
		}
	}
	
	#if 1 // Test Code!
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
	Free_Mark_Hash_Table(&marks, allocator);
	
	Assert(!req_data.get_free_capacity());
	Assert(!req_table_day.get_free_capacity());
	Assert(!req_table_night.get_free_capacity());
	Assert(!events_data.get_free_capacity());
	Assert(!event_table_day.get_free_capacity());
	Assert(!event_table_night.get_free_capacity());
	
	game_state->language = Language::finnish;
	
	game_state->player_names 
		= Create_Dynamic_Array<Game_Player_Name_FI>(allocator, u32(6));
	
	game_state->players = Create_Dynamic_Array<Game_Player>(allocator, u32(6));
	game_state->player_images = Create_Dynamic_Array<Image>(allocator, u32(6));
	
	return result;
}


static Participation_Requirement_GM* Begin_Req_Array(Participant_Header* participant_header)
{
	return (Participation_Requirement_GM*)(participant_header + 1);
}


// Return a pointer pointing to first address after the end of the array.
static Participation_Requirement_GM* End_Req_Array(Participant_Header* participant_header)
{
	return (Participation_Requirement_GM*)(participant_header + 1) + participant_header->count;
}


static Event_Consequens_GM* Begin_Con_Array(Participant_Header* participant_header)
{
	return (Event_Consequens_GM*)(participant_header + 1);
}


// Return a pointer pointing to first address after the end of the array.
static Event_Consequens_GM* End_Con_Array(Participant_Header* participant_header)
{
	return (Event_Consequens_GM*)(participant_header + 1) + participant_header->count;
}


static void Hollow_Player_Name_FI(Game_Player_Name_FI* player)
{
	Assert(player);

	player->full_name.free();
	player->variant_name_1.free();
	player->variant_name_2.free();
}


static void Delete_Game(Game_State* gm, Allocator_Shell* allocator)
{
	allocator->free(gm->memory);
	allocator->free(gm->players);
	
	for(Image* i = Begin(gm->player_images); i < End(gm->player_images); ++i)
	{
		if(i->buffer)
			allocator->free(i->buffer);
	}
	allocator->free(gm->player_images);
	
	if(gm->player_names)
	{
		for(auto n = Begin(gm->player_names); n < End(gm->player_names); ++n)
			Hollow_Player_Name_FI(n);
	}
	allocator->free(gm->player_names);
}


static void Create_Player_Name_FI(Game_State* gm, Allocator_Shell* allocator)
{
	Assert(gm);
	Assert(gm->memory);
	Assert(gm->player_names);

	Game_Player_Name_FI* name = Push(&gm->player_names, allocator);
	
	Init_String(&name->full_name, allocator, u32(0));
	Init_String(&name->variant_name_1, allocator, u32(0));
	Init_String(&name->variant_name_2, allocator, u32(0));
}
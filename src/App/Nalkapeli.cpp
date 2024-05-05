
#pragma once

static inline bool Requirement_Is_Mark_Type(Participation_Requirement::Type type)
{
	bool result = type == Participation_Requirement::Type::mark_item ||
		type == Participation_Requirement::Type::mark_personal;
	
	return result;
}


static inline void Make_Requirement_Hollow(Participation_Requirement* req)
{
	req->mark.free();
}


static inline void Make_Consequense_Hollow(Event_Consequens* con)
{
	con->str.free();
}


static inline Participent* Create_Participent(Dynamic_Array<Participent>** darray, Allocator_Shell* allocator)
{
	Participent* new_parti = Push(darray, allocator);
	new_parti->reqs = Create_Dynamic_Array<Participation_Requirement>(allocator, 3);
	new_parti->cons = Create_Dynamic_Array<Event_Consequens>(allocator, 3);
	
	return new_parti;
}


static inline void Hollow_Participent(u32 idx_to_delete, 
	Dynamic_Array<Participent>* darray, 
	Allocator_Shell* allocator)
{
	Participent* buffer = Begin(darray);
	Participent* parti = buffer + idx_to_delete;
	
	for(Participation_Requirement* req = Begin(parti->reqs); req < End(parti->reqs); ++req)
		Make_Requirement_Hollow(req);
	
	
	for(Event_Consequens* con = Begin(parti->cons); con < End(parti->cons); ++con)
		Make_Consequense_Hollow(con);
	
	allocator->free(parti->reqs);
	allocator->free(parti->cons);
}


static inline void Delete_Participent(
	u32 idx_to_delete, 
	Dynamic_Array<Participent>* darray, 
	Allocator_Shell* allocator)
{
	Hollow_Participent(idx_to_delete, darray, allocator);
	
	Participent* buffer = Begin(darray);
	u32 size = sizeof(*buffer);
	Remove_Element_From_Packed_Array(buffer, &darray->count, size, idx_to_delete);	
}


static inline void Init_Event_Takes_Name_Ownership(
	Event_State* event, 
	Allocator_Shell* allocator, 
	String* name)
{
	event->participents = Create_Dynamic_Array<Participent>(allocator, 4);
	Init_String(&event->event_text, allocator, 128);
	event->name = *name;
}


static void Delete_All_Participants_From_Event(Event_State* event, Allocator_Shell* allocator)
{
	for(u32 i = 0; i < event->participents->count; ++i)
		Hollow_Participent(i, event->participents, allocator);

	event->participents->count = 0;
}


static void Delete_Event(
	Dynamic_Array<Event_State>* darray, 
	Allocator_Shell* allocator, 
	u32 idx_to_delete, 
	bool remove_from_array=true)
{
	Assert(idx_to_delete < darray->count);
	
	Event_State* buffer = Begin(darray);
	Event_State* element = buffer + idx_to_delete;
	
	Delete_All_Participants_From_Event(element, allocator);
	allocator->free(element->participents);
	element->name.free();
	element->event_text.free();
	
	// Optional toggle, to allow optimization when deleting all events, normaly this should be "true"
	if(remove_from_array)
		Remove_Element_From_Packed_Array(buffer, &darray->count, sizeof(*element), idx_to_delete);
}


static String Generate_Unique_Name(Event_State* events, u32 event_count, Allocator_Shell* allocator)
{
	char* def_name = "Uusi tapahtuma";
	u32 def_name_len = Null_Terminated_Buffer_Lenght(def_name);
	String unique_name = Create_String(allocator, def_name);
	for(u32 attempt_count = 1; ;++attempt_count)
	{
		bool name_is_unique = true;
		
		for(Event_State* e = events; e < events + event_count; ++e)
		{
			if(String_Compare(&unique_name, &e->name))
			{
				// make a new name!
				u8 num_buffer[14];
				
				char* num_ptr = U32_To_Char_Buffer((u8*)num_buffer + 2, attempt_count) - 2;
				*(num_ptr) = ' ';
				*(num_ptr + 1) = '(';
				num_buffer[12] = ')';
				num_buffer[13] = 0;
				
				u32 byte_count = u32(((u8*)num_buffer + 14) - (u8*)num_ptr);
				u32 total_str_lenght = def_name_len + byte_count;
				
				// Effectively needs a realloc!
				if(total_str_lenght > unique_name.capacity)
				{
					unique_name.free();
					Init_String(&unique_name, allocator, def_name, num_ptr);					
				}
				// Can just the memory that is there.
				else
				{
					Mem_Copy(unique_name.buffer, def_name, def_name_len);
					Mem_Copy(unique_name.buffer + def_name_len, num_ptr, byte_count);							
					unique_name.lenght = total_str_lenght - 1;
				}
				
				name_is_unique = false;
				break;
			}	
		}
		
		if(name_is_unique)
			break;
	}
	
	return unique_name;
}


static Linear_Allocator serialization_lalloc = {};

static void Serialize_Campaign(
	Events_Container event_container, 
	Platform_Calltable* platform)
{	
	/*
	Version History 1->2: Added seperation of day and night events.
	*/
	
	//TODO: Write like a file description thing or something maybe?
	
	#define WRITE(X, type)\
	{\
		void* adrs = serialization_lalloc.safe_push(sizeof(type));\
		if(adrs != 0)\
		{\
			*(type*)adrs = X;\
		}\
		else\
		{\
			goto OUT_OF_MEMORY;\
		}\
	}
	
	#define WRITE_BLOCK(copy_buffer, copy_amount)\
	{\
		void* adrs = serialization_lalloc.safe_push(copy_amount);\
		if(adrs != 0)\
		{\
			Mem_Copy((u8*)adrs, copy_buffer, copy_amount);\
		}\
		else\
		{\
			goto OUT_OF_MEMORY;\
		}\
	}
	
	serialization_lalloc.clear();
	
	u32 version = 2;
	WRITE(version, u32);
	WRITE(event_container.events->count, u32);
	WRITE(event_container.day_event_count, u32);
	for(Event_State* e = Begin(event_container.events); e < End(event_container.events); ++e)
	{
		// event name
		WRITE(e->name.lenght, u32);
		if(e->name.lenght)
		{
			WRITE_BLOCK(e->name.buffer, e->name.lenght);
		}
		
		// event text
		WRITE(e->event_text.lenght, u32);
		if(e->event_text.lenght)
		{
			WRITE_BLOCK(e->event_text.buffer, e->event_text.lenght);
		}
		
		WRITE(e->participents->count, u32);
		for(Participent* p = Begin(e->participents); p < End(e->participents); ++p)
		{
			WRITE(p->reqs->count, u32);
			for(Participation_Requirement* req = Begin(p->reqs); req < End(p->reqs); ++req)
			{
				WRITE(req->type, Participation_Requirement::Type);
				switch(req->type)
				{
					case Participation_Requirement::Type::character_stat:
					{
						WRITE(req->stat_numerical_relation, Numerical_Relation);
						WRITE(req->stat_relation_target, u16);
						WRITE(req->stat, Character_Stat);
					
					}break;
					
					case Participation_Requirement::Type::mark_item:
					case Participation_Requirement::Type::mark_personal:
					{
						WRITE(req->mark.lenght, u32);
						if(req->mark.lenght)
						{
							WRITE_BLOCK(req->mark.buffer, req->mark.lenght);
						}
						WRITE(req->mark_exists, Exists_Statement);
					
					}break;
					
					default:
					{
						Terminate;
					}
				}
			}
			
			WRITE(p->cons->count, u32);
			for(Event_Consequens* con = Begin(p->cons); con < End(p->cons); ++con)
			{
				WRITE(con->type, Event_Consequens::Type);
				switch(con->type)
				{
					case Event_Consequens::Type::death:
					{
						if(con->items_are_inherited)
						{
							WRITE(con->str.lenght, u32);
							if(con->str.lenght)
							{
								WRITE_BLOCK(con->str.buffer, con->str.lenght);
							}							
						}
						else
						{
							WRITE(0, u32);
						}
					}break;
					
					case Event_Consequens::Type::gains_mark:
					case Event_Consequens::Type::loses_mark:
					{
						WRITE(con->mark_type, Mark_Type);
						WRITE(con->str.lenght, u32);
						if(con->str.lenght)
						{
							WRITE_BLOCK(con->str.buffer, con->str.lenght);
						}
						
					}break;
					
					case Event_Consequens::Type::stat_change:
					{
						WRITE(con->stat_change_amount, u8);
						WRITE(con->stat, Character_Stat::Stats);
					}break;
					
					default:
					{
						Terminate;
						break;
					}
				}
			}
		}
	}
	
	#undef WRITE
	#undef WRITE_BLOCK
	
	u32 buffer_size = serialization_lalloc.get_used_capacity();
	if(buffer_size)
	{
		platform->Create_Directory("Kamppaniat");
		platform->Write_File("Kamppaniat/test.event", (u8*)serialization_lalloc.memory, buffer_size);
	}
	return;
	
	OUT_OF_MEMORY:
	Assert(serialization_lalloc.capacity);
	
	serialization_lalloc.init(platform, serialization_lalloc.capacity * 2);
	Serialize_Campaign(event_container, platform);
}


//TODO: get the minim alloc sizes from some constants.
static bool Load_Campaign(
	Events_Container* container,
	Allocator_Shell* allocator,
	Platform_Calltable* platform)
{
	#define READ(type) *(type*)(buffer + read_head); read_head += sizeof(type); Assert(read_head <= buffer_size);
	#define READ_TEXT(lenght) (char*)(buffer + read_head); read_head += lenght; Assert(read_head <= buffer_size);
	
	u32 buffer_size = 0;
	if(!platform->Get_File_Size("Kamppaniat/test.event", &buffer_size))
		return false;

	serialization_lalloc.clear();
	
	if(serialization_lalloc.get_free_capacity() < buffer_size)
		serialization_lalloc.init(platform, buffer_size);
	
	u8* buffer = (u8*)serialization_lalloc.push(buffer_size);
	u32 read_head = 0;
	
	*container = {};
	
	bool load_success_full;
	
	if(platform->Read_File("Kamppaniat/test.event", (u8*)buffer, buffer_size))
	{
		load_success_full = true;
		
		u32 version = READ(u32);
		if(version != 2)
		{
			return false;
		}
		
		u32 event_count = READ(u32);
		container->day_event_count = READ(u32);
		
		Assert(container->day_event_count <= event_count);
		
		if(event_count)
		{
			container->events = Create_Dynamic_Array<Event_State>(allocator, event_count);
			container->events->count = event_count;
			
			for(Event_State* e = Begin(container->events); e < End(container->events); ++e)
			{
				u32 event_name_lenght = READ(u32);
				if(event_name_lenght)
				{
					char* event_name_buffer = READ_TEXT(event_name_lenght);	
					Init_String(&e->name, allocator, event_name_buffer, event_name_lenght);					
				}
				else
				{
					Init_String(&e->name, allocator, u32(0));
				}
				
				u32 event_text_lenght = READ(u32);
				if(event_text_lenght)
				{
					char* event_text_buffer = READ_TEXT(event_text_lenght);
					Init_String(&e->event_text, allocator, event_text_buffer, event_text_lenght);					
				}
				else
				{
					Init_String(&e->event_text, allocator, u32(0));
				}
				
				u32 participent_count = READ(u32);
				e->participents = Create_Dynamic_Array<Participent>(allocator, Max(u32(4), participent_count));
				e->participents->count = participent_count;
				Participent* parti_buffer = Begin(e->participents);
				
				for(u32 p = 0; p < participent_count; ++p)
				{
					Participent* participent = parti_buffer + p;
					
					// Requirements block
					{
						u32 req_count = READ(u32);
						participent->reqs = Create_Dynamic_Array<Participation_Requirement>(
							allocator, 
							Max(u32(3), 
							req_count));
						
						participent->reqs->count = req_count;
						
						Participation_Requirement* begin = Begin(participent->reqs);
						Participation_Requirement* end = End(participent->reqs);
						for(Participation_Requirement* req = begin; req < end; ++req)
						{
							*req = {};
							req->type = READ(Participation_Requirement::Type);
							switch(req->type)
							{
								case Participation_Requirement::Type::character_stat:
								{
									req->stat_numerical_relation = READ(Numerical_Relation);
									req->stat_relation_target = READ(u16);
									req->stat = READ(Character_Stat);
								}break;
								
								case Participation_Requirement::Type::mark_item:
								case Participation_Requirement::Type::mark_personal:
								{
									u32 mark_lenght = READ(u32);
									if(mark_lenght)
									{
										char* mark_buffer = READ_TEXT(mark_lenght);
										Init_String(&req->mark, allocator, mark_buffer, mark_lenght);					
									}
									else
									{
										Init_String(&req->mark, allocator, u32(0));
									}
									req->mark_exists = READ(Exists_Statement);
								
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
						participent->cons 
							= Create_Dynamic_Array<Event_Consequens>(allocator, Max(u32(3), con_count));
						participent->cons->count = con_count;
						
						Event_Consequens* begin = Begin(participent->cons);		
						Event_Consequens* end = End(participent->cons);
					
						for(Event_Consequens* con = begin; con < end; ++con)
						{
							*con = {};
							con->type = READ(Event_Consequens::Type);
							switch(con->type)
							{
								case Event_Consequens::Type::death:
								{
									u32 inherit_lenght = READ(u32);
									if(inherit_lenght)
									{
										con->items_are_inherited = true;
										char* inherit_text = READ_TEXT(inherit_lenght);
										Init_String(&con->str, allocator, inherit_text, inherit_lenght);
									}
									else
									{
										con->items_are_inherited = false;
										Init_String(&con->str, allocator, u32(0));
									}
									
								}break;
								
								case Event_Consequens::Type::gains_mark:
								case Event_Consequens::Type::loses_mark:
								{
									con->mark_type = READ(Mark_Type);
									
									u32 mark_lenght = READ(u32);
									if(mark_lenght)
									{
										char* mark_text = READ_TEXT(mark_lenght);
										Init_String(&con->str, allocator, mark_text, mark_lenght);
									}
									else
									{
										Init_String(&con->str, allocator, u32(0));
									}
									
								}break;
								
								case Event_Consequens::Type::stat_change:
								{
									con->stat_change_amount = READ(u8);
									con->stat = READ(Character_Stat::Stats);
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
		}
		else
		{
			load_success_full = true;
			container->events = Create_Dynamic_Array<Event_State>(allocator, 1);			
		}
	}
	
	return load_success_full;
	
	#undef READ
	#undef READ_TEXT
}

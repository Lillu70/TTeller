
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


static inline Event_State* Push_New_Event(
	Dynamic_Array<Event_State>** event_darray,
	Allocator_Shell* allocator,
	char* def_name = "Uusi tapahtuma")
{
	Assert(event_darray);
	
	Event_State* event = Push(event_darray, allocator);
	event->participents = Create_Dynamic_Array<Participent>(allocator, 4);
	Init_String(&event->name, allocator, def_name);
	Init_String(&event->event_text, allocator, 128);
	
	return event;
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


static void Serialize_Campaign(
	Dynamic_Array<Event_State>* events, 
	Linear_Allocator* inter_mem, 
	Platform_Calltable* platform)
{
	/*
		File descrition:
		- 4 bytes for version - must 1 atm.
		- 4 bytes for event count.
		
		Per event:
			- 4 bytes for name lenght.
			- followed by the event name characters in 1-byte ascii.
			
			- 4 bytes for the event text lenght.
			- followed by the event text characters in 1-byte ascii.
			
			- 4 bytes for the participent count.
			
			Per participent:
				- 4 bytes for the req count.
				Per req:
					- 1 byte for type:
					if type is character start:
						- 1 byte for numerical relation.
						- 2 bytes for stat relation target.
						- sizeof(Character start) bytes for the stat.
					if type is mark:
						- 4 bytes for name lenght
						- followed by the mark characters in 1-byte ascii.
				
				- 4 bytes for the con count.
				Per con:
					- 1 byte for the type:
					if type is death:
						-1 byte for inheritance
	*/
	
	Linear_Allocator write_buffer = Clone_From_Linear_Allocator_Free_Space(inter_mem);
	
	#define WRITE(X, type) *(type*)write_buffer.push(sizeof(type)) = X
	
	u32 version = 1;
	WRITE(version, u32);
	WRITE(events->count, u32);
	for(Event_State* e = Begin(events); e < End(events); ++e)
	{
		// event name
		WRITE(e->name.lenght, u32);
		if(e->name.lenght)
		{
			Mem_Copy((u8*)write_buffer.push(e->name.lenght), e->name.buffer, e->name.lenght);	
		}
		
		// event text
		WRITE(e->event_text.lenght, u32);
		if(e->event_text.lenght)
		{
			Mem_Copy(
				(u8*)write_buffer.push(e->event_text.lenght), 
				e->event_text.buffer, 
				e->event_text.lenght);
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
							Mem_Copy(
								(u8*)write_buffer.push(req->mark.lenght), 
								req->mark.buffer, 
								req->mark.lenght);
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
								Mem_Copy(
									(u8*)write_buffer.push(con->str.lenght), 
									con->str.buffer, 
									con->str.lenght);
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
							Mem_Copy(
								(u8*)write_buffer.push(con->str.lenght), 
								con->str.buffer, 
								con->str.lenght);
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
	
	u32 buffer_size = write_buffer.get_used_capacity();
	if(buffer_size)
	{
		platform->Create_Directory("Kamppaniat");
		platform->Write_File("Kamppaniat/test.event", (u8*)write_buffer.memory, buffer_size);
	}
	
	#undef WRITE
}


//TODO: get the minim alloc sizes from some constants.
static Dynamic_Array<Event_State>* Load_Campaign(
	Allocator_Shell* allocator,
	Linear_Allocator inter_mem, 
	Platform_Calltable* platform)
{
	#define READ(type) *(type*)(buffer + read_head); read_head += sizeof(type); Assert(read_head <= buffer_size);
	#define READ_TEXT(lenght) (char*)(buffer + read_head); read_head += lenght; Assert(read_head <= buffer_size);
	
	u32 buffer_size = 0;
	platform->Get_File_Size("Kamppaniat/test.event", &buffer_size);
	
	u8* buffer = (u8*)inter_mem.push(buffer_size);	
	u32 read_head = 0;
	
	Dynamic_Array<Event_State>* result = 0;
	if(platform->Read_File("Kamppaniat/test.event", (u8*)buffer, buffer_size))
	{
		u32 version = READ(u32);
		Assert(version == 1);
		
		u32 event_count = READ(u32);
		if(event_count)
		{
			result = Create_Dynamic_Array<Event_State>(allocator, event_count);
			result->count = event_count;
			
			for(Event_State* e = Begin(result); e < End(result); ++e)
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
			result = Create_Dynamic_Array<Event_State>(allocator, 1);			
		}
	}
	
	return result;
	
	#undef READ
}

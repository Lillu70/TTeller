
// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

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
            if(con->str.lenght)
            {
                String_View view = Create_String_View(&con->str);
                result.inheritance_target = Convert_String_View_Into_U32(view);;
            }
            else
            {
                result.inheritance_target = 0;
            }
            
        }break;
        
        case Event_Consequens_Type::stat_change:
        {
            result.change_amount = con->stat_change_amount;
            result.stat = con->stat;
        }break;
        
        case Event_Consequens_Type::gains_mark:
        {
            result.duration = con->mark_duration;
        } // Fall through!
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



static u32 Convert_Editor_Campaign_Into_Game_Format(
    Game_State* game,
    Events_Container* event_container,
    Platform_Calltable* platform,
    Allocator_Shell* allocator)
{
    Assert(game);
    Assert(event_container);
    Assert(allocator);
    
    *game = {};
    
    u32 errors = 0;
    
    if(event_container->events->count == 0)
        errors |= Game_Errors::does_not_contain_any_events;
    
    if(event_container->events->count == event_container->day_event_count)
        errors |= Game_Errors::does_not_contain_a_night_event;
    
    if(event_container->day_event_count == 0)
        errors |= Game_Errors::does_not_contain_a_day_event;
    
    if(!errors)
    {
        u32 alloc_size = 0;
        u32 requirements_data_size = 0;
        u32 events_data_size = 0;
        
        Mark_Hash_Table marks = Create_Mark_Hash_Table(allocator, 64);
        
        // First figure out the memory required for the game state.
        {
            alloc_size += event_container->campaign_name.lenght + 1;
            alloc_size += sizeof(u32) * event_container->events->count * 2;
            events_data_size += sizeof(Event_Header) * event_container->events->count;
            requirements_data_size += sizeof(Req_GM_Header) * event_container->events->count;
            
            for(each(Editor_Event*, e,event_container->events))
            {
                u32 participant_header_size = sizeof(Participant_Header) * e->participents->count;
                
                events_data_size += participant_header_size;
                events_data_size += sizeof(Global_Mark_Consequence_GM) * e->global_mark_cons->count;
                events_data_size += e->name.lenght + 1;
                events_data_size +=    e->event_text.lenght + 1;
                
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
        
        alloc_size += requirements_data_size;                       // req data
        alloc_size += events_data_size;                             // events data
        
        alloc_size += sizeof(u32) * marks.unique_element_count;     // mark table
        alloc_size += marks.total_unique_string_lenght;             // mark data
        
        game->memory = allocator->push(alloc_size);
        Linear_Allocator mem = Create_Linear_Allocator(game->memory, alloc_size);
        
        // Create mark data and table.
        {
            if(marks.total_unique_string_lenght)
            {
                Linear_Allocator mark_data = Create_Linear_Allocator(
                    mem.push(marks.total_unique_string_lenght), 
                    marks.total_unique_string_lenght);
                
                game->mark_data = (char*)mark_data.memory;            
                u32 mark_table_size = sizeof(u32) * marks.unique_element_count;
                Linear_Allocator mark_table = Create_Linear_Allocator(
                    mem.push(mark_table_size), 
                    mark_table_size);
                
                game->mark_table = Table{marks.unique_element_count, mark_table.memory};
                
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
                            
                            u32 offset = u32(buffer - game->mark_data);
                            
                            *mark_table.push<u32>() = offset;
                        }
                    }
                    
                    bucket = bucket->next_bucket;
                }
                
                Assert(!mark_data.get_free_capacity());
            }
        }
        
        game->campaign_name = (char*)mem.push(event_container->campaign_name.lenght + 1);
        Mem_Copy(
            game->campaign_name, 
            event_container->campaign_name.buffer, 
            event_container->campaign_name.lenght + 1);
        
        Linear_Allocator req_data = {};
        Linear_Allocator req_table_day = {};
        Linear_Allocator req_table_night = {};
        
        if(requirements_data_size)
        {
            game->req_data = (Req_GM_Header*)mem.push(requirements_data_size);
            req_data = Create_Linear_Allocator(game->req_data, requirements_data_size);        
        }
        
        u32 req_table_day_size = sizeof(u32) * event_container->day_event_count;
        
        if(req_table_day_size)
        {
            game->req_table_day = Table{event_container->day_event_count, mem.push(req_table_day_size)};
            req_table_day = Create_Linear_Allocator(game->req_table_day.memory, req_table_day_size);        
        }
        
        u32 night_event_count = event_container->events->count - event_container->day_event_count;
        u32 req_table_night_size = sizeof(u32) * night_event_count;
        
        if(req_table_night_size)
        {
            game->req_table_night = Table{night_event_count, mem.push(req_table_night_size)};
            req_table_night = Create_Linear_Allocator(game->req_table_night.memory,req_table_night_size);        
        }
        
        game->events_data = (Event_Header*)mem.push(events_data_size);
        Linear_Allocator events_data = Create_Linear_Allocator(game->events_data, events_data_size);
        
        game->event_table_day = Table{game->req_table_day.count, mem.push(req_table_day_size)};
        Linear_Allocator event_table_day 
            = Create_Linear_Allocator(game->event_table_day.memory, req_table_day_size);
        
        
        game->event_table_night = Table{night_event_count, mem.push(req_table_night_size)};
        Linear_Allocator event_table_night
            = Create_Linear_Allocator(game->event_table_night.memory, req_table_night_size);

        Assert(!mem.get_free_capacity());
        
        // Slot in the data
        {
            u32 i = 0;
            for(each(Editor_Event*, e, event_container->events))
            {
                Req_GM_Header* req_header = req_data.push<Req_GM_Header>();

                {
                    u32 offset = u32((u8*)req_header - (u8*)game->req_data);
                    if(i < event_container->day_event_count)
                        *req_table_day.push<u32>() = offset;
                    else
                        *req_table_night.push<u32>() = offset;
                }
                
                req_header->participant_count = e->participents->count;
                req_header->global_req_count = e->global_mark_reqs->count;
                
                for(each(Global_Mark_Requirement*, gmr, e->global_mark_reqs))
                {
                    *req_data.push<Global_Mark_Requirement_GM>() = Convert_To_GM(gmr, &marks);
                }
                
                Event_Header* event_header = events_data.push<Event_Header>();
                
                {
                    u32 offset = u32((u8*)event_header - (u8*)game->events_data);
                    if(i < event_container->day_event_count)
                        *event_table_day.push<u32>() = offset;
                    else
                        *event_table_night.push<u32>() = offset;
                }
                
                event_header->event_name = String_View
                {
                    (char*)events_data.push(e->name.lenght + 1), 
                    e->name.lenght
                };
                
                // name and text
                
                Mem_Copy(event_header->event_name.buffer, e->name.buffer, e->name.lenght + 1);
                event_header->event_text 
                    = String_View{(char*)events_data.push(e->event_text.lenght + 1), e->event_text.lenght};
                Mem_Copy(event_header->event_text.buffer, e->event_text.buffer, e->event_text.lenght + 1);
                event_header->size = e->name.lenght + e->event_text.lenght + 2;
                // ---
                
                event_header->global_con_count = e->global_mark_cons->count;
                for(auto gmc = Begin(e->global_mark_cons); gmc < End(e->global_mark_cons); ++gmc)
                    *events_data.push<Global_Mark_Consequence_GM>() = Convert_To_GM(gmc, &marks);
                
                
                u32 req_count = 0;
                u32 con_count = 0;
                for(each(Participent*, p, e->participents))
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
                
                i += 1;
            }
        }
        
        Free_Mark_Hash_Table(&marks, allocator);
        
        Assert(!req_data.get_free_capacity());
        Assert(!req_table_day.get_free_capacity());
        Assert(!req_table_night.get_free_capacity());
        Assert(!events_data.get_free_capacity());
        Assert(!event_table_day.get_free_capacity());
        Assert(!event_table_night.get_free_capacity());
        
        game->total_player_count = 0;
        game->live_player_count = 0;
        game->language = Language::finnish;
        
        game->player_names 
            = Create_Dynamic_Array<Game_Player_Name_FI>(allocator, u32(6));
        
        game->player_images = Create_Dynamic_Array<Player_Image>(allocator, u32(6));
        
        game->global_marks = Create_Dynamic_Array<Mark_GM>(allocator, 4);
        game->active_events = Create_Dynamic_Array<Event>(allocator, 4);
        
        Init_String(&game->display_text, allocator, 1);
        
        char seed_buffer[12] = {0};
        char* init_seed = U32_To_Char_Buffer((u8*)&seed_buffer, (u32)platform->Get_Time_Stamp());
        
        Init_String(&game->seed_input, allocator, init_seed);
        
    }
    
    return errors;
}


static void Get_Active_Event_Ref_And_Header(
    Game_State* game,
    u32 event_idx, 
    Event** out_ref, 
    Event_Header** out_header)
{
    Assert(out_ref || out_header);
    
    Table* active_event_table = (game->active_event_list == Event_List::day)?
        &game->event_table_day : 
        &game->event_table_night;
    
    Assert(event_idx < game->active_events->count);
    
    Event* active_event = Begin(game->active_events) + event_idx;
    
    u32 offset = *((u32*)active_event_table->memory + active_event->event_idx);
    Event_Header* active_event_header = (Event_Header*)((u8*)game->events_data + offset);
    
    if(out_ref)
        *out_ref = active_event;
    
    if(out_header)
        *out_header = active_event_header;
}


static void Generate_Display_Text(Game_State* game)
{
    Event_Header* event_header;
    Event* event_ref;
    Get_Active_Event_Ref_And_Header(game, game->display_event_idx, &event_ref, &event_header);
    
    u32 display_text_size = 1;
    
    enum class Mode
    {
        search,
        seek_number,
        seek_form,
        seek_bender
    };
    Mode mode = Mode::search;
    
    String_View number_view = {};
    u32 player_idx = 0;
    bool a_switch = 0;
    bool a_switch_owerride;
    String* full_name = 0;
    
    game->display_text.lenght = 0;
    *game->display_text.buffer = 0;
    
    if(event_header->event_text.buffer)
    {
        for(char* cptr = event_header->event_text.buffer;; ++cptr)
        {
            char c = *cptr;
            switch(mode)
            {
                case Mode::search:
                {
                    if(c == '/')
                    {
                        cptr += 1; // NOTE: skip the next character for now.
                        
                        mode = Mode::seek_number;
                        number_view.buffer = cptr + 1;
                        Assert(*number_view.buffer >= '0' && *number_view.buffer <= '9');
                        number_view.lenght = 0;
                    }
                    else if(c)
                    {
                        game->display_text += c;
                    }
                    
                }break;
                
                case Mode::seek_number:
                {
                    if(c < '0' || c > '9')
                    {
                        cptr -= 1; // NOTE GO back!
                        
                        u32 participant_idx = Convert_String_View_Into_U32(number_view) - 1;
                        Assert(participant_idx < event_ref->participant_count);
                        
                        player_idx = *(event_ref->player_indices + participant_idx);
                        Assert(player_idx < game->live_player_count);
                        
                        mode = Mode::seek_form;
                    }
                    else
                    {
                        number_view.lenght += 1;
                    }
                }break;
                
                case Mode::seek_form:
                {
                    Game_Player_Name_FI* name = Begin(game->player_names) + player_idx;
                    a_switch = false;
                    a_switch_owerride = name->special_char_override;
                    full_name = &name->full_name;
                    switch(c)
                    {
                        case ':':
                        {
                            mode = Mode::seek_bender;
                            if(name->variant_name_1.lenght)
                            {
                                u32 required_memory = game->display_text.lenght + name->variant_name_1.lenght + 1;
                                Reserve_String_Memory(&game->display_text, required_memory);
                                char* dest = game->display_text.buffer + game->display_text.lenght;
                                
                                Mem_Copy(dest, name->variant_name_1.buffer, name->variant_name_1.lenght);
                                game->display_text.lenght += name->variant_name_1.lenght;
                            }
                        }break;
                        
                        case ';':
                        {
                            mode = Mode::seek_bender;
                            if(name->variant_name_2.lenght)
                            {
                                u32 required_memory = game->display_text.lenght + name->variant_name_2.lenght + 1;
                                Reserve_String_Memory(&game->display_text, required_memory);
                                char* dest = game->display_text.buffer + game->display_text.lenght;
                                
                                Mem_Copy(dest, name->variant_name_2.buffer, name->variant_name_2.lenght);
                                game->display_text.lenght += name->variant_name_2.lenght;
                            }
                        }break;
                        
                        default:
                        {
                            
                            mode = Mode::search;
                            cptr -= 1; // NOTE GO back!
                            
                            if(name->full_name.lenght)
                            {
                                u32 required_memory = game->display_text.lenght + name->full_name.lenght + 1;
                                Reserve_String_Memory(&game->display_text, required_memory);
                                char* dest = game->display_text.buffer + game->display_text.lenght;
                                
                                Mem_Copy(dest, name->full_name.buffer, name->full_name.lenght);
                                game->display_text.lenght += name->full_name.lenght;
                            }
                        }
                    }
                    
                }break;
                
                case Mode::seek_bender:
                {
                    if(c == '/')
                    {
                        switch(*(cptr + 1))
                        {
                            case 'k':
                            case 'K':
                            {
                                mode = Mode::search;
                                cptr -= 1;
                            }break;
                        }
                        a_switch = true;
                    }
                    else if(c == 0 || c == ' ' || c == '\n')
                    {
                        mode = Mode::search;
                        cptr -= 1; // NOTE GO back!
                    }
                    else
                    {
                        if(a_switch)
                        {
                            if(full_name->lenght)
                            {
                                char lc = *(full_name->buffer + (full_name->lenght - 1));
                                
                                if(c == 'a' && ((lc == '\xE4' || lc == '\xF6') || a_switch_owerride))
                                    c = '\xE4';
                                
                                if(c)
                                    game->display_text += c;
                            }
                        }
                        else if(c)
                        {
                            game->display_text += c;
                        }
                    }
                    
                }break;
            }
            
            if(!*cptr)
                break;
        }
        
        game->display_text.buffer[game->display_text.lenght + 1] = 0;
    }    
}


static inline void Hollow_Player_Name_FI(Game_Player_Name_FI* player)
{
    Assert(player);

    player->full_name.free();
    player->variant_name_1.free();
    player->variant_name_2.free();
}


static inline void Hollow_Game_Player(Game_Player* player, Allocator_Shell* allocator)
{
    Assert(player->marks);
    allocator->free(player->marks);
}


static void Delete_Game(Game_State* game, Allocator_Shell* allocator)
{
    if(game->total_player_count)
    {
        for(u32 i = 0; i < game->total_player_count; ++i)
            Hollow_Game_Player(game->players + i, allocator);
        
        allocator->free(game->players);
    }
    
    allocator->free(game->memory);
    
    for(Player_Image* i = Begin(game->player_images); i < End(game->player_images); ++i)
    {
        if(i->image.buffer)
        {
            allocator->free(i->image.buffer);
            i->file_path.free();
        }
    }
    allocator->free(game->player_images);
        
    for(auto n = Begin(game->player_names); n < End(game->player_names); ++n)
        Hollow_Player_Name_FI(n);

    allocator->free(game->player_names);
    
    for(each(Event*, e, game->active_events))
        allocator->free(e->player_indices);
    
    allocator->free(game->active_events);
    allocator->free(game->global_marks);
    
    game->display_text.free();
    game->seed_input.free();
    
    *game = {};
}


static void Reset_Game(Game_State* game, Allocator_Shell* allocator)
{
    if(game->total_player_count)
    {
        for(u32 i = 0; i < game->total_player_count; ++i)
            Hollow_Game_Player(game->players + i, allocator);
        
        allocator->free(game->players);
    }
    game->players = 0;
    
    game->player_images->count = game->total_player_count;
    game->player_names->count = game->total_player_count;
    
    game->total_player_count = 0;
    game->live_player_count = 0;
    game->global_marks->count = 0;
}


static void Begin_Game(Game_State* game, Platform_Calltable* platform, Allocator_Shell* allocator)
{
    Assert(!game->live_player_count);
    Assert(!game->players);
    Assert(game->player_images->count);
    
    game->rm.noise_position = 0;
    if(game->seed_input.lenght)
    {
        String_View seed_input_view = Create_String_View(&game->seed_input);
        game->rm.seed = (i32)Convert_String_View_Into_U32(seed_input_view);
    }
    else
    {
        game->rm.seed = (i32)platform->Get_Time_Stamp();
    }
    
    // NOTE: Using the images array as that is application language agnostic.
    
    game->total_player_count = game->player_images->count;
    game->live_player_count = game->total_player_count;
    
    u32 s = sizeof(*game->players) + sizeof(*game->player_assignement_table);
    u32 alloc_size = s * game->live_player_count;
    
    u32 event_count = Max(game->event_table_day.count, game->event_table_night.count);
    alloc_size += sizeof(*game->event_assignement_table) * event_count;
    
    void* m = allocator->push(alloc_size);
    game->players = (Game_Player*)m;
    game->player_assignement_table = (u32*)(game->players + game->live_player_count);
    
    game->event_assignement_table_size = event_count;
    game->event_assignement_table = game->player_assignement_table + game->live_player_count;
    
    for(Game_Player* p = game->players; p < game->players + game->live_player_count; ++p)
    {
        for(u32 i = 0; i < Array_Lenght(p->stats); ++i)
            p->stats[i] = Game_Player::starting_stat_value;
        
        p->alive = true;
        p->marks = Create_Dynamic_Array<Mark_GM>(allocator, 4);
    }
    
    game->day_counter = 1;
}


static void Create_Player_Name_FI(Game_State* game, Allocator_Shell* allocator)
{
    Assert(game);
    Assert(game->memory);
    Assert(game->player_names);

    Game_Player_Name_FI* name = Push(&game->player_names, allocator);
    
    Init_String(&name->full_name, allocator, u32(0));
    Init_String(&name->variant_name_1, allocator, u32(0));
    Init_String(&name->variant_name_2, allocator, u32(0));
}


static inline void Free_Event_List(Dynamic_Array<Event>* events, Allocator_Shell* allocator)
{
    Assert(events);
    
    for(auto e = Begin(events); e < End(events); ++e)
        allocator->free(e->player_indices);
    
    allocator->free(events);
}


static inline bool Numerical_Relation_Match(i8 value, i8 relation_target, Numerical_Relation relation)
{
    bool result = false;
    
    switch(relation)
    {
        case Numerical_Relation::equal_to:
        {
            result = relation_target == value;
        }break;
        
        case Numerical_Relation::greater_than:
        {
            result = value > relation_target;
        }break;
        
        case Numerical_Relation::greater_than_equals:
        {
            result = value  >=  relation_target;
        }break;
        
        case Numerical_Relation::less_than:
        {
            result = value < relation_target;
        }break;
        
        case Numerical_Relation::less_than_equals:
        {
            result = value <= relation_target;
        }break;
    }
    
    return result;
}


static inline void Search_For_Mark(
    Dynamic_Array<Mark_GM>* marks, 
    Mark_Type type,
    u32 search_mark_idx,
    Exists_Statement* out_exists, 
    i8* out_duration,
    u32* out_mark_idx = 0)
{
    u32 i = 0;
    for(each(auto, mark, marks))
    {
        if(mark->idx == search_mark_idx && mark->type == type)
        {
            *out_exists = Exists_Statement::does_have;
            *out_duration = mark->duration;
            if(out_mark_idx)
                *out_mark_idx = i;
            
            return;
        }
        
        i += 1;
    }
    
    *out_exists = Exists_Statement::does_not_have;
    *out_duration = 0;
}



static inline bool Player_Satisfies_Requirement(
    Game_Player* player, 
    Participation_Requirement_GM* req)
{
    bool result = false;
    
    switch(req->type)
    {
        case Participation_Requirement_Type::character_stat:
        {
            i8 stat = player->stats[u32(req->stat_type)];
            result = Numerical_Relation_Match(stat, req->relation_target, req->numerical_relation);
        }break;
        
        case Participation_Requirement_Type::mark_item:
        {
            Exists_Statement exists;
            i8 duration;
            Search_For_Mark(player->marks, Mark_Type::item, req->mark_idx, &exists, &duration);
            
            if(req->mark_exists != exists)
            {
                result = false;
            }
            else if(exists == Exists_Statement::does_have)
            {
                // Note: Zero is infinite in this context!
                if(!duration)
                    duration = I8_MAX;
                
                result = Numerical_Relation_Match(
                    duration, 
                    req->relation_target, 
                    req->numerical_relation);
            }
            else
            {
                result = true;
            }
            
        }break;
        
        case Participation_Requirement_Type::mark_personal:
        {
            Exists_Statement exists;
            i8 duration;
            Search_For_Mark(player->marks, Mark_Type::personal, req->mark_idx, &exists, &duration);
            
            if(req->mark_exists != exists)
            {
                result = false;
            }
            else if(exists == Exists_Statement::does_have)
            {
                // Note: Zero is infinite in this context!
                if(!duration)
                    duration = I8_MAX;
                
                result = Numerical_Relation_Match(
                    duration, 
                    req->relation_target, 
                    req->numerical_relation);
            }
            else
            {
                result = true;
            }
            
        }break;
        
        default:
        {
            Terminate;
        };
    }
    
    return result;
}


static inline void Hollow_Active_Events(Game_State* game, Allocator_Shell* allocator)
{
    for(each(Event*, e, game->active_events))
            allocator->free(e->player_indices);
        
    game->active_events->count = 0;
}


static bool Assign_Events_To_Participants(
    Game_State* game,
    Event_List event_list,
    Allocator_Shell* allocator)
{
    Hollow_Active_Events(game, allocator);
    
    game->active_event_list = event_list;
    
    Assert(game->live_player_count);
    Assert(game->event_assignement_table_size);
    
    for(u32 i = 0; i < game->live_player_count; ++i)
        game->player_assignement_table[i] = i;
    
    // shuffle player indexes.
    for(u32 i = 0; i < game->live_player_count * 1.5; ++i)
    {
        u32 first = game->rm.random_u32(game->live_player_count);
        u32 second = game->rm.random_u32(game->live_player_count);
        
        Swap(game->player_assignement_table + first, game->player_assignement_table + second);
    }

    // Fill the event table
    u32 event_count = 
        (event_list == Event_List::day)? game->event_table_day.count : game->event_table_night.count;
        
    for(u32 i = 0; i < event_count; ++i)
        game->event_assignement_table[i] = i;
    
    u32* req_table_memory = (u32*)((event_list == Event_List::day)? 
        game->req_table_day.memory : game->req_table_night.memory);
    
    
    // Assign players
    u32 unassigned_player_count = game->live_player_count;
    while(unassigned_player_count)
    {    
        u32 untested_event_count = event_count;
        for(;;)
        {
            u32 event_to_test_table_idx = game->rm.random_u32(untested_event_count);
            u32 event_to_test_idx = game->event_assignement_table[event_to_test_table_idx];
            
            u32 offset =  *(req_table_memory + event_to_test_idx);
            Req_GM_Header* req_header = (Req_GM_Header*)((u8*)game->req_data + offset);
            
            // Test event ------------
            bool not_rejected = true;
            // Test player count
            if(unassigned_player_count < req_header->participant_count)
                not_rejected = false;
            
            // Test global reqs
            for(u32 i = 0; i < req_header->global_req_count; ++i)
            {
                auto global_req = (Global_Mark_Requirement_GM*)(req_header + 1) + i;
                
                Exists_Statement exists;
                i8 duration;
                Search_For_Mark(
                    game->global_marks, 
                    Mark_Type::global, 
                    global_req->mark_idx, 
                    &exists, 
                    &duration);
                
                if(global_req->mark_exists != exists)
                {
                    not_rejected = false;
                    break;
                }
                
                if(exists == Exists_Statement::does_have)
                {
                    // Note: Zero is infinite in this context!
                    if(!duration)
                        duration = I8_MAX;
                    
                    if(!Numerical_Relation_Match(
                        duration, 
                        global_req->relation_target, 
                        global_req->numerical_relation))
                    {
                        not_rejected = false;
                        break;
                    }
                }
            }
            
            bool commited = false;
            u32* slots = 0;
            
            if(not_rejected)
            {
                // Test players against participant requirements.
                u32 parti_req_mem_size = 
                    req_header->global_req_count * sizeof(Global_Mark_Requirement_GM);
                
                u8* parti_req_memory = ((u8*)(req_header + 1)) + parti_req_mem_size;
                
                u32 filled_slot_count = 0;
                u32 slots_size = sizeof(u32) * req_header->participant_count;
                slots = (u32*)allocator->push(slots_size);
                Mem_Zero(slots, slots_size);
                
                for(u32 p = 0; p < unassigned_player_count; ++p)
                {
                    u32 player_idx = game->player_assignement_table[p];
                    Game_Player* player = game->players + player_idx;
                    
                    Participant_Header* parti = (Participant_Header*)parti_req_memory;
                    for(u32 s = 0; s < req_header->participant_count; ++s)
                    {
                        if(!slots[s]) // NOTE: skip filled slots
                        {
                            Participation_Requirement_GM* reqs_begin 
                                = (Participation_Requirement_GM*)(parti + 1);
                            
                            Participation_Requirement_GM* reqs_end = reqs_begin + parti->count;
                            
                            bool satisfies_reqs = true;
                            
                            for(auto req = reqs_begin; req < reqs_end; ++req)
                            {
                                bool satisfies = Player_Satisfies_Requirement(player, req);
                                if(!satisfies)
                                {
                                    satisfies_reqs = false;
                                    break;
                                }
                            }
                            
                            if(satisfies_reqs)
                            {
                                slots[s] = player_idx + 1;
                                filled_slot_count += 1;
                                break;
                            }
                        }
                        
                        u32 reqs_size = sizeof(Participation_Requirement_GM) * parti->count;
                        parti = (Participant_Header*)((u8*)parti + reqs_size + sizeof(Participant_Header));
                    }
                    
                    
                    // Commit!
                    if(filled_slot_count == req_header->participant_count)
                    {
                        // Remove selected players from the pool.
                        for(u32 i = 0; i < req_header->participant_count; ++i)
                        {
                            u32 player_idx = slots[i] - 1;
                            slots[i] = player_idx;
                            
                            // Find player idx in the assignement table and nuke it.
                            bool player_found = false;
                            for(u32 a = 0; a < unassigned_player_count; ++a)
                            {
                                if(game->player_assignement_table[a] == player_idx)
                                {
                                    unassigned_player_count -= 1;
                                    u32 last_entry = game->player_assignement_table[unassigned_player_count];
                                    game->player_assignement_table[a] = last_entry;
                                    
                                    game->player_assignement_table[unassigned_player_count] = U32_MAX;
                                    
                                    player_found = true;
                                }
                            }
                            
                            Assert(player_found);                         
                        }
                        
                        Event* e = Push(&game->active_events, allocator);
                        
                        Assert(event_to_test_idx < event_count);
                        
                        e->event_idx = event_to_test_idx;
                        e->participant_count = filled_slot_count;
                        
                        // NOTE: Takes owenership of the ptr!
                        e->player_indices = slots;
                        commited = true;
                        break;
                    }
                }
            }
            
            // REJECTED!
            if(!commited)
            {
                if(slots)
                    allocator->free(slots);

                // NOTE: Unordered remove from table.
                untested_event_count -= 1;
                
                u32 last_table_entry = game->event_assignement_table[untested_event_count];
                game->event_assignement_table[event_to_test_table_idx] = last_table_entry;
                
                // There are no more events that could be tested.
                if(!untested_event_count)
                    return false;
            }
            else
            {
                break;
            }
        }
    }
    
    Assert(game->active_events->count);
    
    game->display_event_idx = 0;
    Generate_Display_Text(game);
    
    return true;
}


static void Give_Player_Mark(Game_Player* player, Mark_GM mark, Allocator_Shell* allocator)
{
    Assert(u32(mark.type) < u32(Mark_Type::COUNT));
    
    Exists_Statement exists;
    i8 duration;
    u32 mark_idx;
    Search_For_Mark(player->marks, mark.type, mark.idx, &exists, &duration, &mark_idx);

    if(u8(exists))
    {
        Mark_GM* existing_mark = Begin(player->marks) + mark_idx;
        
        // NOTE: 0 duration is infinete!
        if(existing_mark->duration == 0 || mark.duration == 0)
            existing_mark->duration = 0;
        else
            existing_mark->duration = Max(existing_mark->duration, mark.duration);
    }
    else
    {
        *Push(&player->marks, allocator) = mark;
    }
}


static void Resolve_Current_Event_Set(Game_State* game, Allocator_Shell* allocator)
{
    // Apply active event consequences
    for(u32 e = 0; e < game->active_events->count; ++e)
    {
        Event_Header* event_header;
        Event* event_ref;
        Get_Active_Event_Ref_And_Header(game, e, &event_ref, &event_header);
        
        
        Global_Mark_Consequence_GM* gcon_begin =
            (Global_Mark_Consequence_GM*)((u8*)event_header +
            sizeof(*event_header) +
            event_header->size);
        
        for(Global_Mark_Consequence_GM* gcon = gcon_begin;
            gcon < gcon_begin + event_header->global_con_count;
            ++gcon)
        {
            // If the global mark already exists set the duration to the bigger one.
            
            Exists_Statement exists;
            i8 duration;
            u32 mark_idx;
            Search_For_Mark(
                game->global_marks, 
                Mark_Type::global, 
                gcon->mark_idx, 
                &exists, 
                &duration, 
                &mark_idx);
            
            if(u8(exists))
            {
                Mark_GM* mark = Begin(game->global_marks) + mark_idx;
                
                // NOTE: 0 duration is infinete!
                if(mark->duration == 0 || gcon->mark_duration == 0)
                    mark->duration = 0;
                else
                    mark->duration = Max(mark->duration, gcon->mark_duration);
            }
            else
            {
                Mark_GM m;
                m.type = Mark_Type::global;
                m.duration = gcon->mark_duration;
                m.idx = gcon->mark_idx;
                
                *Push(&game->global_marks, allocator) = m;
            }
        }
        
        u32 offset = event_header->global_con_count * sizeof(Global_Mark_Consequence_GM);
        
        Participant_Header* parti_head = (Participant_Header*)((u8*)gcon_begin + offset);
        // NOTE: parti_head count referes to the amount of consequences per participant.
        
        for(u32 p = 0; p < event_ref->participant_count; ++p)
        {
            if(parti_head->count)
            {
                u32 slot = *(event_ref->player_indices + p);
                Game_Player* player = game->players + slot;
                
                Event_Consequens_GM* cons = (Event_Consequens_GM*)(parti_head + 1);
                for(u32 c = 0; c < parti_head->count; ++c)
                {
                    Event_Consequens_GM* con = cons + c;
                    
                    switch(con->type)
                    {
                        case Event_Consequens_Type::death:
                        {
                            player->alive = false;
                            
                            if(con->inheritance_target)
                            {
                                u32 target_idx = con->inheritance_target - 1;
                                u32 islot = *(event_ref->player_indices + target_idx);
                                Game_Player* inheritor = game->players + islot;    

                                Assert(inheritor);
                                
                                if(inheritor != player)
                                {
                                    for(each(Mark_GM*, mark, player->marks))
                                    {
                                        if(mark->type == Mark_Type::item)
                                            Give_Player_Mark(inheritor, *mark, allocator);
                                    }                                    
                                }
                            }
                        }break;
                        
                        case Event_Consequens_Type::stat_change:
                        {
                            i8* stat = player->stats + u32(con->stat);
                            *stat += con->change_amount;
                            *stat = Max(STAT_MINIMUM, Min(*stat, STAT_MAXIMUM));
                        }break;
                        
                        case Event_Consequens_Type::gains_mark:
                        {
                            Mark_GM con_mark = {con->mark_type, con->duration, con->mark_idx};
                            Give_Player_Mark(player, con_mark, allocator);
                        }break;
                        
                        case Event_Consequens_Type::loses_mark:
                        {
                            Exists_Statement exists;
                            i8 duration;
                            u32 mark_idx;
                            Search_For_Mark(
                                player->marks, 
                                con->mark_type,
                                con->mark_idx, 
                                &exists, 
                                &duration, 
                                &mark_idx);
                            
                            if(u8(exists))
                                Unordered_Remove(player->marks, mark_idx);
                            
                        }break;
                        
                        default:
                        {
                            Terminate;
                        };
                    }
                }
            }
            
            u32 cons_size = parti_head->count * sizeof(Event_Consequens_GM);
            parti_head = (Participant_Header*)((u8*)(parti_head) + sizeof(*parti_head) + cons_size);
        }
    }
    
    
    // Cull dead players.
    // NOTE: This will sometimes result in incorrect player death display order.
    // If multiple people die in same set they will show up in creation order not event display order.
    
    // CONSIDER: There is no simple fix to this, that I can think of. But is it worth fixing?
    
    for(u32 i = 0; i < game->live_player_count; ++i)
    {
        Game_Player* player = game->players + i;
        if(!player->alive)
        {
            game->live_player_count -= 1;
            
            // Unordered remove of all the player components.
            // NOTE: The underdered remove darray function can't be used here, since it does the 
            // removing based on live player count not the actual array count.
            // Additionally The underdered remove function would override data. For replay we don't want that.
           
           
            // ---
            Game_Player* last_player = game->players + game->live_player_count;
            Game_Player temp_player = *player;
            *player = *last_player;
            *last_player = temp_player;
            // ---
            
            // ---
            Game_Player_Name_FI* player_names = Begin(game->player_names); 
            Game_Player_Name_FI* player_name = player_names + i; 
            Game_Player_Name_FI* last_player_name = player_names + game->live_player_count; 
            Game_Player_Name_FI temp_player_name = *player_name;
            *player_name = *last_player_name;
            *last_player_name = temp_player_name;
            // ---
            
            
            // ---
            Player_Image* player_images = Begin(game->player_images); 
            Player_Image* player_image = player_images + i; 
            Player_Image* last_player_image = player_images + game->live_player_count; 
            Player_Image temp_player_image = *player_image;
            *player_image = *last_player_image;
            *last_player_image = temp_player_image;
            // ---
            
            i -= 1;
        }
    }
}


static void Tickdown_Marks(Game_State* game)
{
    {
        Mark_GM* gmarks = Begin(game->global_marks);
        
        for(u32 i = 0; i < game->global_marks->count; ++i)
        {
            Mark_GM* gmark = gmarks + i;
            if(gmark->duration)
            {
                gmark->duration -= 1;
                if(!gmark->duration)
                {
                    Unordered_Remove(game->global_marks, i--);
                }
            }
        }        
    }
    
    
    for(u32 p = 0; p < game->live_player_count; ++p)
    {
        Game_Player* player = game->players + p;
        Mark_GM* marks = Begin(player->marks);
        
        for(u32 i = 0; i < player->marks->count; ++i)
        {
            Mark_GM* mark = marks + i;
            if(mark->duration > 0)
            {
                mark->duration -= 1;
                if(mark->duration == 0)
                {
                    Unordered_Remove(player->marks, i--);
                }
            }
        }
    }
}


static inline bool Fill_Empty_Names(Game_State* game, Allocator_Shell* allocator)
{    
    u32 filler_name_list_lenght = Array_Lenght(s_filler_name_list_FI);
    
    bool result = true;
    
    for(each(Game_Player_Name_FI*, name, game->player_names))
    {
        if(!name->full_name.lenght && !name->variant_name_1.lenght && !name->variant_name_2.lenght)
        {
            u32 name_idx = game->rm.random_u32(filler_name_list_lenght);
            
            bool success = false;
            u32 attempt_count = 0;
            while(!success && attempt_count < filler_name_list_lenght)
            {
                
                u32 offset = (name_idx + attempt_count) % filler_name_list_lenght;
                Filler_Name_Data_FI* filler_name = s_filler_name_list_FI + offset;

                for(each(Game_Player_Name_FI*, cmp_name, game->player_names))
                    if(C_STR_Compare(cmp_name->full_name.buffer, filler_name->full_name))
                        goto FAIL;
                
                
                // Success!
                {
                    Set_String_Text(&name->full_name, filler_name->full_name);
                    Set_String_Text(&name->variant_name_1, filler_name->variant_name_1);
                    Set_String_Text(&name->variant_name_2, filler_name->variant_name_2);
                    
                    name->special_char_override = filler_name->special_char_override;
                    
                    success = true;
                }
                continue;
                
                
                FAIL:
                attempt_count += 1;
            }
            
            if(!success)
            {
                result = false;
                break;
            }
        }
    }
    
    return result;
}
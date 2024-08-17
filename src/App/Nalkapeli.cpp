

// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#pragma once


static inline Editor_Event* Active_Event(Editor_State* editor)
{
    Editor_Event* result = Begin(editor->event_container.events) + editor->active_event_index;
    return result;
}


static inline void Generate_Folder_Hierarchy(Platform_Calltable* platform)
{
    if(!platform->Create_Directory(data_folder_path))
    {
        Terminate;
    }
    
    if(!platform->Create_Directory(campaigns_folder_path))
    {
        Terminate;
    }
}

// lol these silly functions :D
static inline void Make_Requirement_Hollow(Participation_Requirement* req)
{
    req->mark.free();
}


static inline void Make_Consequense_Hollow(Event_Consequens* con)
{
    con->str.free();
}


static void Make_Global_Mark_Requirement_Hollow(Global_Mark_Requirement* gmr)
{
    gmr->mark.free();
}


static void Make_Global_Mark_Consequence_Hollow(Global_Mark_Consequence* gmc)
{
    gmc->mark.free();
}


static inline Participent* Create_Participent(
    Dynamic_Array<Participent>** darray, 
    Allocator_Shell* allocator)
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


static inline void Hollow_Invalid_Event_Filter_Results(
    Dynamic_Array<Invalid_Event_Filter_Result>* filter_results)
{
    if(filter_results)
    {
        for(each(Invalid_Event_Filter_Result*, iefr, filter_results))
            iefr->name.free();
    }
}


static inline void Delete_Global_Mark_Requirement(
    Dynamic_Array<Global_Mark_Requirement>* darray,
    u32 idx_to_delete)
{
    Assert(idx_to_delete < darray->count);
    
    Global_Mark_Requirement* buffer = Begin(darray);
    Global_Mark_Requirement* gmr = buffer + idx_to_delete;
    Make_Global_Mark_Requirement_Hollow(gmr);
    
    Remove_Element_From_Packed_Array(buffer, &darray->count, sizeof(*gmr), idx_to_delete);
}


static inline void Delete_Global_Mark_Consequence(
    Dynamic_Array<Global_Mark_Consequence>* darray,
    u32 idx_to_delete)
{
    Assert(idx_to_delete < darray->count);
    
    Global_Mark_Consequence* buffer = Begin(darray);
    Global_Mark_Consequence* gmc = buffer + idx_to_delete;
    Make_Global_Mark_Consequence_Hollow(gmc);
    
    Remove_Element_From_Packed_Array(buffer, &darray->count, sizeof(*gmc), idx_to_delete);
}


static inline void Delete_Participent(
    u32 idx_to_delete, 
    Dynamic_Array<Participent>* darray, 
    Allocator_Shell* allocator)
{
    Assert(idx_to_delete < darray->count);
    
    Hollow_Participent(idx_to_delete, darray, allocator);
    
    Participent* buffer = Begin(darray);
    u32 size = sizeof(*buffer);
    Remove_Element_From_Packed_Array(buffer, &darray->count, size, idx_to_delete);    
}


static inline void Init_Event_Takes_Name_Ownership(
    Editor_Event* event, 
    Allocator_Shell* allocator, 
    String* name)
{
    event->issues = {};
    
    event->participents = Create_Dynamic_Array<Participent>(allocator, 4);
    event->global_mark_reqs = Create_Dynamic_Array<Global_Mark_Requirement>(allocator, 4);
    event->global_mark_cons = Create_Dynamic_Array<Global_Mark_Consequence>(allocator, 4);
    
    Init_String(&event->event_text, allocator, 128);
    event->name = *name;
}


static void Delete_All_Participants_From_Event(Editor_Event* event, Allocator_Shell* allocator)
{
    for(u32 i = 0; i < event->participents->count; ++i)
        Hollow_Participent(i, event->participents, allocator);

    event->participents->count = 0;
}


static void Delete_Editor_Event(
    Dynamic_Array<Editor_Event>* darray, 
    Allocator_Shell* allocator, 
    u32 idx_to_delete, 
    bool remove_from_array=true)
{
    Assert(idx_to_delete < darray->count);
    
    Editor_Event* buffer = Begin(darray);
    Editor_Event* element = buffer + idx_to_delete;
    
    Delete_All_Participants_From_Event(element, allocator);
    allocator->free(element->participents);
    
    for(each(Global_Mark_Requirement*, gmr, element->global_mark_reqs))
        Make_Global_Mark_Requirement_Hollow(gmr);
    
    for(each(Global_Mark_Consequence*, gmc, element->global_mark_cons))
        Make_Global_Mark_Consequence_Hollow(gmc);
    
    allocator->free(element->global_mark_reqs);
    allocator->free(element->global_mark_cons);
    
    if(element->name.buffer)
        element->name.free();
    element->event_text.free();
    
    // Optional toggle, to allow optimization when deleting all events, normaly this should be "true"
    if(remove_from_array)
        Remove_Element_From_Packed_Array(buffer, &darray->count, sizeof(*element), idx_to_delete);
}


static Dynamic_Array<Invalid_Event_Filter_Result>* Unordered_Filter_Prolematic_Events(
    Events_Container* event_container, 
    Allocator_Shell* allocator)
{
    Dynamic_Array<Invalid_Event_Filter_Result>* result = 0;
    
    Editor_Event* events = Begin(event_container->events);
    
    for(u32 i = 0; i < event_container->events->count; ++i)
    {
        Editor_Event* event = events + i;
        
        if(event->issues.errors)
        {
            if(!result)
            {
                result = Create_Dynamic_Array<Invalid_Event_Filter_Result>(allocator, 4);
            }
            
            Invalid_Event_Filter_Result IEFI;
            IEFI.name = event->name;
            event->name.buffer = 0;
            IEFI.reasons = event->issues.errors;
            
            *Push(&result, allocator) = IEFI;
            
            Delete_Editor_Event(event_container->events, allocator, i, false);
            Unordered_Remove(event_container->events, i);
            
            i -= 1;
        }
    }
    
    return result;
}


static void Delete_Event_Container(Events_Container* event_container, Allocator_Shell* allocator)
{
    event_container->campaign_name.free();
    
    if(event_container->events)
    {
        for(u32 i = 0; i < event_container->events->count; ++i)
            Delete_Editor_Event(event_container->events, allocator, i, false);
        
        allocator->free(event_container->events);
        
        *event_container = {};
    }
}


static void Init_Participation_Requirement(
    Participation_Requirement* req,
    Participation_Requirement_Type type,
    Allocator_Shell* allocator,
    u32 mark_str_capacity = Participation_Requirement::initial_mark_capacity)
{
    *req = Participation_Requirement();
    req->type = type;
    
    switch(type)
    {
        case Participation_Requirement_Type::mark_personal:
        case Participation_Requirement_Type::mark_item:
        {
            Init_String(&req->mark, allocator, mark_str_capacity);
            req->mark_exists = Exists_Statement::does_have;
            req->numerical_relation = Numerical_Relation::greater_than_equals;
            req->relation_target = 1;
        }break;
    }
}


static void Init_Event_Consequense(
    Event_Consequens* con, 
    Event_Consequens_Type type,
    Allocator_Shell* allocator,
    u32 mark_str_capacity =  Participation_Requirement::initial_mark_capacity)
{
    *con = Event_Consequens();
    con->type = type;
    
    u32 init_capacity = mark_str_capacity;
    
    switch(type)
    {
        case Event_Consequens_Type::stat_change:
        {
            con->stat_change_amount = 1;
        }break;
        
        case Event_Consequens_Type::death:
        {
            init_capacity = 4;
            
        } // Fall through!
        case Event_Consequens_Type::gains_mark:
        case Event_Consequens_Type::loses_mark:
        {
            con->mark_duration = 0;
            Init_String(&con->str, allocator, init_capacity);
            break;
        }break;
        
        default:
        {
            Terminate;
        }
    }
}


static void Init_Event_Container_Takes_Name_Ownership(
    Events_Container* ec, 
    Allocator_Shell* allocator, 
    String* name)
{
    ec->campaign_name = *name;
    ec->events = Create_Dynamic_Array<Editor_Event>(allocator, 12);
    
    *name = {};
}


static String Generate_Unique_Name(Editor_Event* events, u32 event_count, Allocator_Shell* allocator)
{
    char* def_name = "Uusi tapahtuma";
    u32 def_name_len = Null_Terminated_Buffer_Lenght(def_name);
    String unique_name = Create_String(allocator, def_name);
    for(u32 attempt_count = 1; ;++attempt_count)
    {
        bool name_is_unique = true;
        
        for(Editor_Event* e = events; e < events + event_count; ++e)
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


static String Create_Campaign_Full_Path(
    String* source_name, 
    Platform_Calltable* platform, 
    Allocator_Shell* allocator)
{
    char exe_path[260];
    u32 exe_path_lenght = platform->Get_Executable_Path(exe_path, Array_Lenght(exe_path));
    
    Assert(exe_path_lenght);
    
    String result;
    Init_String(
        &result, 
        allocator,
        exe_path,
        campaigns_folder_path, 
        source_name->buffer,
        campaign_file_extension);
    
    return result;
}


static void Update_Editor_Event_Text_Issues(Editor_Event* event)
{
    Editor_Event_Issues issues = event->issues;
    
    u32 text_error_mask = 
        Event_Errors::text_references_uninvolved_participant |
        Event_Errors::escape_character_without_valid_followup |
        Event_Errors::participant_identifier_is_not_a_number;
    
    Inverse_Bit_Mask(&issues.errors, text_error_mask);
    
    u32 text_warning_mask = 
        Event_Warnings::text_is_empty | 
        Event_Warnings::text_does_not_reference_every_participant;
    
    Inverse_Bit_Mask(&issues.warnings, text_warning_mask);
    
    if(event->event_text.lenght)
    {
        u32 array_size = sizeof(bool) * event->participents->count;
        bool* player_referenced_array = (bool*)s_scrach_buffer.push(array_size);
        Mem_Zero(player_referenced_array, array_size);
        
        enum class Mode
        {
            search,
            seek_number,
            seek_space,
            seek_form,
            seek_bender
        };
        Mode mode = Mode::search;
        String_View number_view = {};
        
        for(char* cptr = event->event_text.buffer; *cptr; ++cptr)
        {
            char c = *cptr;
            switch(mode)
            {
                case Mode::search:
                {
                    if(c == '/')
                    {
                        cptr += 1;
                        switch(*cptr)
                        {
                            case 'K':
                            case 'k':
                            {
                                number_view.buffer = cptr + 1;
                                if(*number_view.buffer >= '0' && *number_view.buffer <= '9')
                                {
                                    number_view.lenght = 0;
                                    mode = Mode::seek_number;                                    
                                }
                                else
                                {
                                    issues.errors |= Event_Errors::participant_identifier_is_not_a_number;
                                    mode = Mode::seek_space;
                                }
                                
                            }break;
                            
                            default:
                            {
                                issues.errors |= Event_Errors::escape_character_without_valid_followup;
                            }
                        }
                    }
                    
                }break;
                
                case Mode::seek_space:
                {
                    if(c == ' ' || c == '\n')
                    {
                        mode = Mode::search;
                    }
                }break;
                
                case Mode::seek_number:
                {
                    char c2 = *(cptr + 1);
                    
                    if(c < '0' || c > '9' || c2 == 0)
                    {
                        if(c2)
                            cptr -= 1; // NOTE GO back!                        
                        
                        u32 participant_idx = Convert_String_View_Into_U32(number_view) - 1;
                        
                        if(participant_idx < event->participents->count)
                            *(player_referenced_array + participant_idx) = true;
                        else
                            issues.errors |= Event_Errors::text_references_uninvolved_participant;
                        
                        mode = Mode::seek_space;
                    }
                    else
                    {
                        number_view.lenght += 1;
                    }
                }break;
            }
        }
        
        for(u32 i = 0; i < event->participents->count; ++i)
        {
            if(!*(player_referenced_array + i))
            {
                issues.warnings |= Event_Warnings::text_does_not_reference_every_participant;
                break;
            }
        }
    }
    else
    {
        issues.warnings |= Event_Warnings::text_is_empty;
        issues.warnings |= Event_Warnings::text_does_not_reference_every_participant;
    }
    
    event->issues = issues;
}


static void Update_Editor_Event_Participant_Issues(Editor_Event* event)
{
    Editor_Event_Issues issues = event->issues;
    
    u32 text_error_mask = 
        Event_Errors::contains_impossiple_global_requirement |
        Event_Errors::contains_impossiple_requirement | 
        Event_Errors::has_no_participants | 
        Event_Errors::cotaints_empty_mark_field_in_global_requirements |
        Event_Errors::cotaints_empty_mark_field_in_requirements;
    
    Inverse_Bit_Mask(&issues.errors, text_error_mask);
    
    u32 text_warning_mask = Event_Warnings::contains_irrelevant_requirement;
    
    Inverse_Bit_Mask(&issues.warnings, text_warning_mask);
    
    for(each(Global_Mark_Requirement*, gmark, event->global_mark_reqs))
    {
        if(!gmark->mark.lenght)
        {
            issues.errors |= Event_Errors::cotaints_empty_mark_field_in_global_requirements;
        }
        
        if(gmark->mark_exists == Exists_Statement::does_have && 
            (
                (gmark->numerical_relation == Numerical_Relation::less_than &&
                gmark->relation_target == 1) 
                
                ||
                
                (gmark->numerical_relation == Numerical_Relation::greater_than &&
                gmark->relation_target == 3)
            ))
        {
            issues.errors |= Event_Errors::contains_impossiple_global_requirement;
        }
    }
    
    if(event->participents->count)
    {
        for(each(Participent*, parti, event->participents))
        {
            for(each(Participation_Requirement*, req, parti->reqs))
            {
                switch(req->type)
                {
                    case Participation_Requirement_Type::character_stat:
                    {
                        if((req->numerical_relation == Numerical_Relation::less_than &&
                            req->relation_target == STAT_MINIMUM) ||
                            (req->numerical_relation == Numerical_Relation::greater_than &&
                            req->relation_target == STAT_MAXIMUM))
                        {
                            issues.errors |= Event_Errors::contains_impossiple_requirement;
                        }
                    }break;
                    
                    case Participation_Requirement_Type::mark_item:
                    case Participation_Requirement_Type::mark_personal:
                    {
                        if(req->mark_exists == Exists_Statement::does_have && 
                            req->numerical_relation == Numerical_Relation::less_than &&
                            req->relation_target == STAT_MINIMUM + 1)
                        {
                            issues.warnings |= Event_Warnings::contains_irrelevant_requirement;
                        }
                        
                        if(!req->mark.lenght)
                            issues.errors |= Event_Errors::cotaints_empty_mark_field_in_requirements;
                    
                    }break;
                }
            }
        }
    }
    else
        issues.errors |= Event_Errors::has_no_participants;
    
    event->issues = issues;
}


static void Update_Editor_Event_Consequence_Issues(Editor_Event* event)
{
    Editor_Event_Issues issues = event->issues;
    
    u32 text_error_mask = 
        Event_Errors::cotaints_empty_mark_field_in_consequences | 
        Event_Errors::cotaints_empty_mark_field_in_global_consequences |
        Event_Errors::death_consequence_with_uninvolved_inheritor;
    
    Inverse_Bit_Mask(&issues.errors, text_error_mask);
    
    u32 text_warning_mask = 
        Event_Warnings::death_consequence_with_self_inheriting | 
        Event_Warnings::death_consequence_with_zero_as_inheritor;
    
    Inverse_Bit_Mask(&issues.warnings, text_warning_mask);
    
    for(each(Global_Mark_Consequence*, gmark, event->global_mark_cons))
    {
        if(!gmark->mark.lenght)
        {
            issues.errors |= Event_Errors::cotaints_empty_mark_field_in_global_consequences;
            break;
        }
    }
    
    u32 i = 0;
    for(each(Participent*, parti, event->participents))
    {
        for(each(Event_Consequens*, con, parti->cons))
        {
            switch(con->type)
            {
                case Event_Consequens_Type::death:
                {
                    if(con->items_are_inherited)
                    {
                        if(con->str.lenght)
                        {
                            String_View number_view = Create_String_View(&con->str);
                            u32 inheritor = Convert_String_View_Into_U32(number_view);
                            if(inheritor)
                            {
                                u32 inheritor_idx = inheritor - 1;
                                if(inheritor_idx >= event->participents->count)
                                {
                                    issues.errors |= Event_Errors::death_consequence_with_uninvolved_inheritor;
                                }
                                else if(inheritor_idx == i)
                                {
                                    issues.warnings |= Event_Warnings::death_consequence_with_self_inheriting;
                                }
                            }
                            else
                            {
                                issues.warnings |= Event_Warnings::death_consequence_with_zero_as_inheritor;
                            }
                        }
                        else
                        {
                            issues.warnings |= Event_Warnings::death_consequence_with_zero_as_inheritor;                    
                        }                        
                    }
                }break;
                
                case Event_Consequens_Type::gains_mark:
                case Event_Consequens_Type::loses_mark:
                {
                    if(!con->str.lenght)
                        issues.errors |= Event_Errors::cotaints_empty_mark_field_in_consequences;
                    
                }break;
            }
        }
        
        i += 1;
    }

    event->issues = issues;
}


static void Update_Editor_Event_Issues(Editor_Event* event)
{
    Update_Editor_Event_Participant_Issues(event);
    Update_Editor_Event_Text_Issues(event);
    Update_Editor_Event_Consequence_Issues(event);
}


static void Serialize_Campaign(
    Events_Container event_container,
    Platform_Calltable* platform)
{    
    /*
     ---- Version History ---- 
    1->2: Added seperation of day and night events.
    2->3: Added duration as part of marks for consequenses and requirements.
    */
    
    //TODO: Write like a file description thing or something maybe?
    
    #define WRITE(X, type)\
    {\
        void* adrs = s_scrach_buffer.safe_push(sizeof(type));\
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
        void* adrs = s_scrach_buffer.safe_push(copy_amount);\
        if(adrs != 0)\
        {\
            Mem_Copy((u8*)adrs, copy_buffer, copy_amount);\
        }\
        else\
        {\
            goto OUT_OF_MEMORY;\
        }\
    }
    
    Assert(event_container.campaign_name.buffer);
    
    s_scrach_buffer.clear();
    
    u32 version = 3;
    WRITE(version, u32);
    WRITE(event_container.events->count, u32);
    WRITE(event_container.day_event_count, u32);
    for(Editor_Event* e = Begin(event_container.events); e < End(event_container.events); ++e)
    {
        // event name
        WRITE(e->name.lenght, u32);
        if(e->name.lenght)
        {
            WRITE_BLOCK(e->name.buffer, e->name.lenght);
        }
        
        // global reqs
        WRITE(e->global_mark_reqs->count, u32);
        for(Global_Mark_Requirement* gmr = Begin(e->global_mark_reqs); gmr < End(e->global_mark_reqs); ++gmr)
        {
            WRITE(gmr->mark_exists, Exists_Statement);
            WRITE(gmr->relation_target, i8);
            WRITE(gmr->numerical_relation, Numerical_Relation);
            WRITE(gmr->mark.lenght, u32);
            if(gmr->mark.lenght)
            {
                WRITE_BLOCK(gmr->mark.buffer, gmr->mark.lenght);
            }
        }
        
        // global cons
        WRITE(e->global_mark_cons->count, u32);
        for(Global_Mark_Consequence* gmc = Begin(e->global_mark_cons); gmc < End(e->global_mark_cons); ++gmc)
        {
            WRITE(gmc->mark_duration, i8);
            WRITE(gmc->mark.lenght, u32);
            if(gmc->mark.lenght)
            {
                WRITE_BLOCK(gmc->mark.buffer, gmc->mark.lenght);
            }
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
                WRITE(req->type, Participation_Requirement_Type);
                
                WRITE(req->numerical_relation, Numerical_Relation);
                WRITE(req->relation_target, u16);
                
                switch(req->type)
                {
                    case Participation_Requirement_Type::character_stat:
                    {
                        WRITE(req->stat_type, Character_Stats);
                    
                    }break;
                    
                    case Participation_Requirement_Type::mark_item:
                    case Participation_Requirement_Type::mark_personal:
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
                WRITE(con->type, Event_Consequens_Type);
                switch(con->type)
                {
                    case Event_Consequens_Type::death:
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
                    
                    case Event_Consequens_Type::gains_mark:
                    case Event_Consequens_Type::loses_mark:
                    {
                        WRITE(con->mark_type, Mark_Type);
                        WRITE(con->mark_duration, i8);
                        WRITE(con->str.lenght, u32);
                        if(con->str.lenght)
                        {
                            WRITE_BLOCK(con->str.buffer, con->str.lenght);
                        }
                        
                    }break;
                    
                    case Event_Consequens_Type::stat_change:
                    {
                        WRITE(con->stat_change_amount, i8);
                        WRITE(con->stat, Character_Stats);
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
    
    u32 buffer_size = s_scrach_buffer.get_used_capacity();
    if(buffer_size)
    {
        // TODO: Use a part of the s_scrach_buffer allocation for the full path buffer,
        // to avoid this allocation bussines.
        
        String full_path = Create_Campaign_Full_Path(
            &event_container.campaign_name, 
            platform,
            event_container.campaign_name.alloc);
        
        bool success = platform->Write_File(
            full_path.buffer, 
            (u8*)s_scrach_buffer.memory, 
            buffer_size);
        
        Assert(success);
        full_path.free();
    }
    return;
    
    OUT_OF_MEMORY:
    Assert(s_scrach_buffer.capacity);
    
    s_scrach_buffer.init(platform, s_scrach_buffer.capacity * 2);
    Serialize_Campaign(event_container, platform);
}

#define READ(type) *(type*)(buffer + read_head); read_head += sizeof(type); Assert(read_head <= buffer_size);
#define READ_TEXT(lenght) (char*)(buffer + read_head); read_head += lenght; Assert(read_head <= buffer_size);
    
static inline void Load_Campaign_V2(
    Events_Container* container,
    Allocator_Shell* allocator,
    u8* buffer,
    u32 buffer_size,
    u32 read_head)
{
    u32 event_count = READ(u32);
    container->day_event_count = READ(u32);
    
    Assert(container->day_event_count <= event_count);
    
    container->events = Create_Dynamic_Array<Editor_Event>(allocator, Max(u32(4), event_count));
    container->events->count = event_count;
    
    for(Editor_Event* e = Begin(container->events); e < End(container->events); ++e)
    {
        e->global_mark_reqs = Create_Dynamic_Array<Global_Mark_Requirement>(allocator, 4);
        e->global_mark_cons = Create_Dynamic_Array<Global_Mark_Consequence>(allocator, 4);
        
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
                    req->type = READ(Participation_Requirement_Type);
                    switch(req->type)
                    {
                        case Participation_Requirement_Type::character_stat:
                        {
                            req->numerical_relation = READ(Numerical_Relation);
                            req->relation_target = READ(u16);
                            
                            struct Character_Stat_V2
                            {
                                enum class Stats : u16
                                {
                                    body = 0,
                                    mind
                                };
                                Stats type;
                                u16 value;
                            };
                            
                            Character_Stat_V2 s = READ(Character_Stat_V2);
                            req->stat_type = (Character_Stats)s.type;
                        }break;
                        
                        case Participation_Requirement_Type::mark_item:
                        case Participation_Requirement_Type::mark_personal:
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
                    con->type = READ(Event_Consequens_Type);
                    switch(con->type)
                    {
                        case Event_Consequens_Type::death:
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
                        
                        case Event_Consequens_Type::gains_mark:
                        case Event_Consequens_Type::loses_mark:
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
                        
                        case Event_Consequens_Type::stat_change:
                        {
                            con->stat_change_amount = READ(i8);
                            con->stat = (Character_Stats)READ(u16);
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


//TODO: get the minim alloc sizes from some constants.
static inline void Load_Campaign_V3(
    Events_Container* container,
    Allocator_Shell* allocator,
    u8* buffer,
    u32 buffer_size,
    u32 read_head)
{
    u32 event_count = READ(u32);
    container->day_event_count = READ(u32);
    
    Assert(container->day_event_count <= event_count);
    
    container->events = Create_Dynamic_Array<Editor_Event>(allocator, Max(u32(4), event_count));
    container->events->count = event_count;
    
    for(each(Editor_Event*, event, container->events))
    {
        // -- event name 
        u32 event_name_lenght = READ(u32);
        if(event_name_lenght)
        {
            char* event_name_buffer = READ_TEXT(event_name_lenght);    
            Init_String(&event->name, allocator, event_name_buffer, event_name_lenght);
        }
        else
        {
            Init_String(&event->name, allocator, u32(0));
        }
        
        // -- event reqs
        u32 global_req_count = READ(u32);
        
        event->global_mark_reqs 
            = Create_Dynamic_Array<Global_Mark_Requirement>(allocator, Max(u32(4), global_req_count));
        event->global_mark_reqs->count = global_req_count;
        
        for(each(Global_Mark_Requirement*, gmr, event->global_mark_reqs))
        {
            gmr->mark_exists = READ(Exists_Statement);
            gmr->relation_target = READ(i8);
            gmr->numerical_relation = READ(Numerical_Relation);
            u32 mark_lenght = READ(u32);
            if(mark_lenght)
            {
                char* gmr_mark_buffer = READ_TEXT(mark_lenght);    
                Init_String(&gmr->mark, allocator, gmr_mark_buffer, mark_lenght);
            }
            else
            {
                Init_String(&gmr->mark, allocator, u32(0));
            }
        }
        
        // -- event cons
        u32 global_con_count = READ(u32);
        event->global_mark_cons 
            = Create_Dynamic_Array<Global_Mark_Consequence>(allocator, Max(u32(4), global_con_count));
        event->global_mark_cons->count = global_con_count;
        
        for(each(Global_Mark_Consequence*, gmc, event->global_mark_cons))
        {
            gmc->mark_duration = READ(i8);
            u32 mark_lenght = READ(u32);
            if(mark_lenght)
            {
                char* gmc_mark_buffer = READ_TEXT(mark_lenght);    
                Init_String(&gmc->mark, allocator, gmc_mark_buffer, mark_lenght);
            }
            else
            {
                Init_String(&gmc->mark, allocator, u32(0));
            }
        }
        
        // -- event text
        u32 event_text_lenght = READ(u32);
        if(event_text_lenght)
        {
            char* event_text_buffer = READ_TEXT(event_text_lenght);
            Init_String(&event->event_text, allocator, event_text_buffer, event_text_lenght);
        }
        else
        {
            Init_String(&event->event_text, allocator, u32(0));
        }
        
        // -- participents
        u32 participent_count = READ(u32);
        event->participents = Create_Dynamic_Array<Participent>(allocator, Max(u32(4), participent_count));
        event->participents->count = participent_count;
        Participent* parti_buffer = Begin(event->participents);
        
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
                
                for(each(Participation_Requirement*, req, participent->reqs))
                {
                    *req = {};
                    req->type = READ(Participation_Requirement_Type);
                    
                    req->numerical_relation = READ(Numerical_Relation);
                    req->relation_target = READ(u16);
                    
                    switch(req->type)
                    {
                        case Participation_Requirement_Type::character_stat:
                        {
                            req->stat_type = READ(Character_Stats);
                        }break;
                        
                        case Participation_Requirement_Type::mark_item:
                        case Participation_Requirement_Type::mark_personal:
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
                    con->type = READ(Event_Consequens_Type);
                    switch(con->type)
                    {
                        case Event_Consequens_Type::death:
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
                        
                        case Event_Consequens_Type::gains_mark:
                        case Event_Consequens_Type::loses_mark:
                        {
                            con->mark_type = READ(Mark_Type);
                            con->mark_duration = READ(i8);
                            
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
                        
                        case Event_Consequens_Type::stat_change:
                        {
                            con->stat_change_amount = READ(i8);
                            con->stat = READ(Character_Stats);
                        }break;
                        
                        
                        default:
                        {
                            Terminate;
                        }
                    }
                }
            }
        }

        event->issues = {};
        Update_Editor_Event_Issues(event);
    }
}


static bool Load_Campaign(
    Events_Container* container,
    String* name,
    Allocator_Shell* allocator,
    Platform_Calltable* platform)
{
    Assert(!container->campaign_name.buffer);
    Assert(!container->events);
    
    String full_path = Create_Campaign_Full_Path(name, platform, allocator);
    
    u32 buffer_size = 0;
    bool load_successful = false;
    if(platform->Get_File_Size(full_path.buffer, &buffer_size))
    {
        s_scrach_buffer.clear();
        
        if(s_scrach_buffer.get_free_capacity() < buffer_size)
            s_scrach_buffer.init(platform, buffer_size);
        
        u8* buffer = (u8*)s_scrach_buffer.push(buffer_size);
        u32 read_head = 0;        
        
        *container = {};
        
        if(platform->Read_File(full_path.buffer, (u8*)buffer, buffer_size))
        {
            load_successful = true;
            u32 version = READ(u32);
            switch(version)
            {
                case 2:
                {
                    Load_Campaign_V2(container, allocator, buffer, buffer_size, read_head);
                }break;
                
                case 3:
                {
                    Load_Campaign_V3(container, allocator, buffer, buffer_size, read_head);
                }break;
                
                default:
                {
                    load_successful = false;
                }
            }
            
            if(load_successful)
            {
                Deep_Copy_String(&container->campaign_name, name);
            }
        }
    }
    
    full_path.free();
    return load_successful;
}

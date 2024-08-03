
#pragma once

static constexpr char* data_folder_path = "Tiedostot\\";
static constexpr char* campaigns_folder_path = "Tiedostot\\Kampanjat\\";
static constexpr char* campaign_file_extension = ".nalkmp";
static constexpr char* campaign_folder_wildcard_path = "Tiedostot\\Kampanjat\\*.nalkmp";

static constexpr i8 STAT_MINIMUM = 0;
static constexpr i8 STAT_MAXIMUM = 3;

namespace Event_Errors
{
    enum T : u32
    {
        text_references_uninvolved_participant           = 1 << 0,
        escape_character_without_valid_followup          = 1 << 1,
        participant_identifier_is_not_a_number           = 1 << 2,
        has_no_participants                              = 1 << 3,
        contains_impossiple_requirement                  = 1 << 4,
        contains_impossiple_global_requirement           = 1 << 5,
        cotaints_empty_mark_field_in_requirements        = 1 << 6,
        cotaints_empty_mark_field_in_global_requirements = 1 << 7,
        cotaints_empty_mark_field_in_consequences        = 1 << 8,
        cotaints_empty_mark_field_in_global_consequences = 1 << 9,
        death_consequence_with_uninvolved_inheritor      = 1 << 10,
    };
    
    char* names[] = 
    {
        "Tapahtumateksti viittaa osallistujaan joka ei ole tapahtumassa mukana!",
        "Merkki (/) ilman hyv\xE4ksytt\xE4v\xE4\xE4 jatko merkki\xE4!",
        "Osallistujan tunnistaja ei ole numero!",
        "Tapahtumassa ei ole osallitujia!",
        "Sis\xE4lt\xE4\xE4 mahdottoman vaatimuksen!",
        "Sis\xE4lt\xE4\xE4 mahdottoman yleis vaatimuksen!",
        "Sis\xE4lt\xE4\xE4 tyhj\xE4n merkki kent\xE4n vaatimuksissa!",
        "Sis\xE4lt\xE4\xE4 tyhj\xE4n merkki kent\xE4n yleis vaatimuksissa!",
        "Sis\xE4lt\xE4\xE4 tyhj\xE4n merkki kent\xE4n seuraamuksissa!",
        "Sis\xE4lt\xE4\xE4 tyhj\xE4n merkki kent\xE4n yleis seuraamuksissa!",
        "Kuolema seuraamus jossa tapahtumaan osallistumaton perij\xE4!"
    };
}


namespace Event_Warnings
{
    enum T : u32
    {
        text_is_empty                                   = 1 << 0,
        text_does_not_reference_every_participant       = 1 << 1,
        contains_irrelevant_requirement                 = 1 << 2,
        death_consequence_with_self_inheriting          = 1 << 3,
        death_consequence_with_zero_as_inheritor        = 1 << 4,
    };
    
    char* names[] = 
    {
        "Tapahtumateksti on tyhj\xE4.",
        "Tapahtumateksti ei viittaa jokaiseen osallistujaan.",
        "Sis\xE4lt\xE4\xE4 merkitsem\xE4tt\xF6m\xE4n vaatimuksen.",
        "Kuolema seuraamus jossa perij\xE4n\xE4 on itse kuolija.",
        "Kuolema seuraamus jossa perij\xE4n\xE4 on nolla."
    };
}


enum class Exists_Statement : u8
{
    does_not_have = 0,
    does_have
};


static const char* s_exists_statement_names[] =
{
    "Ei saa olla.",
    "T\xE4ytyy olla."
};


enum class Numerical_Relation : u8
{
    equal_to = 0,
    greater_than,
    greater_than_equals,
    less_than,
    less_than_equals,
};


static const char* s_numerical_relation_names[] = 
{
    "=",
    ">",
    ">=",
    "<",
    "<="
};


static const char* s_arithmatic_names[] = 
{
    "+",
    "-"
};


static const char* s_mark_type_names[] = 
{
    "Esine Merkki", 
    "Hahmo Merkki"
};


static const char* s_duration_names[] = 
{
    "Rajaton",
    "1 p\xE4iv\xE4",
    "2 p\xE4iv\xE4\xE4",
    "3 p\xE4iv\xE4\xE4",
};

enum class Mark_Type :u8
{
    global = U8_MAX,
    item = 0,
    personal,
    COUNT
};


enum class Character_Stats : u8
{
    body = 0,
    mind,
    COUNT
};

static const char* s_stat_names[] = {"Keho", "Mieli"};


struct Global_Mark_Requirement
{
    Exists_Statement mark_exists = Exists_Statement::does_have;
    i8 relation_target = 1;
    Numerical_Relation numerical_relation = Numerical_Relation::greater_than_equals;
    
    String mark = {};
};


struct Global_Mark_Consequence
{
    i8 mark_duration = 0;
    String mark = {};
};


enum class Participation_Requirement_Type : u8
{
    character_stat = 0,
    mark_item,
    mark_personal,
    COUNT
};


struct Participation_Requirement
{
    static inline const char* type_names[] = {"Ominaisuus", "Esine Merkki", "Hahmo Merkki"};
    
    Participation_Requirement_Type type;
    Exists_Statement mark_exists;
    Numerical_Relation numerical_relation = Numerical_Relation::greater_than_equals;
    
    Character_Stats stat_type;
    
    u16 relation_target = 1;

    String mark;
    
    static constexpr u32 initial_mark_capacity = 12;
};


enum class Event_Consequens_Type : u8
{
    death = 0,
    stat_change,
    gains_mark,
    loses_mark,
    COUNT
};


struct Event_Consequens
{
    static inline const char* type_names[] = 
    {
        "Kuolema",
        "Ominaisuus muutos",
        "Saa merkin",
        "Menett\xE4\xE4 merkin"
    };
    
    Event_Consequens_Type type = Event_Consequens_Type::death;
    
    Mark_Type mark_type = Mark_Type::item;
    bool items_are_inherited = true;
    union
    {
        i8 mark_duration;
        i8 stat_change_amount;
    };
    Character_Stats stat = Character_Stats::body;
    
    String str = {};
};


struct Participent
{
    static constexpr u32 max_requirements = 20;
    Dynamic_Array<Participation_Requirement>* reqs;
    
    static constexpr u32 max_consequenses = 20;
    Dynamic_Array<Event_Consequens>* cons;
};


struct Editor_Event_Issues
{
    u32 errors = 0;
    u32 warnings = 0;
};


struct Editor_Event
{
    static constexpr u32 max_participent_count = 100;
    
    Dynamic_Array<Participent>* participents;
    Dynamic_Array<Global_Mark_Requirement>* global_mark_reqs = 0;
    Dynamic_Array<Global_Mark_Consequence>* global_mark_cons = 0;
    
    String name;
    String event_text;
    
    Editor_Event_Issues issues = {};
};


struct Events_Container
{
    String campaign_name = {};
    
    // Day events are stored first, then night events.
    Dynamic_Array<Editor_Event>* events = 0;
    u32 day_event_count = 0;
};


struct Editor_State
{
    Events_Container event_container;
    
    u32 active_event_index = 0;
    u32 event_idx_to_delete = 0;
};


struct Invalid_Event_Filter_Result
{
    String name;
    u32 reasons;
};
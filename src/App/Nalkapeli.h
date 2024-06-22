
#pragma once

static constexpr char* data_folder_path = "Tiedostot\\";
static constexpr char* campaigns_folder_path = "Tiedostot\\Kampanjat\\";
static constexpr char* campaign_file_extension = ".nalkmp";
static constexpr char* campaign_folder_wildcard_path = "Tiedostot\\Kampanjat\\*.nalkmp";

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
	item = 0,
	personal,
	COUNT
};


struct Character_Stat
{
	enum class Stats : u8
	{
		body = 0,
		mind
	};
	
	static inline const char* stat_names[] = 
		{"Keho", "Mieli"};
	
	Stats type;
	u16 value;
};


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
	
	Character_Stat::Stats stat_type;
	
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
	Character_Stat::Stats stat = Character_Stat::Stats::body;
	
	String str = {};
};


struct Participent
{
	static constexpr u32 max_requirements = 20;
	Dynamic_Array<Participation_Requirement>* reqs;
	
	static constexpr u32 max_consequenses = 20;
	Dynamic_Array<Event_Consequens>* cons;
};


struct Event_State
{
	static constexpr u32 max_participent_count = 100;
	Dynamic_Array<Participent>* participents;
	Dynamic_Array<Global_Mark_Requirement>* global_mark_reqs = 0;
	Dynamic_Array<Global_Mark_Consequence>* global_mark_cons = 0;
	
	String name;
	String event_text;
};


struct Events_Container
{
	String campaign_name = {};
	
	// Day events are stored first, then night events.
	Dynamic_Array<Event_State>* events = 0;
	u32 day_event_count = 0;
};
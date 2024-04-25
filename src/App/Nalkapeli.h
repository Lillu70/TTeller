
#pragma once


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


enum class Mark_Type :u8
{
	item = 0,
	personal,
	COUNT
};


struct Character_Stat
{
	enum class Stats : u16
	{
		body = 0,
		mind
	};
	
	static inline const char* stat_names[] = 
		{"Keho", "Mieli"};
	
	Stats type;
	u16 value;
};


struct Participation_Requirement
{
	// TODO/CONSIDER: an union?
	
	static inline const char* type_names[] = 
		{"Ominaisuus", "Esine Merkki", "Hahmo Merkki"};
	
	enum class Type : u8
	{
		character_stat = 0,
		mark_item,
		mark_personal,
		COUNT
	};
	
	Type type;
	Numerical_Relation stat_numerical_relation;
	u16 stat_relation_target;
	Character_Stat stat;
	
	
	// Mark
	String mark;
	Exists_Statement mark_exists;
	
	static constexpr u32 initial_mark_capacity = 12;
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
	
	enum class Type : u8
	{
		death = 0,
		stat_change,
		gains_mark,
		loses_mark,
		COUNT
	};
	Type type = Type::death;
	
	Mark_Type mark_type = Mark_Type::item;
	bool items_are_inherited = true;
	i8 stat_change_amount = 1;
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
	String name;
	String event_text;
};
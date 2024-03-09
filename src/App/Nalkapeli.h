
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


enum class Numerical_Relation : u16
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
	
	enum class Type
	{
		character_stat = 0,
		mark_item,
		mark_personal,
		COUNT
	};
	
	Type type;
	
	u16 stat_relation_target;
	Numerical_Relation stat_numerical_relation;
	Character_Stat stat;
	
	// Mark
	String mark;
	Exists_Statement mark_exists;
	
	static constexpr u32 initial_mark_capacity = 12;
};


struct Participent
{
	static constexpr u32 max_requirements = 20;
	Dynamic_Array<Participation_Requirement>* reqs;
};


static bool Requirement_Is_Mark_Type(Participation_Requirement::Type type)
{
	bool result = type == Participation_Requirement::Type::mark_item ||
		type == Participation_Requirement::Type::mark_personal;
	return result;
}


static void Make_Requirement_Hollow(Participation_Requirement* req)
{
	if(Requirement_Is_Mark_Type(req->type))
		req->mark.free();
}
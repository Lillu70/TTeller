
#pragma once

/*
	GM - game mode structs have been optimized for size compared to the editor versions.
	12B vs 32B so that's about a one third.
	
	This is done to make iteration fast. Especially important for requirements.
*/

enum Languange : u8
{
	Finnish,
	English
};


struct Game_Participant_Localized_FI
{
	String full_name;
	String variant_name_1;
	String variant_name_2;
};


struct Game_Participant
{
	u8 mind;
	u8 body;
	
	Dynamic_Array<u32>* marks;
};


struct Req_GM_Header
{
	u32 global_req_count;
	u32 req_count;
	u32 participant_count;
};


struct Participant_Header
{
	u32 count;
};


struct Global_Mark_Requirement_GM
{
	Exists_Statement mark_exists;
	Numerical_Relation numerical_relation;
	i8 relation_target;
	
	u32 mark_idx;
};


struct Global_Mark_Consequence_GM
{
	i8 mark_duration;
	u32 mark_idx;
};


struct Participation_Requirement_GM
{
	Participation_Requirement_Type type;
	
	union
	{
		i8 duration; // For has mark.
		i8 relation_target; // For stat requirement.
	};
	
	Numerical_Relation numerical_relation; // For has mark and stat requirement.
	
	union
	{
		struct // For has/does not have mark.
		{
			Exists_Statement mark_exists;
			u32 mark_idx;
		};
		
		struct // For stat requirement.
		{
			Character_Stat::Stats stat_type;
		};
	};
};


struct Event_Consequens_GM
{
	Event_Consequens_Type type;
	
	union
	{
		i8 duration; // For gains mark.
		i8 change_amount; // For stat gain/loss.
		i8 inheritance_target; // For death.
	};
	
	union
	{
		struct // For gains_mark and loses_mark.
		{
			Mark_Type mark_type;
			u32 mark_idx;
		};
		
		struct // For stat gain/loss.
		{
			Character_Stat::Stats stat;
		};
	};
};


struct Table
{
	u32 count;
	void* memory;
};


struct Event_Header
{
	u32 size;
	u32 global_con_count;
	u32 con_count;
	
	String_View event_name;
	String_View event_text;
};


struct Game_State
{
	void* memory;
	
	char* mark_data;
	Table mark_table;
	
	Req_GM_Header* req_data;
	Table req_table_day;
	Table req_table_night;
	
	Event_Header* events_data;
	Table event_table_day;
	Table event_table_night;
	
	Dynamic_Array<Game_Participant>* participants;
	Dynamic_Array<Game_Participant_Localized_FI>* participant_names;
};




/*
	- Event
		- Participant
			- Requirements array
			
	
	- Event
		- Participant
			- Consequences array

*/

/* ON ISSUE OF MARKS <--- Solved with a hash table
	
	We don't want to create an entry in the mark table for each occurance of a mark, 
	but instead each unique one. Then the ids have to be properly mapped.
	
	This is proplematic from the perspective of calculating how big of an allocation is required.
	A temporary dynamicly resizing buffer is required for this.
*/


/* MEMOERY DESCRIPTION
	
	X - mark data -> All referenced tags in a sequence.				; sizeof(all mark texts combined)
	
	X - mark table -> Contains offsets to the beging of each mark.	; sizeof(u32 * mark count)
	
	| - req data -> All requirements grouped by event, then by 		; sizeof(Req_GM_Header * Event count + Global_Mark_Requirement_GM * count + Participation_Requirement_GM * count)
		participant. First is stored a Req_GM_Header, it is
		followed by amount of Global_Mark_Requirement_GM's and 
		Participation_Requirement_GM's defined by the header and
		seperated by Participant_Headers
	
	| - req table day -> Containts offsets to the Req_GM_Headers	; sizeof(u32 * event_count_day)
		indexed by event.
	
	| - req table night -> Containts offsets to the Req_GM_Headers	; sizeof(u32 * event_count_night)
		indexed by event.
	
	| - events data -> All Events_Headers global- and participant	; sizeof(Event_Header * Event count + event_text_lenght + event_name_lenght + Global_Mark_Consequence_GM * count + Event_Consequens_GM * count)
		consequences. Event name- and text are listed first.
	
	| - event table day -> Containts offsets to Event_Headers.		; sizeof(u32 * day event count)

	| - event table night -> Containts offsets to Event_Headers.	; sizeof(u32 * night event count)	
*/
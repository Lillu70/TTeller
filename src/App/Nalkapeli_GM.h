
#pragma once

/*
	GM - game mode structs have been optimized for size compared to the editor versions.
	12B vs 32B so that's about a one third.
	
	This is done to make iteration fast. Especially important for requirements.
*/

enum class Event_List : u8
{
	day,
	night,
	COUNT
};


enum class Language : u8
{
	finnish,
	english,
	COUNT
};


struct Game_Player_Name_FI
{
	String full_name;
	String variant_name_1;
	String variant_name_2;
};


struct Player_Image
{
	String file_path;
	Image image;
};


struct Mark_GM
{
	Mark_Type type;
	i8 duration;
	u32 idx;
};


struct Game_Player
{
	static constexpr i8 starting_stat_value = 2;
	
	i8 stats[(u32)Character_Stat::Stats::COUNT];
	bool alive;
	
	Dynamic_Array<Mark_GM>* marks;
};


struct Event
{
	u32 event_idx;
	u32 participant_count;
	u32* player_indices;
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
	u32 mark_idx;
	i8 mark_duration;
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
	u32 size; // TODO: rename to be clear about what size we're talking about.
	u32 global_con_count;
	u32 con_count; // TODO: <- Delete or change to something else.
	
	String_View event_name;
	String_View event_text;
};


struct Game_State
{
	u32 day_counter;
	
	// Campaing data converted into fast to operate format ---------------	
		void* memory; // <-- Combined allocation
		char* campaign_name;
		
		char* mark_data;
		Table mark_table;
		
		Req_GM_Header* req_data;
		Table req_table_day;
		Table req_table_night;
		
		Event_Header* events_data;
		Table event_table_day;
		Table event_table_night;
		// -------------------------------------
	
	// -------------------------------------------------------------------
	// Player data create after all players have been entered ------------
		
		u32 total_player_count;
		u32 live_player_count;
		Game_Player* players; // <-- Combined allocation
		u32* player_assignement_table;
		
		u32 event_assignement_table_size; // NOTE: this is to make multiple of same entry in the array easier for the rarity system.
		u32* event_assignement_table;
		// ---------------------------------------------
		
	// -------------------------------------------------------------------
	
	// runtime allocated items ------------------------
	
	Dynamic_Array<Player_Image>* player_images;
	
	Language language;
	union
	{
		Dynamic_Array<Game_Player_Name_FI>* player_names;
	};
	
	Dynamic_Array<Mark_GM>* global_marks;
	
	Event_List active_event_list;
	u32 display_event_idx;
	Dynamic_Array<Event>* active_events;
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
	
	| - Campaign name buffer
	
	| - mark data -> All referenced tags in a sequence.				; sizeof(all mark texts combined)
	
	| - mark table -> Contains offsets to the beging of each mark.	; sizeof(u32 * mark count)
	
	| - req data -> All requirements grouped by event, then by 		; sizeof(Req_GM_Header * Event count + Global_Mark_Requirement_GM * count + Participation_Requirement_GM * count)
		participant. First is stored a Req_GM_Header, it is
		followed by amount of Global_Mark_Requirement_GM's and 
		Participation_Requirement_GM's defined by the header and
		seperated by Participant_Headers
	
	| - req table day -> Containts byte offsets from 				; sizeof(u32 * event_count_day)
		the req data ptr to the Req_GM_Headers indexed by event.
		
	
	| - req table night -> Containts offsets to the Req_GM_Headers	; sizeof(u32 * event_count_night)
		indexed by event.
	
	| - events data -> All Events_Headers global- and participant	; sizeof(Event_Header * Event count + event_text_lenght + event_name_lenght + Global_Mark_Consequence_GM * count + Event_Consequens_GM * count)
		consequences. Event name- and text are listed first.
	
	| - event table day -> Containts offsets to Event_Headers.		; sizeof(u32 * day event count)

	| - event table night -> Containts offsets to Event_Headers.	; sizeof(u32 * night event count)	
*/
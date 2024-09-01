

// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#pragma once


enum class Menus : u32
{
    main_menu = 0,
    settings_menu,
    campaigns_menu,
    select_campaign_to_play_menu,
    choose_language,
    
    EVENT_EDITOR_BEGIN,
        EE_all_events,
        EE_participants,
        EE_text,
        EE_consequences,
    EVENT_EDITOR_END,
    
    GAME_MODE_BEGIN,
        GM_players,
        GM_let_the_games_begin,
        GM_event_display,
        GM_event_assignement_failed,
        GM_day_counter,
        GM_night_falls,
        GM_everyone_is_dead,
        GM_we_have_a_winner,
    GAME_MODE_END
};
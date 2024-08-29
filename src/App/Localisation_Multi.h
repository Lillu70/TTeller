
// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================


#define E ,

L(main_menu_buttons,
{
    "Continue" E
    "New Game" E
    "Load Game" E
    "Open Editor" E
    "Settings" E
    "Close"
}, 
{
    "Jatka" E
    "Uusi Peli" E
    "Lataa Peli" E
    "Avaa editori" E
    "Asetukset" E
    "Sulje"
})


L(languages, 
{
    "English" E
    "Finnish" E
},
{
    "Englanti" E
    "Suomi" E
})


L(event_error_names,
{
    "Event text references a participant that is not part of the event!" E
    "Mark (/) without acceptable follow-up!" E
    "Participant identifier is not a number!" E
    "Event has no participants!" E
    "Contains an impossible requirement!" E
    "Contains an impossible global requirement!" E
    "Contains an empty mark field in the requirements!" E
    "Contains an empty mark field in the global requirements!" E
    "Contains an empty mark field in the consequences!" E
    "Contains an empty mark field in the global consequences!" E
    "Death consequence where the inheritor is not part of the event!"
},
{
    "Tapahtumateksti viittaa osallistujaan joka ei ole tapahtumassa mukana!" E
    "Merkki (/) ilman hyv\xE4ksytt\xE4v\xE4\xE4 jatko merkki\xE4!" E
    "Osallistujan tunnistaja ei ole numero!" E
    "Tapahtumassa ei ole osallitujia!" E
    "Sis\xE4lt\xE4\xE4 mahdottoman vaatimuksen!" E
    "Sis\xE4lt\xE4\xE4 mahdottoman yleis vaatimuksen!" E
    "Sis\xE4lt\xE4\xE4 tyhj\xE4n merkki kent\xE4n vaatimuksissa!" E
    "Sis\xE4lt\xE4\xE4 tyhj\xE4n merkki kent\xE4n yleis vaatimuksissa!" E
    "Sis\xE4lt\xE4\xE4 tyhj\xE4n merkki kent\xE4n seuraamuksissa!" E
    "Sis\xE4lt\xE4\xE4 tyhj\xE4n merkki kent\xE4n yleis seuraamuksissa!" E
    "Kuolema seuraamus jossa tapahtumaan osallistumaton perij\xE4!"
})


L(event_warning_names,
{
    "Event text is empty." E
    "Event text does not reference all participants." E
    "Contains an an irrelevant consequence." E
    "Death consequence where the inheritor is the dying participant them selfs." E
    "Death consequence where the inheritor is zero, this causes nobody to inherit."
},
{
    "Tapahtumateksti on tyhj\xE4." E
    "Tapahtumateksti ei viittaa jokaiseen osallistujaan." E
    "Sis\xE4lt\xE4\xE4 merkitsem\xE4tt\xF6m\xE4n vaatimuksen." E
    "Kuolema seuraamus jossa perij\xE4n\xE4 on itse kuolija." E
    "Kuolema seuraamus jossa perij\xE4n\xE4 on nolla."
})


L(exists_statement_names,
{
    "Can not have." E
    "Must have."
},
{
    "Ei saa olla." E
    "T\xE4ytyy olla."
})


L(mark_type_names,
{
    "Item Mark" E
    "Character Mark"
},
{
    "Esine Merkki" E
    "Hahmo Merkki"
})


L(duration_names,
{
    "Unlimited" E
    "1 day" E
    "2 days" E
    "3 days"
},
{
    "Rajaton" E
    "1 p\xE4iv\xE4" E
    "2 p\xE4iv\xE4\xE4" E
    "3 p\xE4iv\xE4\xE4"
})


L(stat_names,
{
    "Body" E
    "Mind"
},
{
    "Keho" E
    "Mieli"
})


L(req_names,
{
    "Stat" E 
    "Item Mark" E 
    "Character Mark"
},
{
    "Ominaisuus" E 
    "Esine Merkki" E 
    "Hahmo Merkki"
})


L(con_names,
{
    "Death" E
    "Stat change" E
    "Gains a mark" E
    "Loses a mark"
},
{
    "Kuolema" E
    "Ominaisuus muutos" E
    "Saa merkin" E
    "Menett\xE4\xE4 merkin"
})


L(game_error_names,
{
    "Game does not contain any day events!" E
    "Game does not contain any night events!" E
    "Game does not contain any events!"
},
{
    "Peli ei sis\xE4ll\xE4 p\xE4iv\xE4 tapahtumaa!" E
    "Peli ei sis\xE4ll\xE4 y\xF6 tapahtumaa!" E
    "Peli ei sis\xE4ll\xE4 yht\xE4\xE4n tapahtumaa!"
})


L(game_actions,
{
    "Quit game" E
    "Reset game"
},
{
    "Lopeta peli" E
    "Palaa alkuun"
})


L(theme_names,
{
    "Vintage" E
    "Document" E
    "Cyper" E
    "Oasis"
},
{
    "Vuosikerta" E
    "Dokumentti" E
    "Kyper" E
    "Keidas"
})

#undef E
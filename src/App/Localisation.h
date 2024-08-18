
// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#ifdef SINGLE
    #ifdef FILL_LOC_IDENTIFIER
        #define L(ID, ENG, FIN) ID##,
    #else
        #define L(ID,ENG, FI) case Loc_Identifier::##ID:                \
        {                                                               \
            switch(s_settings.language)                                 \
            {                                                           \
                case Language::english:                                 \
                {                                                       \
                    result = ENG;                                       \
                }break;                                                 \
                                                                        \
                case Language::finnish:                                 \
                {                                                       \
                    result = FI;                                        \
                }break;                                                 \
            }                                                           \
        }break;

    #endif

    // one line localisation goes here!    

    L(menu_title, "T-Teller", "T-TELLER")

    #undef L
#endif

#ifdef MULTI
    #ifdef CREATE_LISTS
        #define L(ID, ENG, FIN)                                         \
        constexpr char* s_##ID##_FIN[] = FIN;                           \
        constexpr char* s_##ID##_ENG[] = ENG;
    #else
        
        #ifdef FILL_LOC_IDENTIFIER
            #define L(ID, ENG, FIN) ID##,
    
        #else
            #define L(ID, ENG, FI) case Loc_List_Identifier::##ID:      \
            {                                                           \
                switch(s_settings.language)                             \
                {                                                       \
                    case Language::english:                             \
                    {                                                   \
                        result.list = (char**)s_##ID##_ENG;             \
                        result.count = Array_Lenght(s_##ID##_ENG);      \
                    }break;                                             \
                                                                        \
                    case Language::finnish:                             \
                    {                                                   \
                        result.list = (char**)s_##ID##_FIN;             \
                        result.count = Array_Lenght(s_##ID##_FIN);      \
                    }break;                                             \
                }                                                       \
            }break;
        #endif
    #endif
        
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
        
        
        
    #undef L
    #undef E
#endif
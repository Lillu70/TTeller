

// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#pragma once


// NOTE: Crazy shit happens here. Please don't look at it xD

enum class Loc_Identifier : u32
{
    #define L(ID, ENG, FIN) ID##,
    #include "Localisation.h"
    
    #undef L
};

char* Get_Localised_Text(Loc_Identifier LI)
{ 
    char* result = "Localisation error!";
    
    switch(LI)
    {
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
        
        #include "Localisation.h"
        
        #undef L
    }
   
    return result;
}
#define L1(X) Get_Localised_Text(Loc_Identifier::##X)


#define L(ID, ENG, FIN)                                                 \
static constexpr char* s_##ID##_FIN[] = FIN;                            \
static constexpr char* s_##ID##_ENG[] = ENG;                    

#include "Localisation_Multi.h"

#undef L


enum class Loc_List_Identifier : u32
{
    #define L(ID, ENG, FIN) ID##,
    #include "Localisation_Multi.h"
    #undef L
};


CSTR_List Get_Localised_Texts(Loc_List_Identifier LI)
{
    char* error_stud[] = {"Localisation list error!"};
    
    CSTR_List result = {};
    result.list = error_stud;
    result.count = Array_Lenght(error_stud);

    switch(LI)
    {
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
        
        #include "Localisation_Multi.h"
        #undef L
    }
    
    return result;
}


#define LN(X) Get_Localised_Texts(Loc_List_Identifier::##X)

#define LN1(X, I) Get_Localised_Texts(Loc_List_Identifier::##X).list[I]

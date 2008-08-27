/*
 * $Header: /Book/handlers.h 4     11.07.01 15:14 Oslph312 $
 *
 * Interface to per-thread error handling, defined in handlers.cpp.
 */

#pragma once

#include "Exception.h"


#if INTERCEPT_SEH

class SE_Translator {
private:
    /*_se_translator_function*/ void *saved_se_translator;

public:
    SE_Translator( void );
    ~SE_Translator();
};

#endif // INTERCEPT_SEH


class NewHandler { // for operator new()
private:
    /*_PNH*/ void *saved_new_handler;

public:
    NewHandler( void ) throw();
    ~NewHandler() throw();
};


class NewMode { // for malloc()
private:
   int saved_new_mode;
   
public:
   NewMode( void ) throw();
   ~NewMode() throw();
};


class ThreadErrorHandling {
private:
#if INTERCEPT_SEH
   SE_Translator _set;
#endif // INTERCEPT_SEH

   NewHandler _nh;
   NewMode    _nm;
};

// end of file

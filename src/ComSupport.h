/*
 * $Header: /FAST/Baldur/ComSupport.h 3     31.01.05 13:33 Oslph312 $
 * 
 */

#pragma once


#ifdef USE_OWN_EXCEPTIONS
#include "Exception.h"
#else
typedef HRESULT ComException;
#endif


/**
 * Exception-safe wrapper for init/uninit
 */
class ComSupport {
public:
   ComSupport( void ) throw( ComException );
   ~ComSupport() throw();
};

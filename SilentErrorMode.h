/*
 * $Header: /Book/SilentErrorMode.h 3     3.07.99 17:46 Oslph312 $
 */

#pragma once


class SilentErrorMode {
private:
   UINT m_uiPrevMode;

public:
   SilentErrorMode( void );
   ~SilentErrorMode();
};


inline SilentErrorMode::SilentErrorMode( void ) { 
   m_uiPrevMode = SetErrorMode( 
      SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );
};


inline SilentErrorMode::~SilentErrorMode() {
   SetErrorMode( m_uiPrevMode );
}

// end of file

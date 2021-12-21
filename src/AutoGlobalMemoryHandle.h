/*
 * $Header: /Book/AutoGlobalMemoryHandle.h 2     20.08.99 16:33 Oslph312 $
 */

#pragma once

class AutoGlobalMemoryHandle {
private:
   HGLOBAL m_hGlobal;

public:
   AutoGlobalMemoryHandle( HGLOBAL hGlobal ) : m_hGlobal( hGlobal ) {
   }
   ~AutoGlobalMemoryHandle() { 
      if ( 0 != m_hGlobal ) {
         verify( 0 == GlobalFree( m_hGlobal ) ); 
      }
   }
};

// end of file

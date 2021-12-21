/*
 * $Header: /Book/MRU.h 11    3.07.99 17:46 Oslph312 $
 */

#pragma once

#include "String.h"
#include "resource.h" // Needs ID_MRU_*


class MRU {
private:
   enum { 
      FILES_TO_SHOW      =   9, // Tested with 19.
      FILES_TO_REMEMBER  =  30,
      STRING_WIDTH       = 200, // Tested with 50.
   };

   bool m_bDirty;
   int  m_nCount;
   String m_astrEntries[ FILES_TO_REMEMBER ];

   bool findFile( const String& strFile, int *pnIndex ) const;

#ifdef _DEBUG
   static int s_nRefCount;
#endif

public:
   MRU();
   ~MRU();

   int getCount( void ) const;

   void addFile( const String& strFile );
   void addFilesToMenu( HMENU hmenu, bool bShowAccelerators );
   void removeFile( const String& strFile );
   void renameFile( const String& strOld, const String& strNew );
   
   String getFile( int iCmdID ) const;
   String getFileTitle( int iCmdID ) const;
};


inline String MRU::getFile( int iCmdID ) const {
   
   assert( ID_MRU_1 <= iCmdID && 
                       iCmdID <= ID_MRU_1 + FILES_TO_REMEMBER );
   const int iFile = iCmdID - ID_MRU_1;
   assert( iFile < m_nCount );
   assert( !m_astrEntries[ iFile ].empty() );
   return m_astrEntries[ iFile ];
}


inline int MRU::getCount( void ) const {
   return m_nCount;
}

// end of file

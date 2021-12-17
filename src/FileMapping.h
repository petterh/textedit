/*
 * $Header: /Book/FileMapping.h 6     3.07.99 17:46 Oslph312 $
 */

#pragma once

/**
 * Mainly to get auto cleanup:
 */ 
class FileMapping {
private:
   HANDLE m_hMapping;
   void *m_pContents;

   void createMapping( HANDLE hFile, DWORD dwBytes, DWORD dwMode );

public:
   explicit FileMapping( HANDLE hFile );                // Read
   explicit FileMapping( HANDLE hFile, DWORD dwBytes ); // Write
   ~FileMapping();
   operator const LPVOID() const;
   operator LPVOID();
};


inline FileMapping::FileMapping( HANDLE hFile ) 
   : m_hMapping ( 0 )
   , m_pContents( 0 )
{
   createMapping( hFile, 0, PAGE_READONLY );
}


inline FileMapping::FileMapping( HANDLE hFile, DWORD dwBytes ) 
   : m_hMapping ( 0 )
   , m_pContents( 0 )
{
   createMapping( hFile, dwBytes, PAGE_READWRITE );
}


inline FileMapping::operator const LPVOID() const {
   return m_pContents;
}


inline FileMapping::operator LPVOID() {
   return m_pContents;
}

// end of file

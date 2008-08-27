/*
 * $Header: /Book/FileMapping.cpp 11    2.09.99 16:53 Oslph312 $
 */

#include "precomp.h"
#include "Exception.h"
#include "FileMapping.h"


void FileMapping::createMapping( 
   HANDLE hFile, DWORD dwBytes, DWORD dwMode ) 
{
   assert( PAGE_READONLY == dwMode || PAGE_READWRITE == dwMode );
   m_hMapping = CreateFileMapping( 
      hFile, 0, dwMode, 0, dwBytes, 0 );
   if ( 0 == m_hMapping ) {
      throwException( _T( "CreateFileMapping failed" ) );
   }

   const DWORD dwAccess = 
      PAGE_READONLY == dwMode ? FILE_MAP_READ : FILE_MAP_WRITE;
   m_pContents = MapViewOfFile( 
      m_hMapping, dwAccess, 0, 0, dwBytes );
   if ( 0 == m_pContents ) {
      const DWORD dwErr = GetLastError();
      verify( CloseHandle( m_hMapping ) );
      m_hMapping = 0;
      throwException( _T( "MapViewOfFile failed" ), dwErr );
   }
}


/**
 * This destructor is the main reason for creating 
 * this class in the first place.
 */
FileMapping::~FileMapping() {
   
   verify( UnmapViewOfFile( m_pContents ) );
   reset_pointer( m_pContents );
   
   verify( CloseHandle( m_hMapping ) );
   reset_pointer( m_hMapping );
}

// end of file

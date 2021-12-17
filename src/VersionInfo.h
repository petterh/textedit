/*
 * $Header: /Book/VersionInfo.h 6     3.07.99 17:46 Oslph312 $
 */

#pragma once

#pragma pack( 1 )

struct Language {
   WORD wLanguage;
   WORD wCharSet ;
};

#pragma pack()

class VersionInfo {
private:
   enum { VER_CHARSET_ANSI = 1252, };

   LPVOID   m_pInfo;
   int      m_nErr;
   int      m_nLanguages;
   Language *m_pLanguages;

   void init( LPCTSTR pszFile );

   LPCTSTR internalGetStringFileInfo (
      LPCTSTR pszItem, LANGID uiLang, UINT uiCharSet ) const;

public:
   VersionInfo( LPCTSTR pszFile );
   VersionInfo( HINSTANCE );

   ~VersionInfo();

   bool isValid ( void ) const;

   int getLanguageCount( void ) const;
   UINT getLanguage( int nIx = 0 ) const;
   UINT getCharSet( int nIx = 0 ) const;

   LPCTSTR getStringFileInfo(
      LPCTSTR pszItem, 
      LANGID uLang = LANGIDFROMLCID( GetThreadLocale() ),
      UINT uChar = CP_WINUNICODE ) const;
   bool getFileVersion( DWORD *pdwLow, DWORD *pdwHigh ) const;
};


// Inline functions:

inline VersionInfo::VersionInfo( LPCTSTR pszFile ) { 
   init( pszFile ); 
}


inline bool VersionInfo::isValid ( void ) const { 
   return ERROR_SUCCESS == m_nErr && 0 != m_pInfo;
}


inline int VersionInfo::getLanguageCount( void ) const {
   return m_nLanguages;
}


inline UINT VersionInfo::getLanguage ( int nIx ) const {

   assert( isValid() );
   if ( 0 < m_nLanguages ) {
      assert( isGoodReadPtr( m_pLanguages, 
         sizeof( m_pLanguages[ 0 ] ) * m_nLanguages ) );
      assert( 0 <= nIx && nIx < m_nLanguages );
      return m_pLanguages[ nIx ].wLanguage;
   }

   // US English as default:
   return MAKELANGID( LANG_ENGLISH, SUBLANG_DEFAULT ); 
}


inline UINT VersionInfo::getCharSet ( int nIx ) const {

   assert( isValid() );
   if ( 0 < m_nLanguages ) {
      assert( isGoodReadPtr( m_pLanguages, 
         sizeof( m_pLanguages[ 0 ] ) * m_nLanguages ) );
      assert( 0 <= nIx && nIx < m_nLanguages );
      return m_pLanguages[ nIx ].wCharSet;
   }
   
   return CP_WINUNICODE; // VER_CHARSET_ANSI
}

// end of file

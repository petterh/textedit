/*
 * $Header: /Book/Document.h 21    5.03.02 10:06 Oslph312 $
 */

#pragma once

#include "String.h"
#include "AutoPtr.h"
#include "AutoHandle.h"
#include "Exception.h"

#define DEFINE_PERSISTENT_DOC_INT( name, def, type )      \
   inline int get ## name( int nDef ) const {             \
      return getPersistentInt( _T( #name ), nDef, type ); \
   };                                                     \
   inline int get ## name( void ) const {                 \
      return get ## name( def );                          \
   };                                                     \
   inline void set ## name( int nValue ) {                \
      setPersistentInt( _T( #name ), nValue, type );      \
   }


#define DEFINE_PERSISTENT_DOC_STRING( name )              \
   inline String get ## name( void ) const {              \
      return getPersistentString( _T( #name ) );          \
   };                                                     \
   inline void set ## name( const String& strValue ) {    \
      setPersistentString( _T( #name ), strValue );       \
   }


class Document {
private:
   WIN32_FILE_ATTRIBUTE_DATA m_FileAttributes;

   String     m_strFileName;
   bool       m_isUnicode;
   bool       m_hasUnicodeTranslationError;
   bool       m_hasUnixLineFeeds;
   UINT       m_uiDriveType;
   bool       m_isReadOnly;
   bool       m_bAccessDenied;
   bool       m_bLockedByOtherProcess;
   bool       m_bBinary;
   bool       m_bDirty; 

   enum { NULL_REPLACEMENT = _T( '?' ) };

   HANDLE openFile( HWND hwnd );
   void createOrgCopy( HANDLE hFile );
   void deleteOrgCopy( void );
   LPTSTR convert( const LPVOID pbRawFile, DWORD dwBytes );
   LPTSTR getContents( const String& strFile, int *pnBytes );
   HFONT createPrintFont( HDC hdc );

   String getRegistryPath( void ) const;
   String getRegistryFileTypePath( void ) const;
   int getPersistentInt( 
      LPCTSTR pszName, int nDefault, bool bType ) const;
   void setPersistentInt( 
      LPCTSTR pszName, int nValue, bool bType );
   String getPersistentString( LPCTSTR pszName ) const;
   void setPersistentString( LPCTSTR pszName, const String& str );
   bool modifyAttribs( 
      const String& strFile, DWORD dwAdd, DWORD dwRemove = 0 );

   void assertValid( void );
   void assertValid( void ) const;

public:
   Document( HWND hwnd, LPCTSTR pszFile = 0 ); // TODO -- String arg?
   ~Document();

   void save( HWND hwnd, const void *pRawContents, int nBytes );
   void update( HWND hwnd, LPTSTR pszNewContents, int nLength = -1 );
   void addCRs( LPTSTR *ppsz ) throw( MemoryException );
   void removeCRs( LPTSTR psz );
   void print( HDC hdc, LPCTSTR pszText, int nCopies = 1 );
   bool deleteFile( HWND hwnd );
   bool modifyAttribs( DWORD dwAdd, DWORD dwRemove = 0 );

   const String& getPath( void ) const;
   bool setPath( HWND hwnd, const String& strNewPath );
   String getFileTypeDescription( bool bDisplay = false ) const;
   String getTitle( void ) const;
   LPTSTR getContents( int *pnBytes = 0 );
   LPTSTR getOrgContents( int *pnBytes = 0 );
   bool isBinary( void ) const;
   bool isFloppy( void ) const;
   bool isReadOnly( void ) const;
   bool isAccessDenied( void ) const;
   bool isClean( void ) const;
   void clean( void );
   bool isUnicode( void ) const;
   void setUnicode( bool bUnicode );
   bool hasUnixLineFeeds( void ) const;
   void setUnixLineFeeds( bool bUnixLineFeeds );

   static String createRegistryPath( const String& strRealPath );
   static bool s_bEndSession; // Set to true on WM_ENDSESSION.

   // These persistent variables are saved for 
   // file types as well as individual files:
   DEFINE_PERSISTENT_DOC_INT( FixedFont, 1, true );
   DEFINE_PERSISTENT_DOC_INT( WordWrap , 1, true );
   DEFINE_PERSISTENT_DOC_INT( Tabs     , 4, true );

   // These persistent variables are saved only for individual files:
   DEFINE_PERSISTENT_DOC_INT( Left        , CW_USEDEFAULT, false );
   DEFINE_PERSISTENT_DOC_INT( Top         , CW_USEDEFAULT, false );
   DEFINE_PERSISTENT_DOC_INT( Width       , CW_USEDEFAULT, false );
   DEFINE_PERSISTENT_DOC_INT( Height      , CW_USEDEFAULT, false );
   DEFINE_PERSISTENT_DOC_INT( WindowState , SW_SHOWNORMAL, false );
   DEFINE_PERSISTENT_DOC_INT( Running     , 0            , false );
   DEFINE_PERSISTENT_DOC_INT( SelStart    , 0            , false );
   DEFINE_PERSISTENT_DOC_INT( SelEnd      , 0            , false );
   DEFINE_PERSISTENT_DOC_INT( FirstLine   , 0            , false );

   // Page setup (per document):
   DEFINE_PERSISTENT_DOC_INT( LeftMargin      , 0, false );
   DEFINE_PERSISTENT_DOC_INT( TopMargin       , 0, false );
   DEFINE_PERSISTENT_DOC_INT( RightMargin     , 0, false );
   DEFINE_PERSISTENT_DOC_INT( BottomMargin    , 0, false );
   DEFINE_PERSISTENT_DOC_INT( MarginsAreMetric, 0, false );

   DEFINE_PERSISTENT_DOC_STRING( OrgCopy );
};

#undef DEFINE_PERSISTENT_DOC_INT


inline const String& Document::getPath( void ) const {
   
   assertValid();
   return m_strFileName;
}


inline bool Document::isBinary( void ) const {
   
   assertValid();
   return m_bBinary;
}


inline bool Document::isFloppy( void ) const {
   
   assertValid();
   return DRIVE_REMOVABLE == m_uiDriveType;
}


inline bool Document::isReadOnly( void ) const {

   assertValid();
   if ( DRIVE_CDROM == m_uiDriveType ) {
      return true;
   }
   const DWORD dwAttribs = 
      GetFileAttributes( m_strFileName.c_str() );
   return 0 != ( dwAttribs & FILE_ATTRIBUTE_READONLY );
}


inline bool Document::isAccessDenied( void ) const {

   assertValid();
   return m_bAccessDenied; // LATER: Ask the file? May take time.
}


inline bool Document::isClean( void ) const {
   
   assertValid();
   return !m_bDirty;
}


inline void Document::clean( void ) {
   
   assertValid();
   m_bDirty = false;
}


inline bool Document::isUnicode( void ) const {
   
   assertValid();
   return m_isUnicode;
}


inline void Document::setUnicode( const bool bUnicode ) {
   
   assertValid();
   if ( bUnicode != m_isUnicode ) {
      m_isUnicode = bUnicode;
      m_bDirty = true;
   }
}


inline void Document::setUnixLineFeeds( const bool bUnixLineFeeds ) {

   assertValid();
   if ( bUnixLineFeeds != m_hasUnixLineFeeds ) {
      m_hasUnixLineFeeds = bUnixLineFeeds;
      m_bDirty = true;
   }
}


inline bool Document::hasUnixLineFeeds( void ) const {

   assertValid();
   return m_hasUnixLineFeeds;
}


inline bool Document::modifyAttribs( DWORD dwAdd, DWORD dwRemove ) {

   assertValid();
   return modifyAttribs( getPath(), dwAdd, dwRemove );
}


inline void Document::assertValid( void ) {

   assert( isGoodPtr( this ) );
   static_cast< const Document * >( this )->assertValid();
}


inline void Document::assertValid( void ) const {

   assert( isGoodConstPtr( this ) );
}

// end of file

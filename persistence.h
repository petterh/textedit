/*
 * $Header: /Book/persistence.h 8     3.07.99 17:46 Oslph312 $
 */

#pragma once

#include "Registry.h"


#define DEFINE_PERSISTENT_BOOL( section, name )        \
   inline bool get ## name( void ) {                   \
      return 0 != Registry::getInt( HKEY_CURRENT_USER, \
         _T( section ), _T( #name ), 0 );              \
   };                                                  \
   inline void set ## name( bool bValue ) {            \
      Registry::setInt( HKEY_CURRENT_USER,             \
         _T( section ), _T( #name ), bValue );         \
   }


#define DEFINE_PERSISTENT_INT( section, name, def )    \
   inline int get ## name( void ) {         \
      return Registry::getInt( HKEY_CURRENT_USER, \
         _T( section ), _T( #name ), def );  \
   };                                             \
   inline void set ## name( int nValue ) {        \
      Registry::setInt( HKEY_CURRENT_USER,        \
         _T( section ), _T( #name ), nValue );    \
   }


#define DEFINE_PERSISTENT_STRING( section, name, def ) \
   inline String get ## name( LPCTSTR pszDefault ) {   \
      return Registry::getString( HKEY_CURRENT_USER,   \
         _T( section ), _T( #name ), pszDefault );     \
   };                                                  \
   inline String get ## name( void ) {                 \
      return get ## name( _T( def ) );                 \
   };                                                  \
   inline void set ## name( LPCTSTR pszValue ) {       \
      Registry::setString( HKEY_CURRENT_USER,          \
         _T( section ), _T( #name ), pszValue );       \
   }


#define DEFINE_PERSISTENT_STRING_EX( section, name )         \
   inline String get ## name( int nIndex ) {                 \
      TCHAR szName[ 20 ];                                    \
      wsprintf( szName, _T( #name ) _T( "%d" ), nIndex );    \
      return Registry::getString( HKEY_CURRENT_USER,         \
         _T( section ), szName );                            \
   };                                                        \
   inline void set ## name( int nIndex, LPCTSTR pszValue ) { \
      TCHAR szName[ 20 ];                                    \
      wsprintf( szName, _T( #name ) _T( "%d" ), nIndex );    \
      Registry::setString( HKEY_CURRENT_USER,                \
         _T( section ), szName, pszValue );                  \
   }


DEFINE_PERSISTENT_INT(    "Fonts", FixedHeight , 10            );
DEFINE_PERSISTENT_INT(    "Fonts", FixedWeight , FW_NORMAL     );
DEFINE_PERSISTENT_INT(    "Fonts", FixedItalic , 0             );
DEFINE_PERSISTENT_STRING( "Fonts", FixedFace   , "Consolas"    );
DEFINE_PERSISTENT_INT(    "Fonts", FixedCharSet, ANSI_CHARSET  );

DEFINE_PERSISTENT_INT(    "Fonts", ProportionalHeight , 10          );
DEFINE_PERSISTENT_INT(    "Fonts", ProportionalWeight , FW_NORMAL   );
DEFINE_PERSISTENT_INT(    "Fonts", ProportionalItalic , 0           );
DEFINE_PERSISTENT_STRING( "Fonts", ProportionalFace   , "Segoe UI"  );
DEFINE_PERSISTENT_INT(    "Fonts", ProportionalCharSet,ANSI_CHARSET );

DEFINE_PERSISTENT_INT(       "Open"  , FilterIndex, 1   );
DEFINE_PERSISTENT_STRING(    "Open"  , CustomFilter, "" );

DEFINE_PERSISTENT_BOOL(      "Search", MatchWholeWord   );
DEFINE_PERSISTENT_BOOL(      "Search", MatchCase        );
DEFINE_PERSISTENT_BOOL(      "Search", Backwards        );
DEFINE_PERSISTENT_STRING_EX( "Search", Pattern          );
DEFINE_PERSISTENT_STRING_EX( "Search", Replacement      );

DEFINE_PERSISTENT_INT(       "View"  , ToolbarVisible   , 1 );
DEFINE_PERSISTENT_INT(       "View"  , StatusbarVisible , 1 );

DEFINE_PERSISTENT_INT(       "Delete", SendToWasteBasket, 1 );
DEFINE_PERSISTENT_INT(       "Delete", ShowDeleteDialog , 1 );

DEFINE_PERSISTENT_INT(    "", Language, -1 );
DEFINE_PERSISTENT_STRING( "", DocumentPath, "" );

// end of file

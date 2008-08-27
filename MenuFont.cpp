/*
 * $Header: /Cleaner/MenuFont.cpp 12    22.03.02 12:57 Oslph312 $
 */

#include "precomp.h"
#include "MenuFont.h"
#include "Registry.h"
#include "os.h"
#include "trace.h"

#ifdef _DEBUG
#include "winUtils.h"
#endif

MenuFont MenuFont::theMenuFont;


MenuFont::MenuFont() : m_hfontMenu( 0 ) {

   assert( this == &theMenuFont ); // One and only one!
   refresh();
}


MenuFont::~MenuFont() {
   trace( _T( "Menufont dtor" ) );
   if ( 0 != m_hfontMenu ) {
      DeleteFont( m_hfontMenu );
      m_hfontMenu = 0;
      trace( _T( " deletes font\n" ) );
   } else {
      trace( _T( " has no font!\n" ) );
   }
}


HFONT MenuFont::getFont( void ) {

   if ( 0 != theMenuFont.m_hfontMenu ) {
      return theMenuFont.m_hfontMenu;
   }
   return GetStockFont( DEFAULT_GUI_FONT );
}


int MenuFont::getHeight( void ) {

   return theMenuFont.m_nCachedHeight;
}


#pragma pack( 1 )

typedef struct LOGFONTWIN95REG {
   short lfHeight;
   short lfWidth;
   short lfEscapement;
   short lfOrientation;
   short lfWeight;
   BYTE  lfItalic;
   BYTE  lfUnderline;
   BYTE  lfStrikeOut;
   BYTE  lfCharSet;
   BYTE  lfOutPrecision;
   BYTE  lfClipPrecision;
   BYTE  lfQuality;
   BYTE  lfPitchAndFamily;
   CHAR  lfFaceName[ LF_FACESIZE ];
} LOGFONTWIN95REG;

#pragma pack()


void MenuFont::refresh( void ) {

   if ( 0 != theMenuFont.m_hfontMenu ) {
      DeleteFont( theMenuFont.m_hfontMenu );
      theMenuFont.m_hfontMenu = 0;
   }

   #define REG_PATH  _T( "Control Panel\\Desktop\\WindowMetrics" )
   #define REG_ENTRY _T( "MenuFont" )

   // Check for wild values:
   #define CHECKFONT( logfont )                            \
      if ( abs( logfont.lfHeight ) < min_height ) {        \
         logfont.lfHeight = min_height;                    \
      } else if ( max_height < abs( logfont.lfHeight ) ) { \
         logfont.lfHeight = max_height;                    \
      }                                                    \
      logfont.lfEscapement = logfont.lfOrientation =       \
      logfont.lfUnderline = logfont.lfStrikeOut = 0

   bool isMenuFontOK = false;
   if ( isWindowsNT() ) {
      LOGFONTW logFontW = { 0 };
      isMenuFontOK = Registry::getBlob( HKEY_CURRENT_USER, 
         REG_PATH, REG_ENTRY, &logFontW, sizeof logFontW );
      if ( isMenuFontOK ) {
         CHECKFONT( logFontW );
         theMenuFont.m_hfontMenu = CreateFontIndirectW( &logFontW );
         GetObject( 
            theMenuFont.m_hfontMenu, sizeof logFontW, &logFontW );
         theMenuFont.m_nCachedHeight = abs( logFontW.lfHeight );
      }
   } else {
      LOGFONTWIN95REG regFont = { 0 };
      isMenuFontOK = Registry::getBlob( HKEY_CURRENT_USER, 
         REG_PATH, REG_ENTRY, &regFont, sizeof regFont );
      if ( isMenuFontOK ) {
         LOGFONTA logFontA = { 
            regFont.lfHeight        ,
            regFont.lfWidth         ,
            regFont.lfEscapement    ,
            regFont.lfOrientation   ,
            regFont.lfWeight        ,
            regFont.lfItalic        ,
            regFont.lfUnderline     ,
            regFont.lfStrikeOut     ,
            regFont.lfCharSet       ,
            regFont.lfOutPrecision  ,
            regFont.lfClipPrecision ,
            regFont.lfQuality       ,
            regFont.lfPitchAndFamily,
         };
         strcpy( logFontA.lfFaceName, regFont.lfFaceName );
         CHECKFONT( logFontA );
         theMenuFont.m_hfontMenu = CreateFontIndirectA( &logFontA );
         GetObject( theMenuFont.m_hfontMenu, sizeof logFontA, &logFontA );
         theMenuFont.m_nCachedHeight = abs( logFontA.lfHeight );
      }
   }

#ifdef _DEBUG
   trace( _T( "Menufont::refresh: OK = %d\n" ), isMenuFontOK );
   if ( !isMenuFontOK ) {
       messageBox( HWND_DESKTOP, MB_OK, _T( "isMenuFontOK is FALSE" ) );
   }
#endif

   #undef REG_PATH
   #undef REG_ENTRY
}

// end of file

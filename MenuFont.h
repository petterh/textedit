/*
 * $Header: /Book/MenuFont.h 6     3.07.99 17:46 Oslph312 $
 *
 * Singleton class.
 */

#pragma once

class MenuFont {
private:
   HFONT m_hfontMenu;
   int   m_nCachedHeight;

   enum Limits { min_height = 6, max_height = 32 };

   static MenuFont theMenuFont;

public:
   MenuFont();
   ~MenuFont();

   static HFONT getFont( void );
   static void refresh( void );
   static int getHeight( void );
   static bool isLarge( void );
};


inline bool MenuFont::isLarge( void ) {
   return 16 <= getHeight();
}

// end of file

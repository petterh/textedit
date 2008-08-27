/*
 * $Header: /Book/graphics.h 5     3.07.99 17:46 Oslph312 $
 * 
 */

#pragma once


inline void fillSolidRect( HDC hdc, LPCRECT prc, COLORREF cr ) {
   
   assert( isGoodConstPtr( prc ) );

   const COLORREF crSaveBkColor = SetBkColor( hdc, cr );
   ExtTextOut( hdc, 0, 0, ETO_OPAQUE, prc, 0, 0, 0 );
   SetBkColor( hdc, crSaveBkColor );
}


inline void fillSysColorSolidRect( 
   HDC hdc, LPCRECT prc, int nSysColorIndex ) 
{
   assert( isGoodConstPtr( prc ) );
   fillSolidRect( hdc, prc, GetSysColor( nSysColorIndex ) );
}

// end of file

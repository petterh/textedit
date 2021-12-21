/*
 * $Header: /Book/geometry.h 6     3.07.99 17:46 Oslph312 $
 */

#pragma once

class Point : public tagPOINT {
public:
   Point( LONG _x = 0, LONG _y = 0 );
};


inline Point::Point( LONG _x, LONG _y ) {
   x = _x;
   y = _y; 
}


class Rect : public tagRECT {
public:
   Rect();
   Rect( const RECT& rc );
   LONG width( void ) const;
   LONG height( void ) const;
};


inline Rect::Rect() {
   left = top = right = bottom = 0;
}


inline Rect::Rect( const RECT& rc ) {
   memcpy( this, &rc, sizeof *this );
}


inline LONG Rect::width( void ) const {

   return right - left;
}


inline LONG Rect::height( void ) const {

   return bottom - top;
}

// end of file

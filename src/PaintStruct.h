/*
 * $Header: /CustEd/PaintStruct.h 6     27.11.01 18:14 Oslph312 $
 */

#pragma once


class PaintStruct : public tagPAINTSTRUCT {

private:
   HWND _hwnd;
   
public:
   explicit PaintStruct ( HWND hwnd );
   ~PaintStruct ( void );
};


inline PaintStruct::PaintStruct ( HWND hwnd ) : _hwnd( hwnd ) {

   assert( IsWindow( _hwnd ) );
   BeginPaint( _hwnd, this );
}


inline PaintStruct::~PaintStruct ( void ) {

   assert( IsWindow( _hwnd ) );
   EndPaint( _hwnd, this );
}

// end of file

/*
 * $Header: /Book/RedrawHider.h 1     16.07.04 10:42 Oslph312 $
 */

#pragma once

class RedrawHider {
private:
	HWND hwnd;
public:
	RedrawHider( HWND hwnd ) : hwnd( hwnd ) {
		SendMessage( this->hwnd, WM_SETREDRAW, false, 0 );
	}
	~RedrawHider() {
		SendMessage( this->hwnd, WM_SETREDRAW, true, 0 );
		InvalidateRect( this->hwnd, 0, false );
	}
};

// end of file

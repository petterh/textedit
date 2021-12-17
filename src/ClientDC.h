/*
 * $Header: /FAST/Baldur/ClientDC.h 7     31.01.05 13:33 Oslph312 $
 */

#pragma once


#if 0

#ifndef assert
#define assert ASSERT
#endif


#ifndef verify
#ifdef _DEBUG
#define verify assert
#else
#define verify( X ) ( X )
#endif
#endif

#endif // 0


/**
 * Wrapper for client display context with auto-delete
 */
class ClientDC {

private:
   HDC  m_hdc ;
   HWND m_hwnd;
   
public:
   explicit ClientDC ( HWND hwnd = HWND_DESKTOP );
   ~ClientDC ( void );
   bool isValid( void ) const;
   operator HDC() const;
};


inline ClientDC::ClientDC ( HWND hwnd ) : m_hwnd( hwnd ) {

#ifdef assert
   assert( HWND_DESKTOP == m_hwnd || IsWindow( m_hwnd ) );
#endif

   m_hdc = GetDC( m_hwnd );
   if ( !isValid() ) {
#ifdef trace
      trace( _T( "ClientDC: GetDC( %#x ) failed: %lu\n" ), 
         m_hwnd, GetLastError() );
#endif
   }
}


inline ClientDC::~ClientDC ( void ) {

#ifdef assert
   assert( HWND_DESKTOP == m_hwnd || IsWindow( m_hwnd ) );
#endif

   if ( 0 != m_hdc ) {
      verify( ReleaseDC( m_hwnd, m_hdc ) );
   }
}


inline bool ClientDC::isValid( void ) const {

   assert( HWND_DESKTOP == m_hwnd || IsWindow( m_hwnd ) );
   return 0 != m_hdc;
}


inline ClientDC::operator HDC() const {

   assert( HWND_DESKTOP == m_hwnd || IsWindow( m_hwnd ) );
   return m_hdc;
}

// end of file

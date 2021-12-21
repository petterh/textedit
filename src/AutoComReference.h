/*
 * $Header: /FAST/Baldur/AutoComReference.h 4     21.12.04 15:27 Oslph312 $
 */

#pragma once

#ifdef USE_OWN_EXCEPTIONS
#include "Exception.h"
#endif

#ifdef USE_OWN_EXCEPTIONS
#define ASSERT assert
#else
inline void comFree( LPVOID pObject ) {
   LPMALLOC lpMalloc = 0;
   const HRESULT hResult = CoGetMalloc( 1, &lpMalloc );
   if ( SUCCEEDED( hResult ) ) {
      lpMalloc->Free( pObject );  
      lpMalloc->Release();
   } else {
      TRACE( _T( "CoGetMalloc failed\n" ) );
   }
}

#endif

template< class T >
class AutoComReference {
private:
   T *m_pComReference;

public:
   explicit AutoComReference( void );
// explicit AutoComReference( T *pComReference );
   explicit AutoComReference( REFCLSID rclsid, REFIID riid );
   explicit AutoComReference( REFIID riid, IUnknown *pComReference );
   ~AutoComReference();
   T* operator->() throw();
   operator T*() throw();
   T** operator &() throw();
};


template< class T >
inline AutoComReference< T >::AutoComReference( void ) 
   : m_pComReference( 0 )
{
}


#if 0
template< class T >
inline AutoComReference< T >::AutoComReference( T *pComReference ) 
   : m_pComReference( pComReference )
{
}
#endif


template< class T >
inline AutoComReference< T >::AutoComReference( 
   REFCLSID rclsid, REFIID riid ) : m_pComReference( 0 )
{
   HRESULT hres = CoCreateInstance( rclsid, 0, 
      CLSCTX_INPROC_SERVER, riid, (LPVOID *) &m_pComReference );
   if ( !SUCCEEDED( hres ) ) {
#ifdef USE_OWN_EXCEPTIONS
      throw ComException( hres );
#else
	   throw hres;
#endif
   }
}


template< class T >
inline AutoComReference< T >::AutoComReference( 
   REFIID riid, IUnknown *pComReference ) : m_pComReference( 0 )
{
   ASSERT( 0 != pComReference );
   HRESULT hres = pComReference->QueryInterface( 
      riid, (LPVOID *) &m_pComReference );
   if ( !SUCCEEDED( hres ) ) {
#ifdef USE_OWN_EXCEPTIONS
      throw ComException( hres );
#else
	   throw hres;
#endif
   }
}


template< class T >
inline AutoComReference< T >::~AutoComReference() {
//   ASSERT( 0 != m_pComReference );  This assertion is plain wrong!
   if ( 0 != m_pComReference ) {
#ifdef _DEBUG
//    const ULONG ulRefCount = 
#endif
         m_pComReference->Release();

#ifdef USE_OWN_EXCEPTIONS
      reset_pointer( m_pComReference );
      //unused( ulRefCount );
#endif
   }
}


template< class T >
inline T* AutoComReference< T >::operator->() throw() {
   return m_pComReference;
}


template< class T >
inline AutoComReference< T >::operator T*() throw() {
   return m_pComReference;
}


template< class T >
inline T** AutoComReference< T >::operator &() throw() {
   return &m_pComReference;
}

// end of file

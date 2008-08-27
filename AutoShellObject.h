/*
 * $Header: /FAST/MailScanner/MailScanner/AutoShellObject.h 8     27.01.05 14:33 Oslph312 $
 *
 * Automatic destruction for object references needing comFree.
 */

#pragma once

#ifdef USE_OWN_EXCEPTIONS
#include "utils.h"
#endif

template< class T >
class AutoShellObject {
private:
   T *_pShellObject;

public:
   explicit AutoShellObject( T* pShellObject = 0 );
   ~AutoShellObject();
   operator T*() throw();
   T** operator &() throw();
   bool isNull( void ) const;
};


template< class T >
inline AutoShellObject< T >::AutoShellObject( T* pShellObject ) 
   : _pShellObject( pShellObject )
{
}


template< class T >
inline AutoShellObject< T >::~AutoShellObject() {
   if ( 0 != _pShellObject ) {
	   CoTaskMemFree( _pShellObject );
   }
}


template< class T >
inline AutoShellObject< T >::operator T*() throw() {
   return _pShellObject;
}


template< class T >
inline T** AutoShellObject< T >::operator &() throw() {
   return &_pShellObject;
}


template< class T >
inline bool AutoShellObject< T >::isNull( void ) const throw() {
   return 0 == _pShellObject;
}

// end of file

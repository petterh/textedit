/*
 * $Header: /FAST/Baldur/ComSupport.cpp 3     31.01.05 13:33 Oslph312 $
 * 
 */


#include "precomp.h"
#include "ComSupport.h"


ComSupport::ComSupport( void ) {

   const HRESULT hres = CoInitialize( 0 ); // TODO -- the coInitialize bit (see book)
   if ( !SUCCEEDED( hres ) ) {
      throw ComException( hres );
   }
}


ComSupport::~ComSupport() {

   __try {
      CoUninitialize();
   }
   __finally {
   }
}

/*
 * $Header: /Book/handlers.cpp 17    16.07.04 10:42 Oslph312 $
 *
 * Defines translator for system-level exceptions 
 * (SEH, Structured Exception Handling)
 * to C++ exceptions and the C++ allocation_failure handler.
 *
 * Exports: initThreadErrorHandling and exitThreadErrorHandling.
 * These functions set up and restore handlers on a per-thread basis.
 */

#include "precomp.h"
#include "handlers.h"
#include "os.h"
#include <new.h>


// Parameters to _set_new_mode:
#define SNM_RETURN_NULL_ON_FAILURE  0
#define SNM_CALL_NEW_HANDLER        1


#if INTERCEPT_SEH

/**
 * This function translates SEH exceptions to C++ exceptions.
 * It is installed using _set_se_translator in 
 * the initThreadErrorHandling function.
 */
void __cdecl se_translator( 
   unsigned int uiCode, EXCEPTION_POINTERS *pE ) 
{
   trace( _T( "se_translator( %#x, %#x )\n" ), 
      uiCode, pE->ExceptionRecord->ExceptionFlags );
   trace( _T( "             ( %#x, %#x )\n" ), 
      pE->ExceptionRecord->ExceptionAddress, 
      pE->ExceptionRecord->ExceptionCode );
   
   switch ( uiCode ) {
   case STATUS_INTEGER_DIVIDE_BY_ZERO: 
      throw DivideByZeroException();
      
   case STATUS_STACK_OVERFLOW: 
      throw StackOverflowException::getStackOverflowException();
      
   case STATUS_INVALID_HANDLE: 
      throw InvalidHandleException();
      
   case STATUS_ACCESS_VIOLATION: 
      throw AccessViolationException();
      
   case STATUS_NO_MEMORY: 
      throwMemoryException();
   }

   throw sehException(); // Store uiCode, perhaps?
}


SE_Translator::SE_Translator( void ) {
   saved_se_translator = _set_se_translator( se_translator );
   trace( _T( "installed new SE translator\n" ) );
}


SE_Translator::~SE_Translator() {
   __try {
      _set_se_translator( 
         (_se_translator_function) saved_se_translator );
      trace( _T( "restored old SE translator\n" ) );
   }
   __finally {
   }
}

#endif // INTERCEPT_SEH


/**
 * This function gets called whenever malloc experiences a failure:
 */
PRIVATE int __cdecl allocation_failure( size_t size ) {
   
#ifdef _DEBUG
   // Calling trace here is not a good idea, 
   // as we may be operating on limited resources.
   // Call OutputDebugString directly instead.
   static TCHAR szMsg[ 30 ] = { 0 };
   wsprintf( szMsg, _T( "allocation_failure: %d\n" ), size );
   OutputDebugString( szMsg );
#endif

   throwMemoryException();

#pragma warning( disable: 4702 ) // Unreachable code
   assert( false ); // Never gets here...
#pragma warning( default: 4702 )
}


NewHandler::NewHandler( void )
   : saved_new_handler( _set_new_handler( allocation_failure ) )
{
   trace( _T( "installed new new_handler\n" ) );
}


NewHandler::~NewHandler() {
   __try {
      _set_new_handler( (_PNH) saved_new_handler );
      trace( _T( "restored old new_handler\n" ) );
   }
   __finally {
   }
}


NewMode::NewMode( void )
   : saved_new_mode( _set_new_mode( SNM_CALL_NEW_HANDLER ) )
{
   trace( _T( "installed new new_mode\n" ) );
}


NewMode::~NewMode() {
   __try {
      _set_new_mode( saved_new_mode );
      trace( _T( "restored old new_mode\n" ) );
   }
   __finally {
   }
}

// end of file

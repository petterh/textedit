/*
 * $Header: /FAST/Baldur/AutoArray.h 11    31.01.05 13:32 Oslph312 $
 *
 * This class wraps a pointer to an array allocated on the heap,
 * deleting the array when the class goes out of scope.
 *
 * AutoString is a template specialization for LPTSTRs.
 *
 * HINT: To see the string wrapped by AutoArray during Visual C++
 * debugging, add the following line to the AUTOEXP.DAT file:
 * AutoArray<*>=_ptr=<_ptr>
 *
 * A typical location for this file is:
 * C:\Program Files\Microsoft Visual Studio\Common\MSDev98\Bin\AUTOEXP.DAT
 */

#pragma once


#ifndef assert
#define assert ASSERT
#endif


#ifndef reset_pointer
#define reset_pointer( X ) X = 0
#endif

template< class T >
class AutoArray {
private:
   T *_ptr;

public:
   //explicit AutoArray( int n ) throw() : _ptr( new T[ n ] ) { // TODO: Dangerous for 0 ptrs!  Is it ever used?
   //   assert( 0 != this->_ptr );
   //}
   explicit AutoArray( T *ptr = 0 ) throw() : _ptr( ptr ) {
   }
   ~AutoArray() {
      delete[] this->_ptr; // NOTE: It's OK to delete a null pointer.
      reset_pointer( this->_ptr );
   }
   void alloc( int n ) {
      assert( 0 == this->_ptr );
      this->_ptr = new T[ n ];
   }
   T *operator=( T *ptr ) {
	   delete[] this->_ptr;
	   this->_ptr = ptr;
	   return this->_ptr;
   }
   bool operator==( T *ptr ) const {
	   return this->_ptr == ptr;
   }
   bool operator!=( T *ptr ) const {
	   return this->_ptr != ptr;
   }
   T ** operator &() {
      return &this->_ptr;
   }
   operator T *() throw() {
      return this->_ptr;
   }
   operator const T *() const throw() {
      return this->_ptr;
   }
   operator void *() throw() {
      return this->_ptr;
   }
   operator const void *() const throw() {
      return this->_ptr;
   }
};


typedef AutoArray< CHAR  > AutoStringA;
typedef AutoArray< WCHAR > AutoStringW;

#ifdef UNICODE
typedef AutoStringW AutoString;
#else
typedef AutoStringA AutoString;
#endif

// end of file

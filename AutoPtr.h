/*
 * $Header: /Book/AutoPtr.h 4     3.07.99 17:46 Oslph312 $
 *
 * HINT: To see the object wrapped by AutoPtr during Visual C++
 * debugging, add the following line to the AUTOEXP.DAT file:
 * AutoPtr<*>=m_ptr=<m_ptr>
 */

#pragma once

template< class T >
class AutoPtr {
private:
   T *m_ptr;

public:
   explicit AutoPtr( T *ptr = 0 ) throw() : m_ptr( ptr ) {
   }
   ~AutoPtr() {
      delete m_ptr;      // NOTE: It's OK to delete a null pointer.
      reset_pointer( m_ptr );
   }
   void reset( T *ptr = 0 ) {
      T *pOldPtr = m_ptr;
      m_ptr = ptr;
      if ( ptr != pOldPtr ) {
         delete pOldPtr; // NOTE: It's OK to delete a null pointer.
      }
   }
   T *operator=( T *ptr ) throw() {
      m_ptr = ptr;
      return m_ptr;
   }
   operator T *() throw() {
      return m_ptr;
   }
   operator const T *() const throw() {
      return m_ptr;
   }
   operator void *() throw() {
      return m_ptr;
   }
   operator const void *() const throw() {
      return m_ptr;
   }
   T *operator->() throw() {
      return m_ptr;
   }
   const T *operator->() const throw() {
      return m_ptr;
   }
};

// end of file

/*
 * $Header: /Book/FileType.h 3     3.07.99 17:46 Oslph312 $
 */

#pragma once


#include "String.h"


class FileType {
private:
   String m_strExtension;
   String m_strClass;
   bool   m_isEditDefault;
   bool   m_bEdit;
   bool   m_bPrint;
   bool   m_bInclude;

   FileType( LPCTSTR pszExtension, 
      bool isEditDefault, bool bEdit, bool bPrint );
   void setCommand( LPCTSTR pszPath,
      LPCTSTR pszFmt, LPCTSTR pszName = 0 );
   void removeCommand( LPCTSTR pszPath, LPCTSTR pszFmt );
   
   LPCTSTR getClass  ( void ) const;
   bool isEditDefault( void ) const;
   bool useEdit      ( void ) const;
   bool shouldInclude( void ) const;
   bool shouldPrint  ( void ) const;

   static FileType sm_aFileType[];
   static String   sm_strCommand;

public:
   LPCTSTR getExtension( void ) const;
   bool    exists      ( void ) const;
   
   void   registerType ( void );
   void unregisterType ( void );

   static int getNumFileTypes( void );
   static FileType *getFileType( int nIndex );
   static void include( LPCTSTR pszExt, bool bInclude );
   static void setCommand( const String& strCommand );
};


inline LPCTSTR FileType::getExtension( void ) const {

   return m_strExtension.c_str();
}


inline LPCTSTR FileType::getClass( void ) const {

   return m_strClass.c_str();
}


inline bool FileType::isEditDefault( void ) const {

   return m_isEditDefault;
}


inline bool FileType::useEdit( void ) const {

   return m_bEdit;
}


inline bool FileType::shouldPrint( void ) const {

   return m_bPrint;
}


inline bool FileType::shouldInclude( void ) const {

   return m_bInclude;
}

// end of file

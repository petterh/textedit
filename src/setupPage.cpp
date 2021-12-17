/*
 * $Header: /Book/setupPage.cpp 12    20.08.99 16:33 Oslph312 $
 */

#include "precomp.h"
#include "AutoGlobalMemoryHandle.h"
#include "setupPage.h"
#include "devMode.h"
#include "devNames.h"
#include "winUtils.h"
#include "persistence.h"
#include "resource.h"


/**
 * This does nothing. Fill in the blanks if you need it.
 */
PRIVATE UINT CALLBACK PagePaintHook( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   assert( IsWindow( hwnd ) );

   switch ( msg ) {
   case WM_PSD_GREEKTEXTRECT:
      ;
      // FillRect( hdc, prc, GetStockBrush( LTGRAY_BRUSH ) );
      // return 1; // if you don't want the parent dialog to paint:
   }

   return 0;
}


/**
 * Centers the dialog on its parent on opening.
 * Stores position on closing; uses this on later openings.
 */
PRIVATE UINT CALLBACK PageSetupHook( 
   HWND hwnd, UINT msg, WPARAM, LPARAM ) 
{
   assert( IsWindow( hwnd ) );
   if ( WM_INITDIALOG == msg ) {
      restorePosition( hwnd, IDD_PRINT_SETUP );
   } else if ( WM_DESTROY == msg ) {
      savePosition( hwnd, IDD_PRINT_SETUP );
   }
   return 0;
}


void setupPage( HWND hwndParent, Document *pDocument ) {

   assert( IsWindow( hwndParent ) );
   assert( isGoodPtr( pDocument ) );

   PAGESETUPDLG pageSetupDlg = {
      sizeof( PAGESETUPDLG ),
      hwndParent,
      getDevMode(), getDevNames(),
      PSD_ENABLEPAGESETUPHOOK | PSD_ENABLEPAGEPAINTHOOK, 
      { 0, }, { 0, }, { 0, }, 
      0, 0, PageSetupHook, PagePaintHook,
   };

   const AutoGlobalMemoryHandle a1( pageSetupDlg.hDevMode  );
   const AutoGlobalMemoryHandle a2( pageSetupDlg.hDevNames );

   pageSetupDlg.rtMargin.left   = pDocument->getLeftMargin  ();
   pageSetupDlg.rtMargin.top    = pDocument->getTopMargin   ();
   pageSetupDlg.rtMargin.right  = pDocument->getRightMargin ();
   pageSetupDlg.rtMargin.bottom = pDocument->getBottomMargin();
   if ( 0 < pageSetupDlg.rtMargin.left ) {
      pageSetupDlg.Flags |= PSD_MARGINS;
      if ( pDocument->getMarginsAreMetric() ) {
         pageSetupDlg.Flags |= PSD_INHUNDREDTHSOFMILLIMETERS;
      } else {
         pageSetupDlg.Flags |= PSD_INTHOUSANDTHSOFINCHES;
      }
   }

   if ( HWND_DESKTOP != hwndParent ) {
      FORWARD_WM_COMMAND( 
         hwndParent, ID_COMMAND_RESETSTATUSBAR, 0, 0, SNDMSG );
   }

   const BOOL bOK = PageSetupDlg( &pageSetupDlg );
   if ( bOK ) {
      // Orientation and paper, common with printing
      setDevMode ( pageSetupDlg.hDevMode  ); 
      setDevNames( pageSetupDlg.hDevNames );
      pDocument->setLeftMargin  ( pageSetupDlg.rtMargin.left   );
      pDocument->setTopMargin   ( pageSetupDlg.rtMargin.top    );
      pDocument->setRightMargin ( pageSetupDlg.rtMargin.right  );
      pDocument->setBottomMargin( pageSetupDlg.rtMargin.bottom );
      if ( pageSetupDlg.Flags & PSD_INHUNDREDTHSOFMILLIMETERS ) {
         pDocument->setMarginsAreMetric( 1 );
      } else if ( pageSetupDlg.Flags & PSD_INTHOUSANDTHSOFINCHES ) {
         pDocument->setMarginsAreMetric( 0 );
      }
   }
}

// end of file

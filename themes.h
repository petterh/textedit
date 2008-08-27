/*
 * $Header: /Book/themes.h 2     17.12.02 9:38 Oslph312 $
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef HANDLE MYHTHEME; // To avoid uxtheme inclusion here

MYHTHEME STDAPICALLTYPE openThemeData( HWND hwnd, LPCWSTR pszClassList );
HRESULT STDAPICALLTYPE drawThemeBackground(
    MYHTHEME ht, HDC hdc, int iPartId, int iStateId,
    const RECT *pRect, OPTIONAL const RECT *pClipRect );
HRESULT STDAPICALLTYPE getThemeColor(
    MYHTHEME ht, int iPartId, int iStateId, int iPropId, OUT COLORREF *pColor );
HRESULT STDAPICALLTYPE closeThemeData( MYHTHEME );

#ifdef __cplusplus
}
#endif

// end of file

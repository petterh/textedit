#include "../src/precomp.h"
#include "../src/formatNumber.h"
#include "CppUnitLite/CppUnitLite.h"

TEST( formatNumber, execution ) {

	String result1 = formatNumber(123);

	TCHAR szThousandSep[ 10 ] = { 0 };
	GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szThousandSep, dim( szThousandSep ) );

	TCHAR szGrouping[ 100 ] = { 0 };
	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szGrouping, dim( szGrouping ) );

	SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, _T("."));
	String result2 = formatNumber(1234);

	SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, _T("3;1;2;0"));
	String result3 = formatNumber(123456789);

	// Restore before checking, as the checks may exit the function
	SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szThousandSep);
	SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szGrouping);

	CHECK_EQUAL(_T("123"), result1.c_str());
	CHECK_EQUAL(_T("1.234"), result2.c_str());
	CHECK_EQUAL(_T("1.23.45.6.789"), result3.c_str());
}

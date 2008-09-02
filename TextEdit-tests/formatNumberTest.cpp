#include "precomp.h"
#include "..\formatNumber.h"
#include "CppUnitLite\CppUnitLite.h"

void test();

TEST( formatNumber, execution ) {

	CHECK_EQUAL(_T("123"), formatNumber(123).c_str());

	TCHAR szThousandSep[ 10 ] = { 0 };
	GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szThousandSep, dim( szThousandSep ) );

	TCHAR szGrouping[ 100 ] = { 0 };
	GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szGrouping, dim( szGrouping ) );

	SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, _T("."));
	CHECK_EQUAL(_T("1.234"), formatNumber(1234).c_str());

	SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, _T("3;1;2;0"));
	CHECK_EQUAL(_T("1.23.45.6.789"), formatNumber(123456789).c_str());

	SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szThousandSep);
	SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szGrouping);
}

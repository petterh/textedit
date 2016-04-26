#include "..\precomp.h"
#include "..\fileUtils.h"
#include "CppUnitLite\CppUnitLite.h"

TEST(_tcscpy_s, SafeStrings) {

    WCHAR sz[2] = { L'X', L'Y' };
    errno_t result = _tcscpy_s(sz, L"Z");

    CHECK(result == NOERROR);
    CHECK(sz[0] == L'Z');
    CHECK(sz[1] == 0);
}

TEST(_tcsncpy_s, SafeStrings) {

    WCHAR sz[2] = { L'X', L'Y' };
    errno_t result = _tcsncpy_s(sz, L"Z", 2);

    CHECK(result == NOERROR);
    CHECK(sz[0] == L'Z');
    CHECK(sz[1] == 0);
}

#pragma once

#include <crtdbg.h>

class DisableDebugFill
{
private:
#ifdef _DEBUG
    size_t saved;
#endif

public:

    DisableDebugFill()
    {
#ifdef _DEBUG
        saved = _CrtSetDebugFillThreshold(0);
#endif
    }

    ~DisableDebugFill()
    {
#ifdef _DEBUG
        _CrtSetDebugFillThreshold(saved);
#endif
    }
};

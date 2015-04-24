#pragma once

#include "comet/handle_except.h"
#include <mfapi.h>

namespace comet
{
    struct auto_mf
    {
        auto_mf(DWORD dwFlags = MFSTARTUP_FULL)
        {
            MFStartup(MF_VERSION) | comet::raise_exception;
        }
        ~auto_mf()
        {
            MFShutdown();
        }
    };
}


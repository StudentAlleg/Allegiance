/*//////////////////////////////////////////////////////////////////////////////
//
// File: common.cpp
//
// Copyright (C) 1999 Microsoft Corporation. All Rights Reserved.
//
//
//////////////////////////////////////////////////////////////////////////////*/


#include "pchgxu.h"
#include "common.h"
#include "strsafe.h"

#ifdef _DEBUG
    #include <stdio.h>
    #include <crtdbg.h>
    inline void __cdecl
        _GXTrace(const char *szFmt, ...)
    {
        char szMsgBuf[1024];
        va_list alist;
        va_start( alist, szFmt );
        StringCchVPrintfA(szMsgBuf, 1024, szFmt, alist );
    #if defined(_WINDOWS) || defined(WIN32) || defined(_WIN32)
        OutputDebugString(szMsgBuf);
    #endif // #if defined(_WINDOWS) || defined(WIN32) || defined(_WIN32)
        fprintf(stderr, "%s", szMsgBuf);
        fflush(stderr);
    }
#endif

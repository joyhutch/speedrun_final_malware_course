#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include "debug.h"

// make an HTTP request and return a heap allocated buffer with the result
// the caller is responsble for freeing
LPBYTE 
HTTPRequest(
    LPCWSTR pwszVerb, 
    LPCWSTR pwszServer, 
    LPCWSTR pwszPath, 
    int nServerPort,
    LPCWSTR pwszUserAgent, 
    LPBYTE data, 
    SIZE_T stData, 
    PSIZE_T stOut, 
    BOOL bTLS
    );
#include <windows.h>
#include <stdio.h>
#include "debug.h"

// Execute a cmd.exe command. Caller is responsible for freeing result
// if there is an error, a NULL pointer is returned 
LPBYTE ExecuteCmd(LPCSTR szCmdline, PSIZE_T stOut);
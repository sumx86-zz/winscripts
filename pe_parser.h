#ifndef __pe_Parser_HH
#define __pe_Parser_HH 1

#include <iostream>
#include <vector>
#include <windows.h>

#define PEHEADER(ptr) ((LPVOID)((ptr + ((PIMAGE_DOS_HEADER)ptr)->e_lfanew) + sizeof(IMAGE_NT_SIGNATURE)))

namespace PE_PARSER {
    LPVOID LoadFile( const char *path );
    bool IsDllFile( VOID );
    WORD DllMachine( VOID );
    bool FileExists( const char *path );
};

#endif
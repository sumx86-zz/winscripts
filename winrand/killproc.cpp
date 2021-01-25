#include <iostream>
#include <windows.h>
#include <tlhelp32.h>

#define ISNULL(ptr) (((ptr) == NULL) || ((ptr) == nullptr))

#define ERROR_PROCESS_NOT_FOUND -2

DWORD __stdcall FindProcess( const char *name ) {
    HANDLE hndl = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if( hndl == INVALID_HANDLE_VALUE )
        return -1;

    PROCESSENTRY32 pe32;
    bool ret = Process32First( hndl, &pe32 );
    
    if ( !ret )
        return -1;
    do {
        if ( _stricmp( pe32.szExeFile, name ) == 0 ) {
            CloseHandle(hndl);
            return pe32.th32ProcessID;
        }
    }
    while( Process32Next( hndl, &pe32 ) );

    CloseHandle(hndl);
    return ERROR_PROCESS_NOT_FOUND;
}

bool __stdcall KillProcess( DWORD pid ) {
    HANDLE hndl = OpenProcess( PROCESS_TERMINATE , false, pid );
    if ( ISNULL(hndl) )
        return false;

    short is_killed = false;
    if ( TerminateProcess( hndl, 1 ) ) {
        is_killed = true;
    } else {
        std::cerr << "[ERROR] - Could't terminate process!\n";
    }
    CloseHandle(hndl);
    return is_killed;
}

SHORT __stdcall WinStrerror( char *err, size_t size, DWORD errcode ) {
    DWORD errCode = (errcode > 0) ? errcode : GetLastError();
    char *str;
    if (!FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        errCode,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR) &str,
                        0,
                        NULL))
        return -1;
    memcpy( err, str, size ), LocalFree( str );
    return 0;
}

int main( int argc, char **argv ) {
    char err[0xff];
    DWORD pid = FindProcess("notepad.exe");
    if( pid == -1 ) {
        goto error;
    }
    else if (pid == ERROR_PROCESS_NOT_FOUND) {
        std::cout << "No such process!\n";
        return -1;
    }
    if( !KillProcess( pid ) )
        return -1;

    return 0;
    error:
        WinStrerror( err, sizeof(err), -1 );
        return -1;
}

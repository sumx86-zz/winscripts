#include "injector.h"

bool dllinject::GetProcList( VOID ) {
    HANDLE hndl = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if( hndl == INVALID_HANDLE_VALUE ) {
#ifdef DEBUG
    std::cerr << "CreateToolhelp32Snapshot() - " << winstrerror(GetLastError()) << std::endl;
#endif
        return false;
    }

    PROCESSENTRY32 pe32;
    bool ret = Process32First( hndl, &pe32 );
    if ( !ret ) {
#ifdef DEBUG
    std::cerr << "Process32First() - " << winstrerror(GetLastError()) << std::endl;
#endif
        return false;
    }
    do {
        _proclist.push_back(pe32);
    }
    while( Process32Next( hndl, &pe32 ) );

    CloseHandle(hndl);
    return true;
}

bool dllinject::FindProcessByName( const char *name ) {
    if( !GetProcList() )
        return false;

    std::vector<PROCESSENTRY32>::iterator it;

    for( it = _proclist.begin(); it != _proclist.end(); it++ ) {
        if( _stricmp( it->szExeFile, name ) == 0 ) {
            _procid = it->th32ProcessID;
            return true;
        }
    }
    return false;
}

HANDLE dllinject::GetPToken( VOID ) {
    _prochndl = OpenProcess( PROCESS_QUERY_INFORMATION, false, _procid );
    if( _prochndl == NULL ) {
#ifdef DEBUG
    std::cerr << winstrerror(GetLastError()) << std::endl;
#endif
        return NULL;
    }
    HANDLE token = NULL;
    bool tok = OpenProcessToken( _prochndl, TOKEN_ADJUST_PRIVILEGES, &token );
    if( !tok ) {
#ifdef DEBUG
    std::cerr << winstrerror(GetLastError()) << std::endl;
#endif
        return NULL;
    }
    return token;
}

bool dllinject::SetDebugPriv( HANDLE token, bool enable ) {
    return true;
}

bool dllinject::IsProcess64( VOID ) {
    LPFN_ISWOW64PROC iswow64process = (LPFN_ISWOW64PROC) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if( iswow64process ) {
        BOOL iswow64 = false;
        if( iswow64process(_prochndl, &iswow64) ) {
            return iswow64;
        }
    }
#ifdef DEBUG
    std::cerr << winstrerror(GetLastError()) << std::endl;
#endif
    return false;
}

bool dllinject::InjectDll( const char *dllpath ) {
    std::cout << dllpath;
    return true;
}

const char * __stdcall winstrerror( DWORD code ) {
    char *str;
    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        code,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR) &str,
                        0,
                        NULL);
    static char err[0xff];
    strcpy_s( err, str ), LocalFree( str );
    return err;
}
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
    _ptoken = NULL;
    bool tok = OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &_ptoken );
    if( !tok ) {
        #ifdef DEBUG
            std::cerr << winstrerror(GetLastError()) << std::endl;
        #endif
        return NULL;
    }
    return _ptoken;
}

bool dllinject::SetDebugPriv( HANDLE token ) {
    LUID luid = {NULL};
    if( !LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &luid ) ) {
        #ifdef DEBUG
            std::cerr << winstrerror(GetLastError()) << std::endl;
        #endif
        return false;
    }
    // process' token privileges
    TOKEN_PRIVILEGES privileges = {NULL};
    // new privileges
    privileges.PrivilegeCount = 1;
    privileges.Privileges[0].Luid = luid;
    privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    DWORD bufsz = sizeof(privileges);
    bool ret = AdjustTokenPrivileges( token, false, &privileges,
                bufsz, NULL, NULL );
    if( !ret ) {
        #ifdef DEBUG
            std::cerr << winstrerror(GetLastError()) << std::endl;
        #endif
        return false;
    }
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

bool dllinject::InjectDll( std::string& dllpath ) {
    // get the token of the current process
    if( GetPToken() == NULL )
        return false;
    // enable the SE_DEBUG_NAME privilege
    if( !SetDebugPriv( _ptoken ) )
        return false;

    _prochndl = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_WRITE |
                PROCESS_VM_OPERATION , false, _procid );
    if( _prochndl == NULL ) {
        #ifdef DEBUG
            std::cerr << winstrerror(GetLastError()) << std::endl;
        #endif
        return false;
    }
    size_t size = dllpath.length();
    LPVOID baseaddr = VirtualAllocEx( _prochndl, NULL, size, MEM_COMMIT, PAGE_READWRITE );
    if( baseaddr == NULL ) {
        #ifdef DEBUG
            std::cerr << winstrerror(GetLastError()) << std::endl;
        #endif
        return false;
    }
    size_t written = 0;
    if( !WriteProcessMemory( _prochndl, baseaddr, dllpath.c_str(), size, &written ) ) {
        #ifdef DEBUG
            std::cerr << winstrerror(GetLastError()) << std::endl;
        #endif
        return false;
    }
    _thread = CreateRemoteThread( _prochndl, NULL, 0, func, baseaddr,  );
    if( !_thread ) {
        #ifdef DEBUG
            std::cerr << winstrerror(GetLastError()) << std::endl;
        #endif
        return false;
    }
    std::cout << dllpath << " -- " << size << "\n";
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
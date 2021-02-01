#include "injector.h"

typedef BOOL (__stdcall *LPFN_ISWOW64PROC)(HANDLE, PBOOL);

typedef NTSTATUS (WINAPI *LPFUN_NtCreateThreadEx)
(
    OUT PHANDLE hThread,
    IN ACCESS_MASK DesiredAccess,
    IN LPVOID ObjectAttributes,
    IN HANDLE ProcessHandle,
    IN LPTHREAD_START_ROUTINE lpStartAddress,
    IN LPVOID lpParameter,
    IN BOOL CreateSuspended,
    IN ULONG StackZeroBits,
    IN ULONG SizeOfStackCommit,
    IN ULONG SizeOfStackReserve,
    OUT LPVOID lpBytesBuffer
);

struct NtCreateThreadExBuffer {
    ULONG Size;
    ULONG Unknown1;
    ULONG Unknown2;
    PULONG Unknown3;
    ULONG Unknown4;
    ULONG Unknown5;
    ULONG Unknown6;
    PULONG Unknown7;
    ULONG Unknown8;
};

#define GET_THREAD_ROUTINE()\
    (LPVOID) GetProcAddress( GetModuleHandle(TEXT("kernel32")), "LoadLibraryW")

#define GET_NTTHREAD_FUNC()\
    (LPVOID) GetProcAddress( GetModuleHandle(TEXT("ntdll")), "NtCreateThreadEx" );

dllinject::dllinject( const char *name ) {
    if( !FindProcessByName( name ) ) {
        #ifdef DEBUG
            std::cerr << "No such process! [" << name << "]" << std::endl;
        #endif
        std::exit(2);
    }
    _prochndl = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_WRITE |
                PROCESS_VM_OPERATION , false, _procid );
    if( _prochndl == NULL ) {
        #ifdef DEBUG
            std::cerr << winstrerror(GetLastError()) << std::endl;
        #endif
        std::exit(2);
    }
}

dllinject::~dllinject() {
    VirtualFreeEx( _prochndl, _lpaddr, sizeof(_lpaddr), 0 );
    if( _prochndl ) CloseHandle( _prochndl );
    if( _ptoken )   CloseHandle( _ptoken );
    if( _thread )   CloseHandle( _thread );
}

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
    #ifdef _WIN64
        LPFN_ISWOW64PROC iswow64process = (LPFN_ISWOW64PROC) GetProcAddress(
            GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

        if( iswow64process ) {
            BOOL iswow64 = false;
            if( iswow64process( _prochndl, &iswow64 ) ) {
                // if the process is running under WOW64 return false, otherwise return true
                return !iswow64;
            }
        }
        #ifdef DEBUG
            std::cerr << winstrerror(GetLastError()) << std::endl;
            return false;
        #endif
    #endif
    #ifdef _WIN32
        return false;
    #endif
}

NTSTATUS dllinject::XCreateRemoteThread( HANDLE proc, PHANDLE thread, LPTHREAD_START_ROUTINE func, LPVOID param ) {
    NtCreateThreadExBuffer ntbuffer;

    ZeroMemory( &ntbuffer, sizeof(NtCreateThreadExBuffer) );
    DWORD temp1 = 0x00;
    DWORD temp2 = 0x00;

    ntbuffer.Size = sizeof(NtCreateThreadExBuffer);
    ntbuffer.Unknown1 = 0x10003;
    ntbuffer.Unknown2 = 0x8;
    ntbuffer.Unknown3 = &temp2;
    ntbuffer.Unknown4 = 0;
    ntbuffer.Unknown5 = 0x10004;
    ntbuffer.Unknown6 = 4;
    ntbuffer.Unknown7 = &temp1;
    ntbuffer.Unknown8 = 0;

    LPFUN_NtCreateThreadEx NtCreateThreadEx = (LPFUN_NtCreateThreadEx) GET_NTTHREAD_FUNC();
    NTSTATUS status = NtCreateThreadEx( thread, 0x1FFFFF, NULL, proc,
        (LPTHREAD_START_ROUTINE) func,
        param,
        FALSE,
        NULL,
        NULL,
        NULL,
        &ntbuffer );
    return status;
}

bool dllinject::InjectDll( const std::wstring& dllpath ) {
    // get the token of the current process
    if( GetPToken() == NULL )
        return false;
    // enable the SE_DEBUG_NAME privilege
    if( !SetDebugPriv( _ptoken ) )
        return false;

    size_t size = dllpath.length() * sizeof(wchar_t);
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
    PTHREAD_START_ROUTINE routine = reinterpret_cast<PTHREAD_START_ROUTINE >(GET_THREAD_ROUTINE());
    if( XCreateRemoteThread( _prochndl, &_thread, routine, baseaddr ) ) {
        #ifdef DEBUG
            std::cerr << winstrerror(GetLastError()) << std::endl;
        #endif
        return false;
    }
    return true;
}

bool dllinject::InjectDll( const std::string& dllpath ) {
    std::wstring libpath( dllpath.begin(), dllpath.end() );
    return InjectDll( libpath );
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
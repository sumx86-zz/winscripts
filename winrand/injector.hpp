#ifndef __injecTor_HH
#define __injecTor_HH 1

#include <iostream>
#include <vector>
#include <windows.h>
#include <tlhelp32.h>

const char * __stdcall winstrerror( DWORD code );

#define ISNULL(ptr) (((ptr) == NULL) || ((ptr) == nullptr))

class dllinject {
    public:
        dllinject( DWORD pid ) {
            if( !FindProcessById( pid ) ) {
                std::exit(1);
            }
        }

        dllinject( const char *name ) {
            if( !FindProcessByName( name ) ) {
                std::exit(1);
            }
        }

        ~dllinject() {
            VirtualFreeEx( _prochndl, _lpaddr, sizeof(_lpaddr), 0 );
            CloseHandle(_prochndl);
            std::cout << "aaa";
        }

    public:
        bool GetProcList( VOID );
        bool FindProcessById( DWORD pid );
        bool FindProcessByName( const char *name );

    private:
        DWORD _procid;
        HANDLE _prochndl;
        LPVOID _lpaddr;
        std::vector<PROCESSENTRY32> _proclist;
};

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

bool dllinject::FindProcessById( DWORD pid ) {
    if( !GetProcList() )
        return false;

    std::vector<PROCESSENTRY32>::iterator it;

    for( it = _proclist.begin(); it != _proclist.end(); it++ ) {
        if( it->th32ProcessID == pid ) {
            _procid = it->th32ProcessID;
            return true;
        }
    }
    return false;
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

#endif
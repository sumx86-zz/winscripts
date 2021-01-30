#ifndef __injecTor_HH
#define __injecTor_HH 1

#include "pe_parser.h"
#include <tlhelp32.h>

const char * __stdcall winstrerror( DWORD code );

typedef BOOL (__stdcall *LPFN_ISWOW64PROC)(HANDLE, PBOOL);

class dllinject {
    public:
        dllinject( const char *name ) {
            if( !FindProcessByName( name ) ) {
                #ifdef DEBUG
                    std::cerr << "No such process! [" << name << "]" << std::endl;
                #endif
                std::exit(2);
            }
        }

        ~dllinject() {
            VirtualFreeEx( _prochndl, _lpaddr, sizeof(_lpaddr), 0 );
            if( _prochndl )
                CloseHandle( _prochndl );   

            if( _ptoken )
                CloseHandle( _ptoken );

            if( _thread )
                CloseHandle( _thread );

            std::cout << "aaa";
        }

    public:
        bool FindProcessByName( const char *name );
        bool InjectDll( std::string& dllpath );
        HANDLE GetPToken( VOID );
        bool SetDebugPriv( HANDLE token );
        bool IsProcess64( VOID );

    private:
        bool GetProcList( VOID );

    private:
        DWORD _procid;
        HANDLE _prochndl;
        HANDLE _ptoken;
        HANDLE _thread;
        LPVOID _lpaddr;
        std::vector<PROCESSENTRY32> _proclist;
};

#endif
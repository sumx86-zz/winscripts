#ifndef __injecTor_HH
#define __injecTor_HH 1

#include "pe_parser.h"
#include <tlhelp32.h>

const char * __stdcall winstrerror( DWORD code );

class dllinject {
    public:
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
        bool FindProcessByName( const char *name );

    private:
        DWORD _procid;
        HANDLE _prochndl;
        LPVOID _lpaddr;
        std::vector<PROCESSENTRY32> _proclist;
};

#endif
#include <stdio.h>
#include <winsock2.h>
#include "include\util.h"
#include <iphlpapi.h>

PMIB_IPNETTABLE wNetTable = NULL;
DWORD wNetSize = 0;

static VOID quitp( const char *str ) {
    std::cout << str;
    std::exit(1);
}

static DWORD wInitTable( VOID ) {
    ULONG ret = GetIpNetTable( wNetTable, &wNetSize, false );

    if ( ret == ERROR_INSUFFICIENT_BUFFER ) {
        wNetTable = (PMIB_IPNETTABLE) malloc( wNetSize );
        if( wNetTable == NULL ) {
            return -1;
        }
    }
    return wNetSize;
}

static std::vector<wArpEntry> wListNetEntries( VOID ) {
    ULONG ret = GetIpNetTable( wNetTable, &wNetSize, false );
    if ( ret != NO_ERROR ) {
        return {};
    }
    std::vector<wArpEntry> entries = {};
    wArpEntry wEntry;
    if ( wNetTable->dwNumEntries > 0 ) {
        for ( int i = 0 ; i < wNetTable->dwNumEntries ; i++ ) {
            wEntry.ipv4 = inet_ntoa(*(struct in_addr *)&wNetTable->table[i].dwAddr);
            wEntry.idx  = wNetTable->table[i].dwIndex;
            //wEntry.hwwd = wNetTable->table[i].bPhysAddr;
            sscanf(
                "%d.%d.%d.%d.%d.%d",
                    wNetTable->table[i].bPhysAddr[0],
                    wNetTable->table[i].bPhysAddr[1],
                    wNetTable->table[i].bPhysAddr[2],
                    wNetTable->table[i].bPhysAddr[3],
                    wNetTable->table[i].bPhysAddr[4],
                    wNetTable->table[i].bPhysAddr[5]
            );
            entries.push_back(wEntry);
        }
    }
    return entries;
}

int main( int argc, char **argv )
{
    if( wOsSupported() < 0 ) {
        quitp("Windows Server 2000 or later required!");
    }
    ULONG ini;
    if( (ini = wInitTable()) == -1 ) {
        quitp("Table initialization error!");
    }
    std::vector<wArpEntry> entries = wListNetEntries();
    if ( !entries.empty() ) {
        for( auto entry : entries ) {
            std::cout << entry.ipv4 << "\n";
        }
    }
    free(wNetTable);
    return 0;
}
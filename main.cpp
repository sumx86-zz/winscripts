#include <stdio.h>
#include <winsock2.h>
#include "include\util.h"
#include <iphlpapi.h>
#include <sstream>

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

static VOID wSetNetEntry( wArpEntry &wEntry, MIB_IPNETROW *table ) {
    char str[20];
    wEntry.ipv4 = std::string(inet_ntoa(*(struct in_addr *)&table->dwAddr));
    wEntry.idx  = table->dwIndex;
    sprintf( str,
            "%x:%x:%x:%x:%x:%x",
            table->bPhysAddr[0], table->bPhysAddr[1],
            table->bPhysAddr[2], table->bPhysAddr[3],
            table->bPhysAddr[4], table->bPhysAddr[5]
    );
    wEntry.hwwd = std::string(str);
}

static std::vector<wArpEntry> wListNetEntries( VOID ) {
    ULONG ret = GetIpNetTable( wNetTable, &wNetSize, false );
    if ( GetIpNetTable( wNetTable, &wNetSize, false ) != NO_ERROR ) {
        return {};
    }
    std::vector<wArpEntry> entries = {};
    wArpEntry wEntry;
    if ( wNetTable->dwNumEntries > 0 ) {
        for ( int i = 0 ; i < wNetTable->dwNumEntries ; i++ ) {
            wSetNetEntry( wEntry, &wNetTable->table[i] );
            entries.push_back(wEntry);
        }
    }
    return entries;
}

int main( int argc, char **argv )
{
    ULONG ini;
    if( (ini = wInitTable()) == -1 ) {
        quitp("Table initialization error!");
    }
    std::vector<wArpEntry> entries = wListNetEntries();
    if ( !entries.empty() ) {
        for( auto entry : entries ) {
            std::cout << entry.ipv4 << " - " << entry.hwwd << " - " << entry.idx << "\n";
        }
    }
    free(wNetTable);
    return 0;
}
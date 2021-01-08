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

static DWORD winit_table( VOID ) {
    ULONG ret = GetIpNetTable( wNetTable, &wNetSize, false );
    
    if ( ret == ERROR_INSUFFICIENT_BUFFER ) {
        wNetTable = (PMIB_IPNETTABLE) malloc( wNetSize );
        if( wNetTable == NULL ) {
            return -1;
        }
    }
    return wNetSize;
}

static VOID wset_net_entry( wArpEntry &wEntry, MIB_IPNETROW *table ) {
    std::string str;
    wEntry.ipv4 = std::string(inet_ntoa(*(struct in_addr *)&table->dwAddr));
    wEntry.idx  = table->dwIndex;
    str += to_hex(table->bPhysAddr[0] & 0xff) + ":";
    str += to_hex(table->bPhysAddr[1] & 0xff) + ":";
    str += to_hex(table->bPhysAddr[2] & 0xff) + ":";
    str += to_hex(table->bPhysAddr[3] & 0xff) + ":";
    str += to_hex(table->bPhysAddr[4] & 0xff) + ":";
    str += to_hex(table->bPhysAddr[5] & 0xff);
    wEntry.hwwd = std::string(str);
}

static std::vector<wArpEntry> wlist_net_entries( VOID ) {
    ULONG ret = GetIpNetTable( wNetTable, &wNetSize, false );
    if ( GetIpNetTable( wNetTable, &wNetSize, false ) != NO_ERROR ) {
        return {};
    }
    std::vector<wArpEntry> entries = {};
    wArpEntry wEntry;
    if ( wNetTable->dwNumEntries > 0 ) {
        for ( int i = 0 ; i < wNetTable->dwNumEntries ; i++ ) {
            wset_net_entry( wEntry, &wNetTable->table[i] );
            entries.push_back(wEntry);
        }
    }
    return entries;
}

int main( int argc, char **argv )
{
    ULONG ini;
    if( (ini = winit_table()) == -1 ) {
        quitp("Table initialization error!");
    }
    std::vector<wArpEntry> entries = wlist_net_entries();
    if ( !entries.empty() ) {
        for( auto entry : entries ) {
            std::cout << entry.ipv4 << " - " << entry.hwwd << " - " << entry.idx << "\n";
        }
    }
    free(wNetTable);
    return 0;
}
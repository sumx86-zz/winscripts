#include "include\warp.h"

static VOID quitp( const char *str ) {
    std::cout << str;
    std::exit(1);
}

static DWORD init_net_table( VOID ) {
    ULONG ret = GetIpNetTable( wNetTable, &wNetSize, false );

    if ( ret == ERROR_INSUFFICIENT_BUFFER ) {
        wNetTable = (PMIB_IPNETTABLE) malloc( wNetSize );
        if( wNetTable == NULL ) {
            return -1;
        }
    }
    ZeroMemory( wNetTable, sizeof(wNetTable) );
    return wNetSize;
}

static VOID set_net_entry( wArpEntry &wEntry, MIB_IPNETROW *table ) {
    char str[20];
    wEntry.ipv4 = std::string(inet_ntoa(*(struct in_addr *)&table->dwAddr));
    wEntry.idx  = table->dwIndex;
    sprintf( str,
            "%02x:%02x:%02x:%02x:%02x:%02x",
            table->bPhysAddr[0],
            table->bPhysAddr[1],
            table->bPhysAddr[2],
            table->bPhysAddr[3],
            table->bPhysAddr[4],
            table->bPhysAddr[5]
    );
    wEntry.hwwd = std::string(str);
}

static std::vector<wArpEntry> list_net_entries( VOID ) {
    ULONG ret = GetIpNetTable( wNetTable, &wNetSize, false );
    if ( ret != NO_ERROR ) {
        return {};
    }
    std::vector<wArpEntry> entries = {};
    wArpEntry wEntry;
    if ( wNetTable->dwNumEntries > 0 ) {
        for ( uint32_t i = 0 ; i < wNetTable->dwNumEntries ; i++ ) {
            set_net_entry( wEntry, &wNetTable->table[i] );
            entries.push_back(wEntry);
        }
    }
    return entries;
}

/* check if an entry exists in the arp table based on the entry's IP address */
static BOOL entry_exists( wArpEntry entry, std::vector<wArpEntry> entries ) {
    for ( auto _entry : entries ) {
        if ( _entry.ipv4 == entry.ipv4 ) {
            return true;
        }
    }
    return false;
}

int main( int argc, char **argv )
{
    PMIB_IPNETROW pArpEntry = NULL;
    /* if( init_net_table() == -1 ) {
        quitp("Table initialization error! [malloc() error]");
    }
    std::vector<wArpEntry> entries = list_net_entries();
    if ( !entries.empty() ) {
        for( auto entry : entries ) {
            std::cout << entry.ipv4 << " - " << entry.hwwd << " - " << entry.idx << "\n";
        }
    }
    free(wNetTable); */
    char str[25] = "b0:4E:26:6d:C2:48";
    uint8_t hw[6] = {0};
    if ( ether_aton( str, hw ) == 0 ) {
        for ( int i = 0 ; i < 6 ; i++ ) {
            printf( "%x:", hw[i] );
        }
    }
    /* std::vector<wArpEntry> entries = parse_net_file("C:\\Users\\babati\\antocpp\\arplist.txt");
    if ( !entries.empty() ) {
        for ( auto entry : entries ) {
            std::cout << entry.wfipv4 << " - " << entry.wfhwwd << "\n";
        }
    } */
    return 0;
}
#include "include\warp.h"

void usage( const char *prog ) {
    std::cout << "Usage: " << prog << " [net file]" << "\n";
    std::exit(1);
}

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

static SHORT net_update( wArpEntry entry, NET_OPT opt ) {
    uint8_t hw[6];
    ZeroMemory( hw, sizeof(hw) );

    if ( ether_aton( entry.hwwd.c_str(), hw ) < 0 )
        return -1;

    MIB_IPNETROW arp_entry;
    arp_entry.dwPhysAddrLen = 6;
    memcpy( arp_entry.bPhysAddr, hw, sizeof(hw));
    arp_entry.dwAddr = inet_addr( entry.ipv4.c_str() );
    arp_entry.dwType = MIB_IPNET_TYPE_STATIC;

    // modify existing entry
    if ( opt == NET_MODIFY )
        SetIpNetEntry( &arp_entry );

    // create a new entry
    if ( opt == NET_CREATE )
        CreateIpNetEntry( &arp_entry );

    return 0;
}

static std::vector<wArpEntry> list_net_entries( VOID ) {
    if( init_net_table() == -1 ) {
        quitp("Table init error!");
    }
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
/* this checks only the ip address and not the MAC address */
/* so if the IP exists the entry will be overriden, but with (possibly) differet MAC address */
static bool net_entry_exists( wArpEntry entry, std::vector<wArpEntry> entries ) {
    for ( auto _entry : entries ) {
        if ( _entry.ipv4 == entry.ipv4 ) {
            return true;
        }
    }
    return false;
}

int main( int argc, char **argv )
{
    if ( argc != 2 )
        usage("./warp");

    std::vector<wArpEntry> entries = list_net_entries();
    if ( entries.empty() ) {
        free(wNetTable);
        quitp("No entries found in arp table!");
    }
    std::vector<wArpEntry> f_entries = parse_net_file(std::string(argv[1]));
    if ( f_entries.empty() ) {
        quitp("No entries found in net file!");
    }
    for ( auto entry : f_entries ) {
        if ( net_update( entry ) < 0 ) {
            quitp("Error updating arp table!");
        }
    }
    free(wNetTable);
    return 0;
}
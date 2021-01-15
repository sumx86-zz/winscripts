#include "include\s-arp.h"

static void __stdcall usage( const char *prog ) {
    std::cout << "Usage: " << prog << " [net file]" << "\n";
    std::exit(1);
}

static VOID __stdcall quitp( const char *str, LPVOID ptr ) {
    std::cout << str << "\n";
    if ( ptr != NULL )
        HeapFree( GetProcessHeap(), 0, ptr );
    std::exit(1);
}

static DWORD __stdcall init_net_table( VOID ) {
    ULONG ret = GetIpNetTable( wNetTable, &wNetSize, false );

    if ( ret == ERROR_INSUFFICIENT_BUFFER ) {
        wNetTable = (PMIB_IPNETTABLE) HeapAlloc( GetProcessHeap(), 0, wNetSize );
        if( wNetTable == NULL )
            return -1;
    }
    ZeroMemory( wNetTable, sizeof(wNetTable) );
    return wNetSize;
}

static VOID __stdcall set_net_entry( wArpEntry &wEntry, MIB_IPNETROW *table ) {
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

static SHORT __stdcall net_update( wArpEntry entry, NET_OPT opt ) {
    BYTE hw_addr[MAXLEN_PHYSADDR];
    ZeroMemory( hw_addr, sizeof(hw_addr) );

    if ( !SUCCEEDED( ether_aton( entry.hwwd.c_str(), hw_addr ) ) )
        return -1;

    MIB_IPNETROW arp_entry;
    arp_entry.dwPhysAddrLen = ADDRESS_ETHERNET_LENGTH;
    memcpy( arp_entry.bPhysAddr, hw_addr, sizeof(hw_addr));
    arp_entry.dwAddr = (DWORD) inet_addr( entry.ipv4.c_str() );
    arp_entry.dwIndex = entry.idx;
    arp_entry.dwType = MIB_IPNET_TYPE_STATIC;

    DWORD stat = 0;
    // modify existing entry
    switch ( opt ) {
        case NET_MODIFY:
            #ifdef DEBUG
                std::cout << "modifying - " << entry.ipv4 << " - " << arp_entry.dwType << "\n";
            #endif
            // first delete the entry then recreate it
            // I'm using this method since I couldn't get SetIpNetEntry() to work :(
            stat = DeleteIpNetEntry( &arp_entry );
            if ( stat != NO_ERROR ) {
                goto bad;
            }
            stat = CreateIpNetEntry( &arp_entry );
            if ( stat != NO_ERROR ) {
                goto bad;
            }
            break;
        case NET_CREATE:
            #ifdef DEBUG
                std::cout << "creating - " << entry.ipv4 << " - " << arp_entry.dwType << "\n";
            #endif
            stat = CreateIpNetEntry( &arp_entry );
            if ( stat != NO_ERROR ) {
                goto bad;
            }
            break;
    }
    return 0;
    bad:
        #ifdef DEBUG
            winstrerror(globalErr, sizeof(globalErr), stat);
            std::cout << "Error -> [" << globalErr << "]" << "\n";
        #endif
        return -1;
}

static std::vector<wArpEntry> __stdcall list_net_entries( VOID ) {
    if( !SUCCEEDED( init_net_table() ) )
        quitp("Table init error! [MALLOC()]", NULL);

    ULONG ret = GetIpNetTable( wNetTable, &wNetSize, false );
    if ( ret != NO_ERROR )
        return {};

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
static bool __stdcall net_entry_exists( wArpEntry entry, std::vector<wArpEntry> entries ) {
    for ( auto _entry : entries ) {
        if ( _entry.ipv4 == entry.ipv4 ) {
            return true;
        }
    }
    return false;
}

int __stdcall main( int argc, char **argv )
{
    if ( argc != 2 )
        usage("./s-arp");

    std::vector<wArpEntry> entries = list_net_entries();
    if ( entries.empty() ) {
        quitp("GetIpNetTable() error!", wNetTable);
    }
    std::vector<wArpEntry> f_entries = parse_net_file(std::string(argv[1]));
    if ( f_entries.empty() ) {
        quitp("No entries found in net file!", wNetTable);
    }
    for ( auto entry : f_entries ) {
        if ( net_update( entry, net_entry_exists( entry, entries ) ? NET_MODIFY : NET_CREATE ) < 0 ) {
            quitp("Error updating arp table!", wNetTable);
        }
    }
    HeapFree(GetProcessHeap(), 0, wNetTable);
    return 0;
}

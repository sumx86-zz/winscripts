#ifndef __sarp_H
#define __sarp_H 1

#include <stdio.h>
#include <winsock2.h>
#include "util.h"
#include <iphlpapi.h>

PMIB_IPNETTABLE wNetTable = NULL;
DWORD wNetSize = 0;

// globally accessible error buffer
char globalErr[0xff];

typedef enum {
    NET_MODIFY = 0,
    NET_CREATE
} NET_OPT;

#ifndef ADDRESS_ETHERNET_LENGTH
    #define ADDRESS_ETHERNET_LENGTH 6
#endif

static VOID quitp( const char *str );
static DWORD init_net_table( VOID );
static VOID set_net_entry( wArpEntry &wEntry, MIB_IPNETROW *table );
static SHORT net_update( wArpEntry entry, NET_OPT opt );
static std::vector<wArpEntry> list_net_entries( VOID );
static bool net_entry_exists( wArpEntry entry, std::vector<wArpEntry> entries );

#endif

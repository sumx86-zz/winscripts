#ifndef __util_H
#define __util_H 1

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <Windows.h>

typedef struct {
    std::string ipv4;
    std::string hwwd;
    ULONG idx;
} wArpEntry;

std::vector<wArpEntry> parse_net_file( std::string filename );
std::vector<std::string> str_split( std::string line, const char delim );
bool is_xdigit( char c );
short xdec( char c, short base );
SHORT ether_aton( const char *str, uint8_t *hw );
SHORT __stdcall winstrerror( char *err, size_t size, DWORD errcode );

#endif

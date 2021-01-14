#include "include\util.h"

std::vector<wArpEntry> parse_net_file( std::string filename ) {
    std::ifstream filestream(filename);
    if ( !filestream.is_open() )
        throw "Error opening file" + filename;

    std::vector<wArpEntry> entries;
    wArpEntry entry;

    for ( std::string line ; std::getline(filestream, line) ; ) {
        auto tokens = str_split( line, '-' );
        entry.ipv4 = tokens[0];
        entry.hwwd = tokens[1];
        entry.idx  = (ULONG) std::stoi(tokens[2]);
        entries.push_back( entry );
    }
    return entries;
}

std::vector<std::string> str_split( std::string line, const char delim ) {
    std::stringstream ss(line);
    std::vector<std::string> tokens;

    for ( std::string s ; std::getline(ss, s, delim) ; ) {
        if ( !s.empty() )
            tokens.push_back( s );
    }
    return tokens;
}

static bool is_xdigit( char c ) {
    if ( !(c >= 'A' && c <= 'F') &&
         !(c >= 'a' && c <= 'f') &&
         !(c >= '0' && c <= '9') ) {
        return false;
    }
    return true;
}

static SHORT xdec( char c, short base ) {
    if ( c >= '0' && c <= '9' )
        return (c - '0') * base;
    if ( c >= 'A' && c <= 'F' )
        return (c - 55) * base;
    if ( c >= 'a' && c <= 'f' )
        return (c - 32 - 55) * base;
}

SHORT ether_aton( const char *str, uint8_t *hw ) {
    if ( !str )
        return -1;

    if ( *(str + 2)  != ':' ||
         *(str + 5)  != ':' ||
         *(str + 8)  != ':' ||
         *(str + 11) != ':' ||
         *(str + 14) != ':') {
        return -1;
    }

    int i   = 0;
    int xd1 = 0;
    int xd2 = 0;

    while ( *str != '\x00' ) {
        if ( *str == ':' )
            str++;

        if ( !is_xdigit(*str) || !is_xdigit(*(str + 1)) )
            goto bad;

        xd1 = xdec(*str++, 16);
        xd2 = xdec(*str++,  1);
        hw[i++] = xd1 + xd2;
    }
    return i;
    
    bad:
        ZeroMemory( hw, 6 ), i = 0;
        return -1;
}

SHORT __stdcall winstrerror( char *err, size_t size, DWORD errcode ) {
    DWORD errCode = (errcode > 0) ? errcode : GetLastError();
    char *str;
    if (!FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        errCode,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR) &str,
                        0,
                        NULL))
        return -1;
    memcpy( err, str, size ), LocalFree( str );
    return 0;
}

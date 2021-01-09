#include "include\util.h"

std::vector<wFileEntry> parse_net_file( std::string filename ) {
    std::ifstream filestream(filename);
    if ( !filestream.is_open() )
        throw "Error opening file" + filename;

    std::vector<wFileEntry> entries;
    wFileEntry entry;

    for ( std::string line ; std::getline(filestream, line) ; ) {
        auto tokens = str_split( line, '-' );
        entry.wfipv4 = tokens[0];
        entry.wfhwwd = tokens[1];
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

SHORT ether_aton( const char *str, uint8_t *hw ) {
    if ( !str )
        return -1;
    
    while ( *str != '\x00' ) {
        str++;
    }
    return 0;
}
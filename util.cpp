#include "include\util.h"

std::vector<wArpEntry> wParseNetFile( std::string filename ) {
    return {};
}

SHORT wOsSupported( VOID ) {
    DWORD dwMajorVersion = 0;
    DWORD dwVersion = 0;

    dwVersion = GetVersion();
    dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));

    if( dwVersion >= 0x80000000 || dwMajorVersion < 6 )
        return -1;
    return 0;
}
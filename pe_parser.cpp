#include "pe_parser.h"

namespace PE_PARSER {
    LPVOID FpBaseAddr = NULL;
    HANDLE file;
    HANDLE mapping;

    LPVOID LoadFile( const char *path ) {
        if( !FileExists( path ) )
            return NULL;

        file = CreateFileA( path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
        if( file == INVALID_HANDLE_VALUE ) {
#ifdef DEBUG
    std::cerr << "Error opening file!\n";
#endif
            return NULL;
        }
        mapping = CreateFileMappingA( file, 0, PAGE_READONLY, 0, 0, 0 );
        if( mapping == NULL ) {
#ifdef DEBUG
    std::cerr << "Error creating file mapping!\n";
#endif
            goto error;
        }

        FpBaseAddr = MapViewOfFile( mapping, FILE_MAP_READ, 0, 0, 0 );
        if( FpBaseAddr == NULL ) {
#ifdef DEBUG
    std::cerr << "Error mapping file!\n";
#endif
            goto error;
        }
        return FpBaseAddr;

    error:
        Clear(); return NULL;
    }

    bool IsDllFile( VOID ) {
        BYTE *localBase = reinterpret_cast<BYTE *>(FpBaseAddr);
        PIMAGE_DOS_HEADER doshdr = (PIMAGE_DOS_HEADER) localBase;

        if( doshdr->e_magic != IMAGE_DOS_SIGNATURE )
            return false;

        PIMAGE_FILE_HEADER pfhdr = (PIMAGE_FILE_HEADER) PEHEADER(localBase);
        return !(pfhdr->Characteristics & IMAGE_FILE_DLL) ? false : true ;
    }

    /* check if the dll is compiled 32bit or 64bit */
    WORD DllMachine( VOID ) {
        BYTE *localBase = reinterpret_cast<BYTE *>(FpBaseAddr);
        PIMAGE_FILE_HEADER pfhdr = (PIMAGE_FILE_HEADER) PEHEADER(localBase);

        if( pfhdr->Machine == 0x14c )
            return DLL_X86;

        if( pfhdr->Machine == 0x8664 )
            return DLL_X64;
        
        return -1;
    }

    bool FileExists( const char *path ) {
        DWORD fattr = GetFileAttributesA( path );
        if( fattr != INVALID_FILE_ATTRIBUTES && (!(fattr & FILE_ATTRIBUTE_ARCHIVE) && !(fattr & FILE_ATTRIBUTE_DIRECTORY)) ) {
#ifdef DEBUG
    std::cerr << "Path (" << path << ") " << "does not exist!\n";
#endif
            return false;
        }
        return true;
    }

    VOID Clear( VOID ) {
        if( FpBaseAddr != NULL )
            UnmapViewOfFile(FpBaseAddr);
            
        if( file != NULL )
            CloseHandle(file);

        if( mapping != NULL )
            CloseHandle(mapping);
    }
};
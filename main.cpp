#include "injector.h"

typedef std::shared_ptr<dllinject> dllinjector;

int main( int argc, char **argv )
{
    char *dllpath = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\PAYDAY The Heist\\D3DX9_40.dll";
    char *addr = (char *) PE_PARSER::LoadFile(dllpath);
    if( addr == NULL ) {
        return -1;
    }
    if( !PE_PARSER::IsDllFile() ) {
        std::cout << "[ERROR] File is not a dll!\n";
    }

    std::cout << PE_PARSER::DllMachine();

    dllinjector injector = dllinjector(new dllinject("notepad.exe"));
    if (injector->InjectDll(dllpath)) {
        std::cout << "[OK] DLL injected successfully."<< std::endl;
    } else {
        std::cerr << "[ERROR] Could not inject DLL!" << std::endl;
        return -1;
    }
    PE_PARSER::Clear();
    return 0;
}
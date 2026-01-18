#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(WIN32) || defined(_WINDOWS)
#include <windows.h>
#elif defined(__linux__) || defined(__linux) || defined(_linux) || __APPLE__
#include <dlfcn.h>
#if __APPLE__
#include <mach-o/dyld.h>	
#include <limits.h>
#include <libgen.h>
#endif
typedef void*  HMODULE;
typedef long   HRESULT;
#define LoadLibraryA(x) dlopen((x), RTLD_LOCAL | RTLD_LAZY)
#define FreeLibrary(x) dlclose((x))
#define GetProcAddress(x, y) dlsym((x), (y))
#else
#error Unsupported platform
#endif

typedef const char16_t* (*GetClassNamesPtr)();
typedef const HRESULT   (*DllUnregisterServerPtr)();

const char kGetClassNames[]          = "GetClassNames";
const char kDllUnregisterServer[]          = "DllUnregisterServer";



int main(int argc, char **argv)
{
    HMODULE hModule = nullptr;
    GetClassNamesPtr proc = nullptr;
    DllUnregisterServerPtr unregsrv  = nullptr;
    int ret = 0;

    if (argc < 2)
    {
        printf("Missing filename!\n");
        exit(EXIT_FAILURE);
    }
    hModule = LoadLibraryA(argv[1]);
    
    if (hModule != 0)
        printf("Success load %s!\n", argv[1]);
    else
        exit(EXIT_FAILURE);

    proc = (GetClassNamesPtr)GetProcAddress(hModule, kGetClassNames);
    if (!proc)
    {
        unregsrv = (DllUnregisterServerPtr)GetProcAddress(hModule, kDllUnregisterServer);
        if (unregsrv)
            ret = 3;
    }
    else
        ret = 2;
    FreeLibrary(hModule);
    return ret;
}

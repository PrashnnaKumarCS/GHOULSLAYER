#include <cstdio>

// WHY THIS COMPATIBILITY SHIM EXISTS:
// SFML 2.6.1 bundles pre-compiled static vendor libraries (such as libFLAC.a) in
// extlibs/libs-mingw/x64/. These prebuilt libraries were compiled against the legacy
// Microsoft Visual C Runtime (MSVCRT), which exported the `__iob_func` function to access
// standard I/O stream arrays (stdin, stdout, stderr).
//
// Modern MSYS2 UCRT64 toolchains (GCC 16+ on Windows) use the Universal C Runtime (UCRT),
// which removed `__iob_func` in favor of `__acrt_iob_func`. When linking statically, the
// linker reports: undefined reference to `__imp___iob_func`.
//
// This compatibility shim defines the missing `__iob_func` symbol and its DLL import
// pointer `__imp___iob_func` so libFLAC.a links cleanly with the modern UCRT runtime.

#ifdef __MINGW32__
extern "C" {
    static FILE iobShim[3];

    FILE* __cdecl __iob_func(void) {
        return iobShim;
    }

    FILE* (__cdecl *__imp___iob_func)(void) = &__iob_func;
}
#endif

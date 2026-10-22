#include <gp4/gp4.h>

typedef void* (__stdcall* GetProcessHeap_t)(void);
typedef char* (__stdcall* HeapAlloc_t)(void* heap, uint32_t flags, uint32_t bytes);

// Wrapper around the CRT _fullpath (0x005dc83d). When the caller passes no buffer, the CRT-allocated
// result is copied into a fresh process-heap block (HeapAlloc) and the CRT block is freed (0x005dc114);
// returns the caller's buffer, or that new block.
// FUNCTION: GP4 0x0046aee0
char* __cdecl FUN_0046aee0(char* buffer, const char* path, uint32_t maxLength) {
    char* full = gp4::cdecl_<0x005dc83d, char*>(buffer, path, maxLength);
    if (buffer == nullptr) {
        uint32_t size = 0;
        while (full[size] != 0) {
            size++;
        }
        size++;
        buffer = GP4_IAT(HeapAlloc_t, 0x005f02a8)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, size);
        char* src = full;
        char c;
        do {
            c = *src;
            src[buffer - full] = c;
            src++;
        } while (c != 0);
        gp4::cdecl_<0x005dc114, void>(full);
    }
    return buffer;
}
GP4_IMPL(0x0046aee0, FUN_0046aee0, "cdecl(nptr[0x120]:bytes, ptr[0x20]:u8{0x41,0x61,0x5c,0x2e,0,0x3a}, u32[0..0x120]) -> i32; emu=skip:win32 GetFullPathNameA")

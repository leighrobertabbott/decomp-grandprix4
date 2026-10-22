#include <gp4/gp4.h>

typedef void*(__stdcall* GetProcessHeap_t)();
typedef void*(__stdcall* HeapAlloc_t)(void* heap, uint32_t flags, uint32_t bytes);

// Allocates `bytes` from the process heap (HeapAlloc(GetProcessHeap(), 0, bytes)).
// FUNCTION: GP4 0x00446a40
void* __cdecl FUN_00446a40(uint32_t bytes) {
    return GP4_IAT(HeapAlloc_t, 0x005f02a8)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, bytes);
}
GP4_IMPL(0x00446a40, FUN_00446a40, "cdecl(u32[1..0x2000]) -> i32")

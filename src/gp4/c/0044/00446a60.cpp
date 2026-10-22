#include <gp4/gp4.h>

typedef void*(__stdcall* GetProcessHeap_t)();
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

// Frees `mem` back to the process heap (HeapFree(GetProcessHeap(), 0, mem)).
// FUNCTION: GP4 0x00446a60
int __cdecl FUN_00446a60(void* mem) {
    return GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, mem);
}
GP4_IMPL(0x00446a60, FUN_00446a60, "cdecl(ptr[0x40]:bytes) -> i32")

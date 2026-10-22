#include <gp4/gp4.h>

typedef void*(__stdcall* GetProcessHeap_t)();
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

// Tears down the object in 0x00a03a18 (if any): clears the flag 0x00a05c28, runs the CRT routine
// 0x005dc52c on the object, clears 0x00a03a18 and frees the buffer pointer kept in 0x00a03a20
// from the process heap.
// FUNCTION: GP4 0x0046b580
void __cdecl FUN_0046b580() {
    const uint32_t object = GP4_GLOBAL(uint32_t, 0x00a03a18);
    if (object != 0) {
        GP4_GLOBAL(uint32_t, 0x00a05c28) = 0;
        gp4::cdecl_<0x005dc52c, int>(object);
        void* buffer = GP4_GLOBAL(void*, 0x00a03a20);
        GP4_GLOBAL(uint32_t, 0x00a03a18) = 0;
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, buffer);
    }
}
GP4_IMPL(0x0046b580, FUN_0046b580, "cdecl() -> u32; globals=fuzz:i32+layout=file_0046b580")

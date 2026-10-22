#include <gp4/gp4.h>

typedef void*(__stdcall* GetProcessHeap_t)();
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

// Scalar deleting destructor: runs the destructor body FUN_0044e7d0 on the object and frees the object
// itself from the process heap when bit 0 of flags is set. Returns this.
// FUNCTION: GP4 0x0044e7a0
void* __fastcall FUN_0044e7a0(void* self, void* /*edx*/, uint8_t flags) {
    gp4::thiscall_<0x0044e7d0, void>(self);
    if (flags & 1) {
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, self);
    }
    return self;
}
GP4_IMPL(0x0044e7a0, FUN_0044e7a0, "thiscall(ptr[0x40]:bytes, u32{0,1,2,3}) -> i32")

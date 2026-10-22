#include <gp4/gp4.h>

typedef void*(__stdcall* GetProcessHeap_t)();
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

// Scalar/vector deleting destructor for the 0x34-byte object whose destructor body is 0x0045afe0.
// Bit 1 of `flags` selects the array form (the element count sits in the dword before the array, the
// elements are destroyed through the CRT vector destructor iterator 0x005dbbb0 and the block starts 4
// bytes before `self`); otherwise the single object is destroyed. Bit 0 frees the block from the
// process heap. Returns the start of the block.
// FUNCTION: GP4 0x00455ce0
void* __fastcall FUN_00455ce0(void* self, void* /*edx*/, uint8_t flags) {
    if ((flags & 2) != 0) {
        uint8_t* block = static_cast<uint8_t*>(self) - 4;
        gp4::stdcall_<0x005dbbb0, int>(self, 0x34, *reinterpret_cast<int32_t*>(block), 0x0045afe0);
        if ((flags & 1) != 0) {
            GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, block);
        }
        return block;
    }
    gp4::thiscall_<0x0045afe0, int>(self);
    if ((flags & 1) != 0) {
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, self);
    }
    return self;
}
GP4_IMPL(0x00455ce0, FUN_00455ce0, "thiscall(ptr[0xb0]+4:{0:u32[0..3], 0x1c*3/52:nptr[0x20]:bytes}, u32[0..3]) -> u32")

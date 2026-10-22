#include <gp4/gp4.h>

typedef void*(__stdcall* GetProcessHeap_t)();
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

// Scalar/vector deleting destructor for the 0x8c-byte object whose destructor body is 0x0044e740
// (it only stores the vtable 0x005f04e8). Bit 1 of `flags` selects the array form (the element count
// sits in the dword before the array, the elements are destroyed through the CRT vector destructor
// iterator 0x005dbbb0 and the block starts 4 bytes before `self`); otherwise the single object is
// destroyed. Bit 0 frees the block from the process heap. Returns the start of the block.
// FUNCTION: GP4 0x004565e0
void* __fastcall FUN_004565e0(void* self, void* /*edx*/, uint8_t flags) {
    if ((flags & 2) != 0) {
        uint8_t* block = static_cast<uint8_t*>(self) - 4;
        gp4::stdcall_<0x005dbbb0, int>(self, 0x8c, *reinterpret_cast<int32_t*>(block), 0x0044e740);
        if ((flags & 1) != 0) {
            GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, block);
        }
        return block;
    }
    gp4::thiscall_<0x0044e740, int>(self);
    if ((flags & 1) != 0) {
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, self);
    }
    return self;
}
GP4_IMPL(0x004565e0, FUN_004565e0, "thiscall(ptr[0x1d0]+4:{0:u32[0..3]}, u32[0..3]) -> u32")

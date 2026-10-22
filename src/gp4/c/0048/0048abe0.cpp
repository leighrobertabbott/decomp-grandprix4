#include <gp4/gp4.h>

// Returns the dword at +0xa4 of the block pointed to by the record at self + index * 0x2a0 + 0x178.
// FUNCTION: GP4 0x0048abe0
int __fastcall FUN_0048abe0(uint8_t* self, void* /*edx*/, int index) {
    uint8_t* block = *(uint8_t**)(self + index * 0x2a0 + 0x178);
    return *(int*)(block + 0xa4);
}
GP4_IMPL(0x0048abe0, FUN_0048abe0, "thiscall(ptr[0x980]:{0x178*4/0x2a0:ptr[0xb0]:bytes}, i32[0..3]) -> i32")

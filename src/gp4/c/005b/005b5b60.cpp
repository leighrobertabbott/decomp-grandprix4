#include <gp4/gp4.h>

// Works on the sub-object at +0x18 of the object: frees its singly linked list with 0x005b57a0,
// then releases its line vertex buffer and clears its fields with 0x005b56e0.
// FUNCTION: GP4 0x005b5b60
void __fastcall FUN_005b5b60(void* self, void* /*edx*/) {
    void* sub = (uint8_t*)self + 0x18;
    gp4::thiscall_<0x005b57a0, void>(sub);
    gp4::thiscall_<0x005b56e0, void>(sub);
}
GP4_IMPL(0x005b5b60, FUN_005b5b60, "thiscall(ptr[0x60]:{0x40:nptr[0x50]:{0x4c:nptr[0x50]:{0x4c:nptr[0x50]:{0x4c:nptr[0x50]:{0x4c:u32{=0}}}}}, 0x38:nptr[16]:com=IDirect3DVertexBuffer8}) -> void; globals=fuzz:i32; gfields={0x00b2e808:ptr[16]:com=IDirect3DDevice8}")

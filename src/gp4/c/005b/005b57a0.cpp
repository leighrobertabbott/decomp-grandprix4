#include <gp4/gp4.h>

// Forwarder: `jmp 0x005bcbf0` with ecx passed through. 0x005bcbf0 frees the singly linked
// list headed at +0x28 of the object and clears the head; it is a registered function, so it is
// called through its original address.
// FUNCTION: GP4 0x005b57a0
void __fastcall FUN_005b57a0(void* self, void* /*edx*/) {
    gp4::thiscall_<0x005bcbf0, void>(self);
}
GP4_IMPL(0x005b57a0, FUN_005b57a0, "thiscall(ptr[0x40]:{0x28:nptr[0x50]:{0x4c:nptr[0x50]:{0x4c:nptr[0x50]:{0x4c:nptr[0x50]:{0x4c:u32{=0}}}}}}) -> void")

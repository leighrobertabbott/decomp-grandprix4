#include <gp4/gp4.h>

// Forwarder: `jmp 0x005c3a90` with ecx passed through. 0x005c3a90 clears the dword at +0x10 of
// the object; it is a registered function, so it is called through its original address.
// FUNCTION: GP4 0x005c3a80
void __fastcall FUN_005c3a80(void* self, void* /*edx*/) {
    gp4::thiscall_<0x005c3a90, void>(self);
}
GP4_IMPL(0x005c3a80, FUN_005c3a80, "thiscall(ptr[0x100]:bytes) -> void")

#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_005cad80 { const void* Vptr00; };
#pragma pack(pop)

// Constructor-like routine: stores the vtable pointer 0x005fa374 at +0.
// FUNCTION: GP4 0x005cad80
void __fastcall FUN_005cad80(Object_005cad80* self, void* /*edx*/) {
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005fa374);
}
GP4_IMPL(0x005cad80, FUN_005cad80, "thiscall(ptr[0x10]:bytes) -> void")

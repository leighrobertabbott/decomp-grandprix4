#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_005b4330 { const void* Vptr00; };
#pragma pack(pop)

// Constructor-like routine: stores the vtable pointer 0x005fa068 at +0.
// FUNCTION: GP4 0x005b4330
void __fastcall FUN_005b4330(Object_005b4330* self, void* /*edx*/) {
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005fa068);
}
GP4_IMPL(0x005b4330, FUN_005b4330, "thiscall(ptr[0x10]:bytes) -> void")

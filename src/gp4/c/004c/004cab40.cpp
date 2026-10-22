#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004cab40 { uint32_t Vptr00; };
#pragma pack(pop)

// Stores the vtable pointer 0x005f0cec at the start of the object (constructor stub).
// FUNCTION: GP4 0x004cab40
void __fastcall FUN_004cab40(Unknown_004cab40* self, void* /*edx*/) {
    self->Vptr00 = 0x005f0cec;
}
GP4_IMPL(0x004cab40, FUN_004cab40, "thiscall(ptr[8]:bytes) -> void")

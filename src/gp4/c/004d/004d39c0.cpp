#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d39c0 { uint32_t Vtable00; uint8_t Unknown04[0xfc]; };
#pragma pack(pop)

// Stores the vtable pointer 0x005f0efc at +0 of the object.
// FUNCTION: GP4 0x004d39c0
void __fastcall FUN_004d39c0(Unknown_004d39c0* self, void* /*edx*/) { self->Vtable00 = 0x005f0efc; }
GP4_IMPL(0x004d39c0, FUN_004d39c0, "thiscall(ptr[0x100]:bytes) -> void")

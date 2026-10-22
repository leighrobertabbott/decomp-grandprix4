#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0046bbb0 { const void* Vtable00; uint8_t Unknown04[0xfc]; };
#pragma pack(pop)

// Stores the vtable pointer 0x005f0758 at +0 of the object.
// FUNCTION: GP4 0x0046bbb0
void __fastcall FUN_0046bbb0(Unknown_0046bbb0* self, void* /*edx*/) { self->Vtable00 = GP4_ARRAY(const uint8_t, 0x005f0758); }
GP4_IMPL(0x0046bbb0, FUN_0046bbb0, "thiscall(ptr[0x100]:bytes) -> void")

#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0044e7d0 { const void* Vtable00; uint8_t Unknown04[0xfc]; };
#pragma pack(pop)

// Constructor: stores the vtable 0x005f04ec at +0, then runs the base constructor
// 0x0045ef30 (which overwrites it with the base vtable 0x005f05f4).
// FUNCTION: GP4 0x0044e7d0
void __fastcall FUN_0044e7d0(Unknown_0044e7d0* self, void* /*edx*/) {
    self->Vtable00 = GP4_ARRAY(const uint8_t, 0x005f04ec);
    gp4::thiscall_<0x0045ef30, void>(self);
}
GP4_IMPL(0x0044e7d0, FUN_0044e7d0, "thiscall(ptr[0x100]:bytes) -> void")

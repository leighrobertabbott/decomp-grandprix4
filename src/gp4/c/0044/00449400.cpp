#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00449400 { const void* Vtable00; uint8_t Unknown04[0xfc]; };
#pragma pack(pop)

// Constructor: stores the vtable 0x005f04a0 at +0, then runs the base constructor
// 0x0045ef30 (which overwrites it with the base vtable 0x005f05f4).
// FUNCTION: GP4 0x00449400
void __fastcall FUN_00449400(Unknown_00449400* self, void* /*edx*/) {
    self->Vtable00 = GP4_ARRAY(const uint8_t, 0x005f04a0);
    gp4::thiscall_<0x0045ef30, void>(self);
}
GP4_IMPL(0x00449400, FUN_00449400, "thiscall(ptr[0x100]:bytes) -> void")

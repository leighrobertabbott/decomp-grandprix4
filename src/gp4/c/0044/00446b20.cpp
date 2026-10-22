#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00446b20 { uint32_t Vtable00; uint8_t Unknown04[0xfc]; };
#pragma pack(pop)

// Destructor-chain stub: stores the vtable pointer 0x005f0430 at +0, then tail-jumps to
// 0x0045ef30 (which stores the base vtable 0x005f05f4 at +0).
// FUNCTION: GP4 0x00446b20
void __fastcall FUN_00446b20(Unknown_00446b20* self, void* /*edx*/) {
    self->Vtable00 = 0x005f0430;
    gp4::thiscall_<0x0045ef30, void>(self);
}
GP4_IMPL(0x00446b20, FUN_00446b20, "thiscall(ptr[0x100]:bytes) -> void")

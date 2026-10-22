#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0045ad00 { uint32_t Vtable00; uint8_t Unknown04[0xfc]; };
#pragma pack(pop)

// Derived-class constructor stub: stores the vtable pointer 0x005f05a4, then runs the base-class
// stub 0x0045ef30 (which stores the base vtable pointer 0x005f05f4) as a tail jump.
// FUNCTION: GP4 0x0045ad00
void __fastcall FUN_0045ad00(Unknown_0045ad00* self, void* /*edx*/) {
    self->Vtable00 = 0x005f05a4;
    gp4::thiscall_<0x0045ef30, void>(self);
}
GP4_IMPL(0x0045ad00, FUN_0045ad00, "thiscall(ptr[0x100]:bytes) -> void")

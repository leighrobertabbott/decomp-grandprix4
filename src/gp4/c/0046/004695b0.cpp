#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004695b0 { uint32_t Vtable00; uint8_t Unknown04[0xfc]; };
#pragma pack(pop)

// Derived-class constructor stub: stores the vtable pointer 0x005f0728, then runs the base-class
// stub 0x0045ef30 (which stores the base vtable pointer 0x005f05f4) as a tail jump.
// FUNCTION: GP4 0x004695b0
void __fastcall FUN_004695b0(Unknown_004695b0* self, void* /*edx*/) {
    self->Vtable00 = 0x005f0728;
    gp4::thiscall_<0x0045ef30, void>(self);
}
GP4_IMPL(0x004695b0, FUN_004695b0, "thiscall(ptr[0x100]:bytes) -> void")

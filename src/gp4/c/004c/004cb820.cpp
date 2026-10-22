#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_004cb820 { const void* Vptr00; uint8_t Unknown04[0xfc]; };
#pragma pack(pop)

// Derived-class destructor stub: stores the vtable 0x005f0d78 at +0, then tail-jumps to the
// base routine 0x004cadd0 (which stores the base vtable 0x005f0d3c over it).
// FUNCTION: GP4 0x004cb820
void __fastcall FUN_004cb820(Object_004cb820* self, void* /*edx*/) {
    self->Vptr00 = GP4_ARRAY(const uint8_t, 0x005f0d78);
    gp4::thiscall_<0x004cadd0, void>(self);
}
GP4_IMPL(0x004cb820, FUN_004cb820, "thiscall(ptr[0x100]:bytes) -> void")

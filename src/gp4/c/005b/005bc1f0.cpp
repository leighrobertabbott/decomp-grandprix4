#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Unknown_005bc1f0 { uint8_t Unknown00[0x1c]; float Value1c; uint8_t Unknown20[0xe0]; };
#pragma pack(pop)

// Stores (float)count * (float constant at 0x005fa12c) at +0x1c of the object: `fild [esp+4]; fmul [0x5fa12c];
// fstp [ecx+0x1c]; ret 0xc`. Only the first of the three stack arguments is read.
// FUNCTION: GP4 0x005bc1f0
void __fastcall FUN_005bc1f0(Unknown_005bc1f0* self, void* /*edx*/, int count, int /*arg2*/, int /*arg3*/) {
    using namespace gp4::x87;
    self->Value1c = narrow_f32(ext_i32(count) * ext_f32(GP4_GLOBAL(float, 0x005fa12c)));
}
GP4_IMPL(0x005bc1f0, FUN_005bc1f0, "thiscall(ptr[0x100]:bytes, i32, i32, i32) -> void")

#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Unknown_005bc200 { uint8_t Unknown00[4]; float Value04; uint8_t Unknown08[8]; };
#pragma pack(pop)

// Truncates (float at +4 of the object) * (float constant at 0x005fa130) with the CRT's float-to-int
// routine 0x005daabc (tail jump): the product stays at extended precision on the x87 stack.
// FUNCTION: GP4 0x005bc200
int __fastcall FUN_005bc200(const Unknown_005bc200* self, void* /*edx*/) {
    using namespace gp4::x87;
    const Ext v = ext_f32(self->Value04) * ext_f32(GP4_GLOBAL(float, 0x005fa130));
    gp4::Regs c = {};
    c.st_in = 1;
    set_in_st(c, 0, v);
    gp4::call_regs(0x005daabc, c);
    return (int)c.eax;
}
GP4_IMPL(0x005bc200, FUN_005bc200, "thiscall(ptr[0x10]:f32) -> i32")

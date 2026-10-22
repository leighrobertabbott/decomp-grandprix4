#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Unknown_005d8360 { uint8_t Unknown00[0x70c]; float Value70c; uint8_t Unknown710[8]; };
#pragma pack(pop)

// Truncates (float at +0x70c of the object) * (float constant at 0x005fa130) with the CRT's float-to-int
// routine 0x005daabc (tail jump): the product stays at extended precision on the x87 stack.
// FUNCTION: GP4 0x005d8360
int __fastcall FUN_005d8360(const Unknown_005d8360* self, void* /*edx*/) {
    using namespace gp4::x87;
    const Ext v = ext_f32(self->Value70c) * ext_f32(GP4_GLOBAL(float, 0x005fa130));
    gp4::Regs c = {};
    c.st_in = 1;
    set_in_st(c, 0, v);
    gp4::call_regs(0x005daabc, c);
    return (int)c.eax;
}
GP4_IMPL(0x005d8360, FUN_005d8360, "thiscall(ptr[0x718]:f32) -> i32")

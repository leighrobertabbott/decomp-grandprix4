#include <gp4/gp4.h>
#include <gp4/x87.h>

// Returns the sum of the floats at +0x3c and +0x2c of the object (ecx), left in st0 at
// extended precision: `fld dword [ecx+0x3c]; fadd dword [ecx+0x2c]; ret`.
// FUNCTION: GP4 0x005ba0b0
void __cdecl FUN_005ba0b0(gp4::Regs* r) {
    using namespace gp4::x87;
    const uint8_t* self = (const uint8_t*)r->ecx;
    const float first = *(const float*)(self + 0x3c);
    const float second = *(const float*)(self + 0x2c);
    set_out_st(r, 0, ext_f32(first) + ext_f32(second));
}
GP4_IMPL(0x005ba0b0, FUN_005ba0b0, "regs(ecx:ptr[0x48]:f32) -> (st0)")

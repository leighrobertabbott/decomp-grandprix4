#include <gp4/gp4.h>
#include <gp4/x87.h>

// Returns the product of the floats at +0x1c and +0xc of the object (ecx), left in st0 at
// extended precision: `fld dword [ecx+0x1c]; fmul dword [ecx+0xc]; ret`.
// FUNCTION: GP4 0x005bc210
void __cdecl FUN_005bc210(gp4::Regs* r) {
    using namespace gp4::x87;
    const uint8_t* self = (const uint8_t*)r->ecx;
    const float first = *(const float*)(self + 0x1c);
    const float second = *(const float*)(self + 0xc);
    set_out_st(r, 0, ext_f32(first) * ext_f32(second));
}
GP4_IMPL(0x005bc210, FUN_005bc210, "regs(ecx:ptr[0x28]:f32) -> (st0)")

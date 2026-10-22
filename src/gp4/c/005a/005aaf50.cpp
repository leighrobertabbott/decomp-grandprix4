#include <gp4/gp4.h>
#include <gp4/x87.h>

// Getter: loads the float at +0x64 of the object (ecx) onto the x87 stack (`fld dword [ecx+0x64]`).
// FUNCTION: GP4 0x005aaf50
void __cdecl FUN_005aaf50(gp4::Regs* r) {
    using namespace gp4::x87;
    const float* self = (const float*)r->ecx;
    const float value = *(const float*)((const uint8_t*)self + 0x64);
    set_out_st(r, 0, ext_f32(value));
}
GP4_IMPL(0x005aaf50, FUN_005aaf50, "regs(ecx:ptr[0x70]:f32) -> (st0)")

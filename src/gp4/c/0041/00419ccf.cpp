#include <gp4/gp4.h>
#include <gp4/x87.h>

// Four 0x300-byte records at 0x006380d8 (only the fields this routine touches).
struct Unknown_006380d8 {
    uint8_t Unknown00[0x34];
    int32_t Unknown34;
    uint8_t Unknown38[4];
    int32_t Unknown3C;
    uint8_t Unknown40[0x300 - 0x40];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record stride");

// FUNCTION: GP4 0x00419ccf
// For each of the four records: the complex product (via 0x00401d41, all in extended
// precision on the x87 stack) of
//   (rec.Unknown34 * DAT_00625a18 * DAT_006f2b9c  +  i * rec.Unknown3C)   and
//   (DAT_00623ee0 * DAT_006f29f4  +  i * -(DAT_00623edc * DAT_006f29f4)),
// real part stored (rounded) back to rec.Unknown34, imaginary part to rec.Unknown3C.
// Leaves ebx at the end of the array (0x00638cd8) and the flags of the final cmp (equal).
void __cdecl FUN_00419ccf(gp4::Regs* r) {
    using namespace gp4::x87;
    Unknown_006380d8* const rec = GP4_ARRAY(Unknown_006380d8, 0x006380d8);

    for (int k = 0; k < 4; ++k) {
        Ext a = ext_i32(GP4_GLOBAL(int32_t, 0x00623edc)) * ext(GP4_GLOBAL(double, 0x006f29f4));
        a.b[9] ^= 0x80;   // fchs
        const Ext b = ext_i32(GP4_GLOBAL(int32_t, 0x00623ee0)) * ext(GP4_GLOBAL(double, 0x006f29f4));
        const Ext c = ext_i32(rec[k].Unknown3C);
        const Ext d = ext_i32(rec[k].Unknown34);
        const Ext e = ext_i32(GP4_GLOBAL(int32_t, 0x00625a18));
        const Ext x = (d * e) * ext(GP4_GLOBAL(double, 0x006f2b9c));

        gp4::Regs callee = *r;
        callee.st_xin_mask = 0;
        callee.st_xout_mask = 0;
        callee.st_in = 4;
        callee.st_out = 2;
        set_in_st(callee, 0, x);
        set_in_st(callee, 1, c);
        set_in_st(callee, 2, b);
        set_in_st(callee, 3, a);
        gp4::call_regs(0x00401d41, callee);

        rec[k].Unknown34 = fistp32(out_st(callee, 0));
        rec[k].Unknown3C = fistp32(out_st(callee, 1));
    }

    // cmp ebx, 0x638cd8 with ebx == 0x638cd8: ZF | PF
    r->ebx = 0x00638cd8u;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00419ccf, FUN_00419ccf, "regs() -> (ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

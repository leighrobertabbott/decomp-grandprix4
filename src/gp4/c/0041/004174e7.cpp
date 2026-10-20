#include <gp4/gp4.h>

// FUNCTION: GP4 0x004174e7
// Runs the 3x3 transform 0x00401adc (input vector DAT_00623e18/1c/20, result
// DAT_00623e24/28/2c) on three vectors and stores the results:
//   (DAT_00638d04, DAT_00638d08, DAT_00638d0c) -> DAT_00637fa4/a8/ac
//   (0, DAT_00637f40, 0)                       -> DAT_00637fb0/b4/b8
//   (0, DAT_00637f44, 0)                       -> DAT_00637fbc/c0/c4
// eax, edx, ebp and the flags are left as the last 0x00401adc call leaves them (eax is
// the last result dword).
void __cdecl FUN_004174e7(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;

    GP4_GLOBAL(uint32_t, 0x00623e18) = GP4_GLOBAL(uint32_t, 0x00638d04);
    GP4_GLOBAL(uint32_t, 0x00623e1c) = GP4_GLOBAL(uint32_t, 0x00638d08);
    GP4_GLOBAL(uint32_t, 0x00623e20) = GP4_GLOBAL(uint32_t, 0x00638d0c);
    gp4::call_regs(0x00401adc, call);
    GP4_GLOBAL(uint32_t, 0x00637fa4) = GP4_GLOBAL(uint32_t, 0x00623e24);
    GP4_GLOBAL(uint32_t, 0x00637fa8) = GP4_GLOBAL(uint32_t, 0x00623e28);
    GP4_GLOBAL(uint32_t, 0x00637fac) = GP4_GLOBAL(uint32_t, 0x00623e2c);

    GP4_GLOBAL(uint32_t, 0x00623e18) = 0u;
    GP4_GLOBAL(uint32_t, 0x00623e1c) = GP4_GLOBAL(uint32_t, 0x00637f40);
    GP4_GLOBAL(uint32_t, 0x00623e20) = 0u;
    gp4::call_regs(0x00401adc, call);
    GP4_GLOBAL(uint32_t, 0x00637fb0) = GP4_GLOBAL(uint32_t, 0x00623e24);
    GP4_GLOBAL(uint32_t, 0x00637fb4) = GP4_GLOBAL(uint32_t, 0x00623e28);
    GP4_GLOBAL(uint32_t, 0x00637fb8) = GP4_GLOBAL(uint32_t, 0x00623e2c);

    GP4_GLOBAL(uint32_t, 0x00623e18) = 0u;
    GP4_GLOBAL(uint32_t, 0x00623e1c) = GP4_GLOBAL(uint32_t, 0x00637f44);
    GP4_GLOBAL(uint32_t, 0x00623e20) = 0u;
    gp4::call_regs(0x00401adc, call);
    GP4_GLOBAL(uint32_t, 0x00637fbc) = GP4_GLOBAL(uint32_t, 0x00623e24);
    GP4_GLOBAL(uint32_t, 0x00637fc0) = GP4_GLOBAL(uint32_t, 0x00623e28);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x00623e2c);
    GP4_GLOBAL(uint32_t, 0x00637fc4) = last;

    r->eax = last;
    r->edx = call.edx;
    r->ebp = call.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x004174e7, FUN_004174e7, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

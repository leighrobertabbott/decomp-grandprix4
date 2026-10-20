#include <gp4/gp4.h>
#include <gp4/flags.h>

// Tests the key-state bit `key` through 0x00510e04 (ecx = key; ZF set when the bit is set); the
// callee's ecx/flags stay in *c.
static inline bool KeyDown(gp4::Regs& c, uint32_t key) {
    c.ecx = key;
    gp4::call_regs(0x00510e04, c);
    return (c.eflags & gp4::flags::ZF) != 0u;
}

// Stepper driven by three keys (0x6d, 0x6b, 0x60) with the latch byte [0x009324b0]. Latch set: waits
// until one of the three keys is down (the latch stays) or none is (the latch is cleared). Latch
// clear: key 0x6d lowers and key 0x6b raises the value [0x009324a4] by 0x10, key 0x60 only fires;
// each of them sets the latch to 0x80 and returns the value in eax. ecx is the last key test's
// result and the flags are those of the last flag writer: the key test, or the `sub`/`add` on the
// value (the stores do not touch them).
// FUNCTION: GP4 0x00518491
void __cdecl FUN_00518491(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint32_t arith = 0u;

    if (GP4_GLOBAL(uint8_t, 0x009324b0) != 0u) {
        if (!KeyDown(c, 0x6d) && !KeyDown(c, 0x6b) && !KeyDown(c, 0x60)) {
            GP4_GLOBAL(uint8_t, 0x009324b0) = 0u;
        }
        arith = c.eflags;
    } else {
        bool fire = true;
        if (KeyDown(c, 0x6d)) {
            const uint32_t value = GP4_GLOBAL(uint32_t, 0x009324a4);
            arith = gp4::flags::sub32(value, 0x10u);
            GP4_GLOBAL(uint32_t, 0x009324a4) = value - 0x10u;
        } else if (KeyDown(c, 0x6b)) {
            const uint32_t value = GP4_GLOBAL(uint32_t, 0x009324a4);
            arith = gp4::flags::add32(value, 0x10u);
            GP4_GLOBAL(uint32_t, 0x009324a4) = value + 0x10u;
        } else {
            arith = c.eflags;
            fire = KeyDown(c, 0x60);
            arith = c.eflags;
        }
        if (fire) {
            GP4_GLOBAL(uint8_t, 0x009324b0) = 0x80u;
            c.eax = GP4_GLOBAL(uint32_t, 0x009324a4);
        }
    }

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00518491, FUN_00518491, "regs(eax:u32) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_008245a0")

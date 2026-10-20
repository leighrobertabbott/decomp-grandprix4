#include <gp4/gp4.h>
#include <gp4/flags.h>

// Kept out of line: MSVC otherwise merges the two calls into one call with a selected address, hiding
// the callees from the shape audit.
static __declspec(noinline) void run_00510ec4(gp4::Regs& c) { gp4::call_regs(0x00510ec4, c); }
static __declspec(noinline) void run_00510eda(gp4::Regs& c) { gp4::call_regs(0x00510eda, c); }

// When the byte [0x0066d7a4] has its sign bit set 0x00510ec4 runs; otherwise 0x00510eda runs if the
// dword [0x00679bdc] is zero. Either way the byte [0x00852d94] is then shifted left by two. The flags
// are those of that `shl byte, 2` (CF = bit 6 of the old value, AF = 0, OF = bit 7 of the value
// shifted by one xor the result - the pinned oracle model); the callees keep every register.
// FUNCTION: GP4 0x00524f0f
void __cdecl FUN_00524f0f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x0066d7a4);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(mode));
    if ((mode & 0x80u) != 0u) {
        run_00510ec4(c);
    } else {
        const uint32_t gate = GP4_GLOBAL(uint32_t, 0x00679bdc);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(gate, 0u));
        if (gate == 0u) {
            run_00510eda(c);
        }
    }

    const uint32_t value = GP4_GLOBAL(uint8_t, 0x00852d94);
    const uint32_t shifted = (value << 2) & 0xffu;
    GP4_GLOBAL(uint8_t, 0x00852d94) = static_cast<uint8_t>(shifted);
    uint32_t arith = gp4::flags::logic8(shifted);
    if (((value >> 6) & 1u) != 0u) {
        arith |= gp4::flags::CF;
    }
    if ((((value << 1) ^ shifted) & 0x80u) != 0u) {
        arith |= gp4::flags::OF;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00524f0f, FUN_00524f0f, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040f5f3+order_00418f8f+gate_00524f0f")

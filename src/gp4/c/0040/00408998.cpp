#include <gp4/gp4.h>

// Blends two rows of the 3-entry byte table at esi + 0x199 into place. Window: DAT_00601fc4 - DAT_00601fb4
// (positive) over the lookup of the entry before DAT_00601fb8 (0x00408936) gives a 14-bit fraction
// that is compared with the row bounds DAT_00601fc8[k] / DAT_00601fd8[k] (k = DAT_00601fbc - 1); when
// it falls below the lower bound, or inside the band and the 0x00408964 value of the interpolated
// position is under 0x2000, the row count DAT_00601fc0 becomes DAT_00601fbc + 1 and 0x004088c2 is
// run for the lookup of DAT_00601fb8 - 1; otherwise DAT_00601fc0 = DAT_00601fbc and 0x004088c2 runs
// for DAT_00601fb8. All general registers are restored; eax is the last call's result (the
// argument of 0x004088c2 on the carry-set exit) and the flags are the last callee's with CF = 1 on
// the second exit (stc) and CF = 0 on the first (clc).
// FUNCTION: GP4 0x00408998
void __cdecl FUN_00408998(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    bool raise = false;
    const uint32_t window = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00601fc4)) -
                            static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00601fb4));
    if ((window & 0x80000000u) == 0 && window != 0) {
        c.eax = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00601fb8)) - 1u;
        gp4::call_regs(0x00408936, c);

        int32_t span = static_cast<int32_t>(0u - c.eax + static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00601fc4)));
        if (span < 1) {
            span = 1;
        }
        int32_t part = static_cast<int32_t>(window);
        if (part > span) {
            part = span;
        }
        int32_t fraction = 0x4000 - static_cast<int32_t>(static_cast<uint32_t>(part) << 14) / span;
        if (fraction < 0) {
            fraction = 0;
        }
        if (fraction > 0x4000) {
            fraction = 0x4000;
        }

        const int32_t row = GP4_GLOBAL(int32_t, 0x00601fbc) - 1;
        const int32_t lower = GP4_ARRAY(int32_t, 0x00601fc8)[row];
        const int32_t upper = GP4_ARRAY(int32_t, 0x00601fd8)[row];
        if (fraction < lower) {
            raise = true;
        } else if (fraction < upper) {
            int32_t width = upper - lower;
            if (width == 0) {
                width = 1;
            }
            const uint32_t scaled = static_cast<uint32_t>(fraction - lower) << 14;
            int32_t remainder;
            c.eax = static_cast<uint32_t>(gp4::idiv64(static_cast<int64_t>(scaled), width, &remainder));
            gp4::call_regs(0x00408964, c);
            raise = static_cast<int32_t>(c.eax) < 0x2000;
        }
    }

    if (raise) {
        GP4_GLOBAL(int32_t, 0x00601fc0) = GP4_GLOBAL(int32_t, 0x00601fbc) + 1;
        c.eax = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00601fb8)) - 1u;
        gp4::call_regs(0x00408936, c);
        c.eax = 0u - c.eax + static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00601fb4));
        gp4::call_regs(0x004088c2, c);
        c.eax = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00601fb8));
        gp4::call_regs(0x00408936, c);
        r->eax = c.eax;
        r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d4u);
    } else {
        GP4_GLOBAL(int32_t, 0x00601fc0) = GP4_GLOBAL(int32_t, 0x00601fbc);
        c.eax = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00601fb8));
        gp4::call_regs(0x00408936, c);
        c.eax = 0u - c.eax + static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00601fb4));
        gp4::call_regs(0x004088c2, c);
        r->eax = c.eax;
        r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d4u) | 1u;
    }
}
GP4_IMPL(0x00408998, FUN_00408998, "regs(esi:ptr[0x1d0]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_004088c2+state_0066d8c0+state_00408998")

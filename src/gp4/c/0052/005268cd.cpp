#include <gp4/gp4.h>
#include <gp4/flags.h>

// Nudges one of the two limited values at 0x00852dc4 (index 1 when the byte [0x0066d7a4] is 0xa0, else 0) by
// the step [0x006f2e90] >> 8: it is skipped when the mode dword [0x00645950] is above 2, or equal to 2 with both
// [0x00a53ad0] and [0x00645954] nonzero. Otherwise the keys 0x37 / 0x36 / 0x35 of the wanted bitmap
// (0x00510e04: ZF set = pressed) add the step, subtract the step and reset the value to 0, in that order;
// the result is clamped to the signed range [[0x006f39b4], [0x006f39b0]] (upper limit first) and stored back.
// ebx = the step and ebp = the (new) value are left behind, ecx is the last key probe (body only); edi is
// pushed and popped. The flags are those of the mode compare / test that skipped the body, or of the final
// lower-limit compare.
// FUNCTION: GP4 0x005268cd
void __cdecl FUN_005268cd(gp4::Regs* r) {
    const uint32_t index = (GP4_GLOBAL(uint8_t, 0x0066d7a4) == 0xa0u) ? 1u : 0u;
    r->ebx = GP4_GLOBAL(uint32_t, 0x006f2e90) >> 8;
    r->ebp = GP4_ARRAY(uint32_t, 0x00852dc4)[index];

    const int32_t mode = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00645950));
    uint32_t flags = gp4::flags::sub32(static_cast<uint32_t>(mode), 2u);
    if (mode > 2) {
        r->eflags = gp4::flags::merge(r->eflags, flags);
        return;
    }
    if (mode == 2) {
        const uint32_t first = GP4_GLOBAL(uint32_t, 0x00a53ad0);
        flags = gp4::flags::logic32(first);
        if (first != 0u) {
            const uint32_t second = GP4_GLOBAL(uint32_t, 0x00645954);
            flags = gp4::flags::logic32(second);
            if (second != 0u) {
                r->eflags = gp4::flags::merge(r->eflags, flags);
                return;
            }
        }
    }

    r->ecx = 0x37u;
    gp4::call_regs(0x00510e04, *r);
    if ((r->eflags & gp4::flags::ZF) != 0u) {
        r->ebp += r->ebx;
    }
    r->ecx = 0x36u;
    gp4::call_regs(0x00510e04, *r);
    if ((r->eflags & gp4::flags::ZF) != 0u) {
        r->ebp -= r->ebx;
    }
    r->ecx = 0x35u;
    gp4::call_regs(0x00510e04, *r);
    if ((r->eflags & gp4::flags::ZF) != 0u) {
        r->ebp = 0u;
    }

    const int32_t upper = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006f39b0));
    if (static_cast<int32_t>(r->ebp) > upper) {
        r->ebp = GP4_GLOBAL(uint32_t, 0x006f39b0);
    }
    const int32_t lower = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006f39b4));
    flags = gp4::flags::sub32(r->ebp, static_cast<uint32_t>(lower));
    if (static_cast<int32_t>(r->ebp) < lower) {
        r->ebp = GP4_GLOBAL(uint32_t, 0x006f39b4);
    }
    GP4_ARRAY(uint32_t, 0x00852dc4)[index] = r->ebp;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005268cd, FUN_005268cd, "regs(ecx:u32) -> (ebx, ecx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=gate_0051ae76+keys_008245a0")

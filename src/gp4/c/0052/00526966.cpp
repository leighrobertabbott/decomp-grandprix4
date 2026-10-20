#include <gp4/gp4.h>
#include <gp4/flags.h>

// The two-entry table of dwords at 0x00852dcc (slot 0, or slot 1 when the byte at 0x0066d7a4 is 0xa0) is
// read and stored through helpers so the compiler keeps the table base as one global.
static __declspec(noinline) uint32_t LoadSlot(uint32_t index) {
    return GP4_ARRAY(uint32_t, 0x00852dcc)[index];
}

static __declspec(noinline) void StoreSlot(uint32_t index, uint32_t value) {
    GP4_ARRAY(uint32_t, 0x00852dcc)[index] = value;
}

// Key-driven nudge of a signed setting held in the table at 0x00852dcc: unless the mode dword 0x00645950 is
// above 2 (or exactly 2 with both 0x00a53ad0 and 0x00645954 nonzero), the setting is raised by the step
// ([0x006f2e90] >> 7) while key 0x34 is down in the 0x008245a0 key bitmap (0x00510e04 returns cl = 0 for a
// pressed key), lowered by it while key 0x33 is down, reset to 0 while key 0x35 is down, then clamped to the
// signed range [0x006f39bc, 0x006f39b8] (upper bound first) and stored back. ebx leaves holding the step,
// ebp the (clamped) setting, ecx the last key-test result and the flags those of the final compare (or of the
// gate that skipped the body); edi is saved and restored.
// FUNCTION: GP4 0x00526966
void __cdecl FUN_00526966(gp4::Regs* r) {
    gp4::Regs c = *r;
    const uint32_t slot = GP4_GLOBAL(uint8_t, 0x0066d7a4) == 0xa0u ? 1u : 0u;
    const uint32_t step = GP4_GLOBAL(uint32_t, 0x006f2e90) >> 7;
    int32_t value = static_cast<int32_t>(LoadSlot(slot));
    const int32_t mode = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00645950));
    uint32_t arith = gp4::flags::sub32(static_cast<uint32_t>(mode), 2u);

    bool run = true;
    if (mode > 2) {
        run = false;
    } else if (mode == 2) {
        const uint32_t first = GP4_GLOBAL(uint32_t, 0x00a53ad0);
        arith = gp4::flags::logic32(first);
        if (first != 0u) {
            const uint32_t second = GP4_GLOBAL(uint32_t, 0x00645954);
            arith = gp4::flags::logic32(second);
            if (second != 0u) {
                run = false;
            }
        }
    }

    if (run) {
        c.ecx = 0x34u;
        gp4::call_regs(0x00510e04, c);
        if ((c.eflags & gp4::flags::ZF) != 0u) {
            value = static_cast<int32_t>(static_cast<uint32_t>(value) + step);
        }
        c.ecx = 0x33u;
        gp4::call_regs(0x00510e04, c);
        if ((c.eflags & gp4::flags::ZF) != 0u) {
            value = static_cast<int32_t>(static_cast<uint32_t>(value) - step);
        }
        c.ecx = 0x35u;
        gp4::call_regs(0x00510e04, c);
        if ((c.eflags & gp4::flags::ZF) != 0u) {
            value = 0;
        }
        if (value > static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006f39b8))) {
            value = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006f39b8));
        }
        arith = gp4::flags::sub32(static_cast<uint32_t>(value), GP4_GLOBAL(uint32_t, 0x006f39bc));
        if (value < static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006f39bc))) {
            value = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006f39bc));
        }
        StoreSlot(slot, static_cast<uint32_t>(value));
    }

    r->ebx = step;
    r->ebp = static_cast<uint32_t>(value);
    r->ecx = c.ecx;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00526966, FUN_00526966, "regs(ecx:u32) -> (ebx, ecx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_008245a0+gate_0051ae76")

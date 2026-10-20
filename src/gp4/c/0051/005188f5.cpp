#include <gp4/gp4.h>
#include <gp4/flags.h>

// Hand-written routine. When byte 0x007ad868 is set: raises dword 0x0088fba4 by 0x0088fba8 towards
// 0x4000 (clamped, left alone once it is 0x4000), then walks the set bits k of the mask 0x0088f704.
// For a channel whose dword 0x0088f948[k] is not negative, three ramps move a current value towards
// its target by a signed step (clamped at the target, signed compares) and report value >> 8 in edx
// with eax = k to their handler:
//   step 0x0088f888[k], value 0x0088f708[k], target 0x0088f7c8[k] -> 0x00518c3e
//   step 0x0088f908[k], value 0x0088f788[k], target 0x0088f848[k] -> 0x00518cbe
//   step 0x0088f8c8[k], value 0x0088f748[k], target 0x0088f808[k] -> 0x00518cfe
// A zero step skips its ramp; the mask bit is cleared (btr) when the channel's status is negative
// or none of its steps is nonzero. When byte 0x007ad868 is clear and word 0x00679b4e is zero,
// 0x00509fa0 is called inside pushal/popal. Register effects (byte set): eax = 0, ebx = bit length
// of the mask, ebp = -1 when the last examined channel ran ramp 1 or 2 (else 0), edx = the last
// ramp's value >> 8 or the zero third step.
static uint32_t ramp(int32_t* value, const int32_t* target, int32_t step) {
    int32_t v = step + *value;
    if (step < 0) {
        if (v < *target) {
            v = *target;
        }
    } else {
        if (v > *target) {
            v = *target;
        }
    }
    *value = v;
    return static_cast<uint32_t>(v) >> 8;
}

static void report(const gp4::Regs* r, uint32_t addr, uint32_t channel, uint32_t level, uint32_t ebp) {
    gp4::Regs c = {};
    c.eax = channel;
    c.ebx = channel;
    c.ecx = r->ecx;
    c.edx = level;
    c.esi = r->esi;
    c.edi = r->edi;
    c.ebp = ebp;
    c.eflags = r->eflags;
    gp4::call_regs(addr, c);
}

// FUNCTION: GP4 0x005188f5
void __cdecl FUN_005188f5(gp4::Regs* r) {
    uint32_t flags = 0;
    if (GP4_GLOBAL(uint8_t, 0x007ad868) != 0) {
        if (GP4_GLOBAL(int32_t, 0x0088fba4) != 0x4000) {
            int32_t level = GP4_GLOBAL(int32_t, 0x0088fba4) + GP4_GLOBAL(int32_t, 0x0088fba8);
            if (level > 0x4000) {
                level = 0x4000;
            }
            GP4_GLOBAL(int32_t, 0x0088fba4) = level;
        }

        uint32_t mask = GP4_GLOBAL(uint32_t, 0x0088f704);
        uint32_t k = 0xffffffffu;
        uint32_t edx = r->edx;
        uint32_t ebp = r->ebp;
        for (;;) {
            ++k;
            const uint32_t bit = mask & 1u;
            mask >>= 1;
            if (bit == 0) {
                if (mask == 0) {
                    break;
                }
                continue;
            }
            if (GP4_ARRAY(int32_t, 0x0088f948)[k] >= 0) {
                ebp = 0;
                int32_t step = GP4_ARRAY(int32_t, 0x0088f888)[k];
                edx = static_cast<uint32_t>(step);
                if (step != 0) {
                    edx = ramp(&GP4_ARRAY(int32_t, 0x0088f708)[k], &GP4_ARRAY(int32_t, 0x0088f7c8)[k], step);
                    report(r, 0x00518c3e, k, edx, ebp);
                    ebp = 0xffffffffu;
                }
                step = GP4_ARRAY(int32_t, 0x0088f908)[k];
                edx = static_cast<uint32_t>(step);
                if (step != 0) {
                    edx = ramp(&GP4_ARRAY(int32_t, 0x0088f788)[k], &GP4_ARRAY(int32_t, 0x0088f848)[k], step);
                    report(r, 0x00518cbe, k, edx, ebp);
                    ebp = 0xffffffffu;
                }
                step = GP4_ARRAY(int32_t, 0x0088f8c8)[k];
                edx = static_cast<uint32_t>(step);
                if (step != 0) {
                    edx = ramp(&GP4_ARRAY(int32_t, 0x0088f748)[k], &GP4_ARRAY(int32_t, 0x0088f808)[k], step);
                    report(r, 0x00518cfe, k, edx, ebp);
                    continue;
                }
                if (ebp != 0) {
                    continue;
                }
            }
            GP4_GLOBAL(uint32_t, 0x0088f704) &= ~(1u << (k & 31u));
        }
        r->eax = 0;
        r->ebx = k;
        r->edx = edx;
        r->ebp = ebp;
    }

    const uint8_t enabled = GP4_GLOBAL(uint8_t, 0x007ad868);
    flags = gp4::flags::sub8(enabled, 0u);
    if (enabled == 0) {
        const uint32_t pending = GP4_GLOBAL(uint16_t, 0x00679b4e);
        flags = gp4::flags::sub32(pending, 0u);
        if (pending == 0) {
            gp4::Regs c = {};
            c.eax = pending;
            c.ebx = r->ebx;
            c.ecx = r->ecx;
            c.edx = r->edx;
            c.esi = r->esi;
            c.edi = r->edi;
            c.ebp = r->ebp;
            c.eflags = gp4::flags::merge(r->eflags, flags);
            gp4::call_regs(0x00509fa0, c);
            flags = c.eflags;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005188f5, FUN_005188f5, "regs(eax:u32, ebx:u32, edx:u32, ebp:u32) -> (eax, ebx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=chan_005188f5")

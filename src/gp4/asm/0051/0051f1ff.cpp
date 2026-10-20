#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal ... popal routine over the 100-entry sample table: byte 0x0088fb28[eax] bit 7 marks a sample that is
// playing. The sample -> channel map is 0x0088f998[eax] (channel k, 0..15) and the channel's status word is
// 0x0088f948[k]. When the status still names this sample, the slot predicate 0x00518735 is asked (it answers
// with CF); otherwise the bit is just dropped. When the predicate does not set CF the sample is released:
// bit 7 cleared, bit k cleared in the masks 0x0088f704 and 0x0088f700, byte 0x0088f988[k] = 0.
// Every register is restored; the flags are those of the last flag writer, CF = 0 (`clc`) on every path
// except a predicate answer of CF = 1, which leaves that CF (`stc`).
static inline uint32_t* MaskWord(uint32_t* base, uint32_t bit) {
    return base + (static_cast<int32_t>(bit) >> 5);
}

// FUNCTION: GP4 0x0051f1ff
void __cdecl FUN_0051f1ff(gp4::Regs* r) {
    const uint32_t slot = r->eax;
    uint8_t* state = &GP4_ARRAY(uint8_t, 0x0088fb28)[slot];

    uint32_t flags = gp4::flags::logic8(*state & 0x80u);
    if ((*state & 0x80u) != 0u) {
        const uint32_t k = GP4_ARRAY(uint32_t, 0x0088f998)[slot];
        flags = gp4::flags::sub32(slot, GP4_ARRAY(uint32_t, 0x0088f948)[k]);
        if (slot == GP4_ARRAY(uint32_t, 0x0088f948)[k]) {
            gp4::Regs c = *r;
            c.eax = k;
            c.eflags = gp4::flags::merge(c.eflags, flags);
            gp4::call_regs(0x00518735, c);
            if ((c.eflags & gp4::flags::CF) != 0u) {
                r->eflags = (gp4::flags::merge(r->eflags, c.eflags) | gp4::flags::CF);
                return;
            }
            *state &= 0x7fu;
            flags = gp4::flags::logic8(*state);
            const uint32_t bit = 1u << (k & 31u);
            *MaskWord(&GP4_ARRAY(uint32_t, 0x0088f704)[0], k) &= ~bit;
            *MaskWord(&GP4_ARRAY(uint32_t, 0x0088f700)[0], k) &= ~bit;
            GP4_ARRAY(uint8_t, 0x0088f988)[k] = 0;
        } else {
            *state &= 0x7fu;
            flags = gp4::flags::logic8(*state);
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags) & ~static_cast<uint32_t>(gp4::flags::CF);
}
GP4_IMPL(0x0051f1ff, FUN_0051f1ff, "regs(eax:u32[0..99]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=chan_005188f5+chan_00518654")

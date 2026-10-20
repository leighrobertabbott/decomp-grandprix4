#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a byte test / and / or / cmp-with-zero: CF = OF = AF = 0, ZF SF PF from the byte result.
static inline uint32_t logicFlags8(uint32_t res) {
    res &= 0xffu;
    return parityFlag(res) | (res == 0u ? 0x40u : 0u) | (res & 0x80u);
}

// Flags of a 32-bit `cmp a, b` (a - b): CF PF AF ZF SF OF.
static inline uint32_t cmpFlags32(uint32_t a, uint32_t b) {
    const uint32_t res = a - b;
    uint32_t f = parityFlag(res);
    if (a < b) f |= 0x01u;
    if (((a ^ b ^ res) & 0x10u) != 0u) f |= 0x10u;
    if (res == 0u) f |= 0x40u;
    if ((res & 0x80000000u) != 0u) f |= 0x80u;
    if ((((a ^ b) & (a ^ res)) & 0x80000000u) != 0u) f |= 0x800u;
    return f;
}

// Per-car update of the track-window state (esi = car record, edi = the car's track-segment pointer): byte
// +0x232 bit 0 means "inside the window [0x866ed0 + 0x420, 0x866ed4 - 0x420)". Entering the window sets the bit,
// runs 0x0043464e when the debug byte 0x603c35 is set, and in the first half of the window raises the flag
// 4 of byte +0xfe once; leaving it below the pit start [0x891500] clears the bit and runs 0x0043462a.
// FUNCTION: GP4 0x00434667
void __cdecl FUN_00434667(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
    const uint32_t edi = c.edi;
    const uint32_t state = GP4_FIELD(uint8_t, record, 0x232);
    uint32_t flags = logicFlags8(state & 1u);

    if ((state & 1u) == 0u) {
        const uint32_t low = GP4_GLOBAL(uint32_t, 0x00866ed0) + 0x420u;
        c.eax = low;
        flags = cmpFlags32(edi, low);
        if (edi >= low) {
            const uint32_t high = GP4_GLOBAL(uint32_t, 0x00866ed4) - 0x420u;
            c.eax = high;
            flags = cmpFlags32(edi, high);
            if (edi < high) {
                const uint32_t entered = state | 1u;
                GP4_FIELD(uint8_t, record, 0x232) = static_cast<uint8_t>(entered);
                flags = logicFlags8(entered);
                const uint32_t debug = GP4_GLOBAL(uint8_t, 0x00603c35);
                flags = logicFlags8(debug);
                if (debug != 0u) {
                    c.eflags = (c.eflags & ~0x8d5u) | flags;
                    gp4::call_regs(0x0043464e, c);
                    flags = c.eflags & 0x8d5u;
                }
                const uint32_t start = GP4_GLOBAL(uint32_t, 0x00866ed0);
                const uint32_t middle =
                    static_cast<uint32_t>((static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00866ed4) - start)) >> 1) + start;
                c.eax = middle;
                flags = cmpFlags32(edi, middle);
                if (edi < middle) {
                    const uint32_t cars = GP4_FIELD(uint8_t, record, 0xfe);
                    flags = logicFlags8(cars & 4u);
                    if ((cars & 4u) == 0u) {
                        const uint32_t raised = cars | 4u;
                        GP4_FIELD(uint8_t, record, 0xfe) = static_cast<uint8_t>(raised);
                        flags = logicFlags8(raised);
                        const uint32_t marked = GP4_FIELD(uint8_t, record, 0x71) | 0x80u;
                        GP4_FIELD(uint8_t, record, 0x71) = static_cast<uint8_t>(marked);
                        flags = logicFlags8(marked);
                        const uint32_t cleared = GP4_FIELD(uint8_t, record, 0x230) & 0x7fu;
                        GP4_FIELD(uint8_t, record, 0x230) = static_cast<uint8_t>(cleared);
                        flags = logicFlags8(cleared);
                    }
                }
            }
        }
    } else {
        const uint32_t pitStart = GP4_GLOBAL(uint32_t, 0x00891500);
        flags = cmpFlags32(edi, pitStart);
        if (edi < pitStart) {
            const uint32_t left = state & 0xfeu;
            GP4_FIELD(uint8_t, record, 0x232) = static_cast<uint8_t>(left);
            flags = logicFlags8(left);
            const uint32_t debug = GP4_GLOBAL(uint8_t, 0x00603c35);
            flags = logicFlags8(debug);
            if (debug != 0u) {
                c.eflags = (c.eflags & ~0x8d5u) | flags;
                gp4::call_regs(0x0043462a, c);
                flags = c.eflags & 0x8d5u;
            }
        }
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x00434667, FUN_00434667, "regs(esi:ptr[0x23a]:bytes, edi:rec[0x8943a0,0x160,0..25]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_00405a12+bounds_00406a57")

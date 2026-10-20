#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `test reg, reg` (or `test mem, -1`): CF = OF = AF = 0.
static inline uint32_t testFlags32(uint32_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Word +0x14 of the track-segment record at `record`, sign extended.
static inline int32_t segmentWord(uint32_t record) {
    return static_cast<int16_t>(GP4_FIELD(uint16_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record)), 0x14));
}

// Fills the two words at ebx+4 / ebx+6 with 0x145f30 divided by (a smoothed segment measure plus or
// minus the length 0x43d27c / 0x43d2c4 derive from the current segment [0x603ed0]). The smoothing is the
// average of word +0x14 of four ring records (the current one, the next one and the two before it, stepped
// with the pit / lap wrap rules), turned into 0x200000 / average and clamped to +-0x5000. All registers
// are restored (pushal / popal); the flags are those of the last `test ebp, ebp`.
// FUNCTION: GP4 0x0043dda2
void __cdecl FUN_0043dda2(gp4::Regs* r) {
    const uint32_t start = GP4_GLOBAL(uint32_t, 0x00603ed0);
    uint32_t flags = testFlags32(start);

    if (start != 0u) {
        uint32_t edi = start;
        int32_t sum = 0;
        for (int step = 0; step < 2; ++step) {
            sum += segmentWord(edi);
            if (GP4_GLOBAL(uint8_t, 0x0086857a) == 0u) {
                edi += 0x160u;
                if (edi >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                    if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
                        edi -= GP4_GLOBAL(uint32_t, 0x00866ec4);
                    } else if (edi >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                        edi = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
                    }
                }
            } else {
                edi += 0x160u;
                if (edi >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                    edi -= 0x160u;
                }
            }
        }

        edi = start;
        for (int step = 0; step < 2; ++step) {
            if (GP4_GLOBAL(uint8_t, 0x0086857a) != 0u) {
                edi -= 0x160u;
                if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
                    edi += 0x160u;
                }
            } else if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
                edi -= 0x160u;
                if (edi < GP4_GLOBAL(uint32_t, 0x00891504)) {
                    edi += GP4_GLOBAL(uint32_t, 0x00866ec4);
                }
            } else {
                edi -= 0x160u;
                if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
                    edi = GP4_GLOBAL(uint32_t, 0x00891500);
                }
            }
            sum += segmentWord(edi);
        }

        int32_t average = sum >> 2;
        if (average == 0) {
            average = 1;
        }
        int32_t remainder;
        int32_t quotient = gp4::idiv64(0x200000, average, &remainder);
        if (quotient < 0) {
            uint32_t magnitude = 0u - static_cast<uint32_t>(quotient);
            if (magnitude >= 0x5000u) {
                magnitude = 0x5000u;
            }
            quotient = static_cast<int32_t>(0u - magnitude);
        } else if (static_cast<uint32_t>(quotient) >= 0x5000u) {
            quotient = 0x5000;
        }

        uint8_t* const out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
        const uint16_t cx = static_cast<uint16_t>(quotient);

        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        c.eax = static_cast<uint32_t>(quotient);
        c.ecx = static_cast<uint32_t>(quotient);
        c.edx = static_cast<uint32_t>(remainder);
        c.edi = start;
        gp4::call_regs(0x0043d27c, c);

        int32_t divisor = static_cast<int16_t>(static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x006077d8) + cx));
        flags = testFlags32(static_cast<uint32_t>(divisor));
        if (divisor == 0) {
            divisor = 1;
        }
        int32_t rem1;
        const int32_t first = gp4::idiv64(0x145f30, divisor, &rem1);
        GP4_FIELD(uint16_t, out, 4) = static_cast<uint16_t>(first);

        c.eax = static_cast<uint32_t>(first);
        c.edx = static_cast<uint32_t>(rem1);
        c.ebp = static_cast<uint32_t>(divisor);
        gp4::call_regs(0x0043d2c4, c);

        divisor = static_cast<int16_t>(static_cast<uint16_t>(static_cast<uint16_t>(0u - GP4_GLOBAL(uint16_t, 0x006077da)) + cx));
        flags = testFlags32(static_cast<uint32_t>(divisor));
        if (divisor == 0) {
            divisor = 1;
        }
        int32_t rem2;
        const int32_t second = gp4::idiv64(0x145f30, divisor, &rem2);
        GP4_FIELD(uint16_t, out, 6) = static_cast<uint16_t>(second);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043dda2, FUN_0043dda2, "regs(ebx:ptr[0x10]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+rec_0043d38f+ptr_00603ed0")

#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit `test byte, mask` whose result is `result`: CF = OF = AF = 0.
static inline uint32_t testFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of `or reg, reg` / `test reg, reg` on a 32-bit value: CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

static inline void storeDword(uint32_t address, uint32_t value) {
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(address)) = value;
}

// Registers a car (esi) in the lists of the cars in play: appends it to the table 0x607968, stores its
// lane class (0x0042df3a) in byte +0xc6, appends it to the table 0x6078e4 when bit 4 of +0xce is set
// (and stops there when one of the exclusion bits +0x179 / +0xe5 / +0x163 is set), then files it by its
// lane word +0xea (sign extended into ecx) in the per-lane tables 0x604224 / 0x6041a4 and the
// first / last car slots (0x604194.. 0x6043dc) selected by the flag byte [edi+0x12] of the lane
// record. eax = lane class (or the old 0x6078e0 count), ecx = lane word, edx from 0x0042f7fb; the
// flags are those of the last test / cmp executed.
// FUNCTION: GP4 0x0042f8bf
void __cdecl FUN_0042f8bf(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(esi));
    const uint8_t* const lane = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x0042f7fb, c);

    const uint32_t seen = GP4_GLOBAL(uint32_t, 0x00607964);
    GP4_ARRAY(uint32_t, 0x00607968)[seen] = esi;
    GP4_GLOBAL(uint32_t, 0x00607964) = seen + 1u;

    gp4::call_regs(0x0042df3a, c);
    uint32_t eax = c.eax;
    car[0xc6] = static_cast<uint8_t>(eax);

    uint32_t ecx = r->ecx;
    uint32_t flags;

    do {
        const uint8_t status = car[0xce];
        flags = testFlags8(status & 0x10u);
        if ((status & 0x10u) != 0u) {
            eax = GP4_GLOBAL(uint32_t, 0x006078e0);
            GP4_ARRAY(uint32_t, 0x006078e4)[eax] = esi;
            GP4_GLOBAL(uint32_t, 0x006078e0) = eax + 1u;
            flags = testFlags8(car[0x179] & 0x40u);
            if ((car[0x179] & 0x40u) != 0u) break;
            flags = testFlags8(car[0xe5] & 1u);
            if ((car[0xe5] & 1u) != 0u) break;
            flags = testFlags8(car[0x163] & 0x80u);
            if ((car[0x163] & 0x80u) != 0u) break;
        }

        ecx = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(GP4_FIELD(uint16_t, car, 0xea))));
        storeDword(0x00604224u + ecx, esi);

        const uint8_t side = car[0xfe];
        if ((side & 0x40u) != 0u) {
            if ((side & 0x10u) != 0u) {
                GP4_GLOBAL(uint32_t, 0x00604274) = esi;
            } else {
                GP4_GLOBAL(uint32_t, 0x00604278) = esi;
            }
        }

        flags = cmpFlags32(ecx, 0x18u);
        if (static_cast<int32_t>(ecx) >= 0x18) {
            storeDword(0x006041a4u + ecx, esi);
            break;
        }

        const uint8_t mode = lane[0x12];
        bool both = false;  // reaches the common tail at 0x0042f9cb
        if ((mode & 0x80u) != 0u) {
            flags = testFlags8(mode & 0x40u);
            if ((mode & 0x40u) != 0u) {
                GP4_GLOBAL(uint32_t, 0x00604194) = esi;
                GP4_GLOBAL(uint32_t, 0x0060419c) = esi;
                GP4_GLOBAL(uint32_t, 0x006041ac) = esi;
                GP4_GLOBAL(uint32_t, 0x006041b4) = esi;
                both = true;
            } else {
                flags = logicFlags32(ecx);
                if (static_cast<int32_t>(ecx) > 0) {
                    storeDword(0x006041a4u + ecx, esi);
                    break;
                }
                GP4_GLOBAL(uint32_t, 0x0060419c) = esi;
                GP4_GLOBAL(uint32_t, 0x00604194) = esi;
                both = true;
            }
        } else {
            flags = testFlags8(mode & 0x40u);
            if ((mode & 0x40u) != 0u) {
                flags = logicFlags32(ecx);
                if (static_cast<int32_t>(ecx) < 0) {
                    storeDword(0x006041a4u + ecx, esi);
                    break;
                }
                GP4_GLOBAL(uint32_t, 0x006041ac) = esi;
                GP4_GLOBAL(uint32_t, 0x006041b4) = esi;
                both = true;
            } else {
                flags = logicFlags32(ecx);
                if (ecx != 0u) {
                    if (static_cast<int32_t>(ecx) < 0) {
                        GP4_GLOBAL(uint32_t, 0x0060419c) = esi;
                        GP4_GLOBAL(uint32_t, 0x00604194) = esi;
                    } else {
                        GP4_GLOBAL(uint32_t, 0x006041ac) = esi;
                        GP4_GLOBAL(uint32_t, 0x006041b4) = esi;
                    }
                    break;
                }
                both = true;
            }
        }

        if (both) {
            GP4_GLOBAL(uint32_t, 0x006041a4) = esi;
            flags = testFlags8(side & 0x40u);
            if ((side & 0x40u) == 0u) {
                GP4_GLOBAL(uint32_t, 0x006043dc) = esi;
                GP4_GLOBAL(uint32_t, 0x006043d8) = esi;
            } else {
                flags = testFlags8(side & 0x10u);
                if ((side & 0x10u) == 0u) {
                    GP4_GLOBAL(uint32_t, 0x006043dc) = esi;
                } else {
                    GP4_GLOBAL(uint32_t, 0x006043d8) = esi;
                }
            }
        }
    } while (false);

    r->eax = eax;
    r->ecx = ecx;
    r->edx = c.edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042f8bf, FUN_0042f8bf, "regs(eax:u32, ecx:u32, edx:u32, esi:rec[0x66fef0,0x41c,0..21], edi:ptr[0x1a]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_00403800+pair_0042e8e6+cars_0066fef0+links_00405a12+div_00403bb6+ringw_004291f4+carf_004036ca+carea_0042f6ee+flags_00408f7b+state_0042f8bf")

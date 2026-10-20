#include <gp4/gp4.h>

enum : uint32_t { kArithFlags = 0x8d5u };

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Flags of `test ax, 0xffff` (CF = OF = AF = 0).
static inline uint32_t testFlags16(uint32_t value) {
    value &= 0xffffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x8000u) != 0u ? 0x80u : 0u);
}

static inline uint32_t withLow16(uint32_t reg, uint32_t value) {
    return (reg & 0xffff0000u) | (value & 0xffffu);
}

static inline uint8_t* record(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

static inline uint16_t listWord(uint32_t address) {
    return *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(address));
}

// 16-bit |v| - 0x700 (neg keeps 0x8000).
static inline uint16_t absMinus700(uint16_t value) {
    const uint16_t magnitude = (static_cast<int16_t>(value) < 0) ? static_cast<uint16_t>(0u - value) : value;
    return static_cast<uint16_t>(magnitude - 0x700u);
}

static inline int16_t carWord(uint32_t car, uint32_t offset) {
    return GP4_FIELD(int16_t, record(car), offset);
}

static inline uint8_t carByte(uint32_t car, uint32_t offset) {
    return GP4_FIELD(uint8_t, record(car), offset);
}

enum Step { kNextPartner, kNextLead, kDistanceTest, kSideTest, kContact };

// FUNCTION: GP4 0x0043123e
// Car-pair scan over the order list that ends at [0x6043f8] (word entries: car byte offsets into
// 0x66fef0, a negative word ends the list). First clears bits 7..5 of byte +0x134 of every listed
// car. Then, for each lead car A (walking back from the end, skipping cars with byte +0x7a bit 7),
// it walks the cars B before it (same skip) while the word +0x68 gap B - A stays within [0x603b08]:
// the lateral gap (0x40dc8a when either car has +0x9c bit 1, else B.+0xe2 - A.+0xe2) goes to
// 0x603e70; cars on the same +0x9c bit 4 side are filtered by the gap limits [0x603b02]/[0x603b04],
// the |+0x90| - 0x700 < +0xb0 tests and [0x603b3e]; the +0x8d bit 1 / +0x9c bit 4 combination picks
// skip, the contact test 0x43a964 (-> 0x436f5f on CF) or the side test (0x431165 limit against
// |A.+0x90 - B.+0x90|, or the sign of A.+0x90 ^ B.+0x90); while the gap is within [0x603b06]
// 0x431018 then runs for the pair. Registers end as the original leaves them; the flags are those
// of the final `test ax` on the terminating word.
void __cdecl FUN_0043123e(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    const uint32_t listEnd = GP4_GLOBAL(uint32_t, 0x006043f8);

    for (uint32_t entry = listEnd - 2u;; entry -= 2u) {
        const uint16_t word = listWord(entry);
        s.eax = withLow16(s.eax, word);
        s.ebx = entry;
        if (static_cast<int16_t>(word) < 0) {
            break;
        }
        s.esi = 0x0066fef0u + static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(word)));
        uint8_t* const car = record(s.esi);
        GP4_FIELD(uint8_t, car, 0x134) &= 0x7fu;
        GP4_FIELD(uint8_t, car, 0x134) &= 0xbfu;
        GP4_FIELD(uint8_t, car, 0x134) &= 0xdfu;
    }

    uint32_t lead = listEnd;
    for (;;) {
        lead -= 2u;
        s.ebx = lead;
        const uint16_t leadWord = listWord(lead);
        s.eax = withLow16(s.eax, leadWord);
        if (static_cast<int16_t>(leadWord) < 0) {
            r->eax = s.eax;
            r->ebx = s.ebx;
            r->ecx = s.ecx;
            r->edx = s.edx;
            r->esi = s.esi;
            r->edi = s.edi;
            r->eflags = (r->eflags & ~kArithFlags) | testFlags16(leadWord);
            return;
        }
        s.esi = 0x0066fef0u + static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(leadWord)));
        if ((carByte(s.esi, 0x7a) & 0x80u) != 0u) {
            continue;
        }

        uint32_t partner = lead;
        for (;;) {
            partner -= 2u;
            s.edi = partner;
            const uint16_t partnerWord = listWord(partner);
            s.eax = withLow16(s.eax, partnerWord);
            if (static_cast<int16_t>(partnerWord) < 0) {
                break;
            }
            s.ebx = 0x0066fef0u + static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(partnerWord)));
            if ((carByte(s.ebx, 0x7a) & 0x80u) != 0u) {
                continue;
            }

            const int16_t gap = static_cast<int16_t>(carWord(s.ebx, 0x68) - carWord(s.esi, 0x68));
            s.edx = withLow16(s.edx, static_cast<uint16_t>(gap));
            if (gap > GP4_GLOBAL(int16_t, 0x00603b08)) {
                break;
            }

            if ((carByte(s.esi, 0x9c) & 2u) != 0u || (carByte(s.ebx, 0x9c) & 2u) != 0u) {
                gp4::call_regs(0x0040dc8a, s);
            } else {
                s.eax = withLow16(s.eax, static_cast<uint16_t>(carWord(s.ebx, 0xe2) - carWord(s.esi, 0xe2)));
            }
            GP4_GLOBAL(uint16_t, 0x00603e70) = static_cast<uint16_t>(s.eax);

            const bool leadSide = (carByte(s.esi, 0x9c) & 0x10u) != 0u;
            const bool partnerSide = (carByte(s.ebx, 0x9c) & 0x10u) != 0u;
            if (leadSide == partnerSide) {
                if (gap > GP4_GLOBAL(int16_t, 0x00603b02)) {
                    break;
                }
                uint16_t reach = absMinus700(static_cast<uint16_t>(carWord(s.esi, 0x90)));
                s.ecx = withLow16(s.ecx, reach);
                if (static_cast<int16_t>(reach) < carWord(s.esi, 0xb0)) {
                    reach = absMinus700(static_cast<uint16_t>(carWord(s.ebx, 0x90)));
                    s.ecx = withLow16(s.ecx, reach);
                    if (static_cast<int16_t>(reach) < carWord(s.ebx, 0xb0)) {
                        if (gap > GP4_GLOBAL(int16_t, 0x00603b04)) {
                            break;
                        }
                        if (static_cast<int16_t>(s.eax) > GP4_GLOBAL(int16_t, 0x00603b3e)) {
                            continue;
                        }
                    }
                }
            }

            Step step;
            if ((carByte(s.esi, 0x8d) & 2u) != 0u) {
                if ((carByte(s.ebx, 0x8d) & 2u) != 0u) {
                    step = kSideTest;
                } else {
                    step = ((carByte(s.esi, 0x9c) & 0x10u) != 0u) ? kContact : kNextPartner;
                }
            } else {
                if ((carByte(s.ebx, 0x8d) & 2u) == 0u) {
                    step = kSideTest;
                } else {
                    step = ((carByte(s.ebx, 0x9c) & 0x10u) != 0u) ? kContact : kNextPartner;
                }
            }
            if (step == kNextPartner) {
                continue;
            }

            if (step == kSideTest) {
                bool useLimit = false;
                uint16_t reach = absMinus700(static_cast<uint16_t>(carWord(s.esi, 0x90)));
                s.edx = withLow16(s.edx, reach);
                if (static_cast<int16_t>(reach) < carWord(s.esi, 0xb0)) {
                    reach = absMinus700(static_cast<uint16_t>(carWord(s.ebx, 0x90)));
                    s.edx = withLow16(s.edx, reach);
                    useLimit = static_cast<int16_t>(reach) < carWord(s.ebx, 0xb0);
                }
                if (useLimit) {
                    const uint32_t savedEax = s.eax;
                    gp4::call_regs(0x00431165, s);
                    const int16_t limit = static_cast<int16_t>(s.eax);
                    uint16_t spread = static_cast<uint16_t>(carWord(s.esi, 0x90) - carWord(s.ebx, 0x90));
                    if (static_cast<int16_t>(spread) < 0) {
                        spread = static_cast<uint16_t>(0u - spread);
                    }
                    s.edx = withLow16(s.edx, spread);
                    s.eax = savedEax;
                    step = (static_cast<int16_t>(spread) > limit) ? kDistanceTest : kContact;
                } else {
                    const uint16_t sides = static_cast<uint16_t>(carWord(s.esi, 0x90) ^ carWord(s.ebx, 0x90));
                    s.edx = withLow16(s.edx, sides);
                    step = (static_cast<int16_t>(sides) < 0) ? kDistanceTest : kContact;
                }
            }

            if (step == kContact) {
                gp4::call_regs(0x0043a964, s);
                if ((s.eflags & 1u) != 0u) {
                    gp4::call_regs(0x00436f5f, s);
                }
            }

            const int16_t gapAgain = static_cast<int16_t>(carWord(s.ebx, 0x68) - carWord(s.esi, 0x68));
            s.edx = withLow16(s.edx, static_cast<uint16_t>(gapAgain));
            if (gapAgain > GP4_GLOBAL(int16_t, 0x00603b06)) {
                break;
            }
            gp4::call_regs(0x00431018, s);
        }
    }
}
GP4_IMPL(0x0043123e, FUN_0043123e, "regs(eax:u32, ecx:u32, edx:u32, esi:u32, edi:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+trackgeo_008943a0+tbl_00868598+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+pitw_00409467+ranges_004322a6+state_0040705f+frac_0040705f+state_00407af6+state_004381ab+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+state_0040759e+trackfar_008943a0+sel_004377a4+state_00437888+pair_00437859+carst_0040f6d0+cars_00444d60+state_00418756+list_0066fe3c")

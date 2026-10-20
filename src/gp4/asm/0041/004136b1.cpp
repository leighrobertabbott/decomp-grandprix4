#include <gp4/gp4.h>

struct Unknown_004136b1_Car {
    uint8_t Unknown00[0x0e];
    int16_t Unknown0E;
    uint8_t Unknown10[0x71 - 0x10];
    uint8_t Flags71;
    uint8_t Unknown72[0x7a - 0x72];
    uint8_t Flags7A;
    uint8_t Unknown7B[0x8d - 0x7b];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x90 - 0x8e];
    int16_t Unknown90;
    uint8_t Unknown92[0x94 - 0x92];
    int16_t Unknown94;
    uint8_t Unknown96[0x9a - 0x96];
    int16_t Unknown9A;
    uint8_t Flags9C;
    uint8_t Unknown9D[0xb4 - 0x9d];
    uint16_t UnknownB4;
    uint8_t UnknownB6[0xca - 0xb6];
    uint8_t FlagsCA;
    uint8_t FlagsCB;
    uint8_t UnknownCC[0xe5 - 0xcc];
    uint8_t FlagsE5;
    uint8_t FlagsE6;
    uint8_t UnknownE7;
    uint16_t UnknownE8;
    uint8_t UnknownEA[0xf4 - 0xea];
    uint16_t UnknownF4;
    uint8_t UnknownF6[0xfe - 0xf6];
    uint8_t FlagsFE;
    uint8_t FlagsFF;
    uint8_t Unknown100[0x162 - 0x100];
    uint8_t Flags162;
    uint8_t Flags163;
    uint8_t Unknown164[0x1a0 - 0x164];
    int32_t Unknown1A0;
    uint8_t Unknown1A4[0x1ce - 0x1a4];
    uint8_t Flags1CE;
    uint8_t Unknown1CF[0x20e - 0x1cf];
    uint8_t Unknown20E;
    uint8_t Unknown20F;
    uint8_t Unknown210[0x25f - 0x210];
    uint8_t Flags25F;
};

struct Unknown_004136b1_Seg {
    int16_t Unknown00;
    uint8_t Unknown02[0x10 - 0x02];
    uint8_t Unknown10;
    uint8_t Unknown11[0x48 - 0x11];
    uint8_t Flags48;
    uint8_t Unknown49[0x92 - 0x49];
    uint8_t Flags92;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of test/and/or on a byte result (CF = OF = AF = 0).
static inline uint32_t logic8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of `cmp left, right` on the given width.
static inline uint32_t subFlags(uint32_t left, uint32_t right, uint32_t mask, uint32_t sign) {
    left &= mask;
    right &= mask;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & sign) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & sign) != 0u ? 0x800u : 0u);
}

// Flags of a 16-bit `add`.
static inline uint32_t add16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t sum = left + right;
    const uint32_t result = sum & 0xffffu;
    return (sum > 0xffffu ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((~(left ^ right)) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

static inline void setFlags(gp4::Regs& c, uint32_t flags) {
    c.eflags = (c.eflags & ~0x8d5u) | flags;
}
static inline void setLow16(uint32_t& reg, uint32_t value) {
    reg = (reg & 0xffff0000u) | (value & 0xffffu);
}
static inline void setLow8(uint32_t& reg, uint32_t value) {
    reg = (reg & 0xffffff00u) | (value & 0xffu);
}
static inline uint32_t movsx16(uint32_t value) {
    return static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(value)));
}

// `imul word` followed by n times `shl ax,1; rcl dx,1`: the shifted dx:ax product.
static inline uint32_t mulShift16(uint32_t a, uint32_t b, unsigned shift) {
    const int32_t product = static_cast<int32_t>(static_cast<int16_t>(a)) * static_cast<int32_t>(static_cast<int16_t>(b));
    return static_cast<uint32_t>(product) << shift;
}

// `imul dword factor; shrd eax, edx, 0xe`: eax = product >> 14, edx = high dword.
static inline void mulQ14(gp4::Regs& c, uint32_t factor) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(c.eax)) * static_cast<int64_t>(static_cast<int32_t>(factor));
    c.eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    c.edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
}

// Keeps the sign of value (a sign-extended word) and raises its magnitude to at least floor
// (signed compares).
static inline uint32_t atLeast(uint32_t value, uint32_t floor) {
    if ((value & 0x80000000u) != 0u) {
        uint32_t magnitude = 0u - value;
        if (static_cast<int32_t>(magnitude) < static_cast<int32_t>(floor)) {
            magnitude = floor;
        }
        return 0u - magnitude;
    }
    if (static_cast<int32_t>(value) < static_cast<int32_t>(floor)) {
        return floor;
    }
    return value;
}

// Keeps the sign of value and limits its magnitude to limit (unsigned compares).
static inline uint32_t clampMagnitude(uint32_t value, uint32_t limit) {
    if ((value & 0x80000000u) != 0u) {
        uint32_t magnitude = 0u - value;
        if (magnitude >= limit) {
            magnitude = limit;
        }
        return 0u - magnitude;
    }
    return value >= limit ? limit : value;
}

// Each original routine is called from its own helper so the compiler cannot merge calls to
// different addresses into one call with a selected address.
template <uint32_t A>
static __declspec(noinline) void step(gp4::Regs& c) {
    gp4::call_regs(A, c);
}

static inline bool carry(const gp4::Regs& c) { return (c.eflags & gp4::CF) != 0u; }

// (baseline spec re-run)
// FUNCTION: GP4 0x004136b1
// Per-car output stage on the car record in esi (edi = its track segment): runs the input
// routines, then picks the value stored to word +0xb4 - the routine 0x004165c9, the current
// value, the 0x00413606 result, or the blended value of the main path (0x00412af5/0x00412d5d/
// 0x0040312c/0x00412f9b, Q14 products of the tuning words and dwords at 0x006038xx/0x006039xx,
// clamped by 0x006013ac and finally passed through 0x0041362a when +0xe6 bit 3 is set). Byte
// 0x00625a48 records the path taken; unless 0x00601ba7 was set, bit 5 of +0x1ce is set.
void __cdecl FUN_004136b1(gp4::Regs* r) {
    Unknown_004136b1_Car* car = reinterpret_cast<Unknown_004136b1_Car*>(static_cast<uintptr_t>(r->esi));
    const Unknown_004136b1_Seg* seg = reinterpret_cast<const Unknown_004136b1_Seg*>(static_cast<uintptr_t>(r->edi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(uint8_t, 0x00601ba7) = 0;
    GP4_GLOBAL(uint8_t, 0x00625a48) = 0;
    c.eax = c.edi;
    step<0x00403a1e>(c);
    GP4_GLOBAL(uint8_t, 0x00601ba5) = static_cast<uint8_t>(c.eax);
    step<0x004133e8>(c);
    step<0x004134b6>(c);
    step<0x004134df>(c);
    step<0x00413597>(c);

    if ((car->FlagsE5 & 0x10u) != 0u) {
        setFlags(c, 0u);  // test byte [esi+0xe5], 0x10 (set)
        step<0x004165c9>(c);
        goto store;
    }
    if ((car->FlagsE5 & 4u) != 0u && ((car->FlagsFF & 8u) != 0u || (car->Flags71 & 1u) != 0u)
        && GP4_GLOBAL(uint8_t, 0x00603c34) == 0xffu) {
        setLow16(c.eax, car->UnknownB4);
        goto store;
    }

    GP4_GLOBAL(uint8_t, 0x0063964a) = 0;
    if ((car->Flags7A & 0x20u) != 0u || (car->Flags163 & 8u) != 0u) {
        const uint32_t product = mulShift16(GP4_GLOBAL(uint16_t, 0x00638d20), GP4_GLOBAL(uint16_t, 0x0060391e), 2u);
        setLow16(c.edx, product >> 16);
        setLow16(c.eax, product >> 16);
        goto result;
    }

    GP4_GLOBAL(uint8_t, 0x00639650) = 0;
    GP4_GLOBAL(uint8_t, 0x00639652) = 0;
    {
        bool hold = false;
        bool sides = false;
        if ((car->Flags8D & 2u) != 0u) {
            hold = true;
        } else if ((car->FlagsFE & 4u) != 0u) {
            sides = true;
        } else if ((car->Flags71 & 0x40u) != 0u) {
            setFlags(c, 0u);  // test byte [esi+0x71], 0x40 (set)
            step<0x00413064>(c);
            if (!carry(c)) {
                const uint32_t lanes = seg->Flags48 & 3u;
                setLow8(c.eax, lanes);
                if (lanes != 0u) {
                    hold = true;
                } else {
                    sides = true;
                }
            }
        }
        if (sides) {
            const uint32_t open = seg->Flags92 & 3u;
            setLow8(c.eax, open);
            if (open != 0u) {
                uint16_t offset = static_cast<uint16_t>(car->Unknown0E - seg->Unknown00);
                if ((offset & 0x8000u) != 0u) {
                    offset = static_cast<uint16_t>(0u - offset);
                }
                offset = static_cast<uint16_t>(offset << 1);
                setLow16(c.eax, offset);
                if ((offset & 0x8000u) != 0u) {
                    if ((seg->Flags92 & 2u) != 0u) {
                        GP4_GLOBAL(uint8_t, 0x00639650) = 0x80;
                    }
                    hold = (seg->Flags48 & 2u) != 0u;
                } else {
                    if ((seg->Flags92 & 1u) != 0u) {
                        GP4_GLOBAL(uint8_t, 0x00639650) = 0x80;
                    }
                    hold = (seg->Flags48 & 1u) != 0u;
                }
            }
        }
        if (hold) {
            GP4_GLOBAL(uint8_t, 0x00639652) = 0x80;
            car->Flags71 = static_cast<uint8_t>(car->Flags71 | 0x40u);
        } else {
            car->Flags71 = static_cast<uint8_t>(car->Flags71 & 0xbfu);
        }
        setFlags(c, logic8(car->Flags71));
    }
    step<0x0041368e>(c);

    GP4_GLOBAL(uint8_t, 0x00639646) = 0;
    if ((car->Flags162 & 8u) != 0u) {
        GP4_GLOBAL(uint8_t, 0x00639646) = 0x80;
    }
    if ((car->FlagsCA & 4u) != 0u && (car->FlagsCB & 0x10u) == 0u && (car->Flags9C & 2u) == 0u
        && ((car->Flags25F & 2u) != 0u
            || ((car->Flags1CE & 0x20u) == 0u && (car->Unknown9A < 0x5e || (car->Flags71 & 4u) == 0u)))) {
        setLow16(c.eax, car->UnknownB4);
        GP4_GLOBAL(uint8_t, 0x00601ba7) = 0xff;
        goto store;
    }
    if ((car->FlagsE5 & 4u) == 0u) {
        const uint8_t gate = GP4_GLOBAL(uint8_t, 0x006036d7);
        if (gate != 0xffu) {
            setFlags(c, subFlags(gate, 0xffu, 0xffu, 0x80u));
            step<0x00413606>(c);
            goto store;
        }
        setFlags(c, 0x44u);  // cmp byte [0x6036d7], 0xff (equal)
    } else {
        setFlags(c, 0u);  // test byte [esi+0xe5], 4 (set)
    }
    step<0x00412af5>(c);
    car->Unknown20F = 0;
    car->Unknown20E = 0;
    step<0x00412d5d>(c);
    if ((car->FlagsE5 & 4u) != 0u) {
        setFlags(c, 0u);
        step<0x00413666>(c);
    } else {
        setFlags(c, 0x44u);
    }
    GP4_GLOBAL(uint16_t, 0x0063964c) = static_cast<uint16_t>(c.eax);
    step<0x0040312c>(c);
    GP4_GLOBAL(uint16_t, 0x00639640) = static_cast<uint16_t>(c.eax);
    {
        uint16_t magnitude = GP4_GLOBAL(uint16_t, 0x00639640);
        if ((magnitude & 0x8000u) != 0u) {
            magnitude = static_cast<uint16_t>(0u - magnitude);
        }
        if (static_cast<int16_t>(magnitude) > 0x100) {
            magnitude = 0x100;
        }
        const uint32_t product = static_cast<uint32_t>(magnitude) * GP4_GLOBAL(uint16_t, 0x006038f2);
        setLow16(c.edx, product >> 16);
        const uint32_t base = GP4_GLOBAL(uint16_t, 0x006038f0);
        setFlags(c, add16(product, base));
        const uint16_t sum = static_cast<uint16_t>(product + base);
        setLow16(c.eax, sum);
        GP4_GLOBAL(uint16_t, 0x00639648) = sum;
    }
    step<0x00412f9b>(c);
    GP4_GLOBAL(uint8_t, 0x00625a48) = 1;

    if ((car->FlagsE6 & 8u) == 0u) {
        if ((car->FlagsE5 & 4u) != 0u) {
            if (GP4_GLOBAL(uint8_t, 0x00603c34) != 0xffu && (car->Flags163 & 8u) == 0u) {
                goto steady;
            }
        } else if (GP4_GLOBAL(uint16_t, 0x00638d20) == 0u) {
            goto steady;
        }
    }

    car->UnknownF4 = 0x4000;
    if ((car->FlagsFF & 0x10u) != 0u) {
        GP4_GLOBAL(uint16_t, 0x00639648) = 0;
        setFlags(c, 0u);  // test byte [esi+0xff], 0x10 (set)
        step<0x00412f9b>(c);
        step<0x00413045>(c);
        const uint32_t floor = c.eax;
        c.edx = floor;
        c.eax = atLeast(movsx16(GP4_GLOBAL(uint16_t, 0x00603454)), floor);
        mulQ14(c, GP4_GLOBAL(uint32_t, 0x00603954));
        goto scaled;
    }
    if ((car->Flags71 & 0x40u) == 0u) {
        if (GP4_GLOBAL(uint8_t, 0x00639650) != 0u) {
            c.eax = movsx16(GP4_GLOBAL(uint16_t, 0x00603454));
            mulQ14(c, GP4_GLOBAL(uint32_t, 0x00603950));
            goto scaled;
        }
        if (!((car->FlagsE5 & 4u) != 0u && GP4_GLOBAL(uint16_t, 0x00638d20) != 0u && (car->FlagsCA & 1u) != 0u)) {
            GP4_GLOBAL(uint8_t, 0x00625a48) = 2;
            GP4_GLOBAL(uint32_t, 0x006036d0) = 0x4000;
            setLow8(c.ebx, seg->Unknown10);
            const uint32_t speed = movsx16(static_cast<uint16_t>(car->Unknown90));
            c.ecx = speed;
            c.edx = movsx16(static_cast<uint16_t>(car->Unknown94)) ^ speed;
            bool check = true;
            if ((c.edx & 0x80000000u) == 0u && car->Unknown20E != 0u) {
                c.edx = GP4_GLOBAL(uint32_t, 0x00638d20) ^ GP4_GLOBAL(uint32_t, 0x006036dc) ^ speed;
                check = (c.edx & 0x80000000u) != 0u;
            }
            if (check) {
                c.ebx = (c.ebx & 0x7fu) << 6;
                const uint16_t threshold = GP4_GLOBAL(uint16_t, 0x0060391c);
                if (static_cast<int16_t>(c.ebx) >= static_cast<int16_t>(threshold)) {
                    setFlags(c, subFlags(c.ebx, threshold, 0xffffu, 0x8000u));
                    step<0x00413045>(c);
                    const uint32_t floor = c.eax;
                    c.edx = floor;
                    c.eax = atLeast(movsx16(GP4_GLOBAL(uint16_t, 0x00603454)), floor);
                    GP4_GLOBAL(uint32_t, 0x00603458) = c.eax;
                    mulQ14(c, GP4_GLOBAL(uint32_t, 0x00603958));
                    mulQ14(c, GP4_GLOBAL(uint32_t, 0x006036d0));
                    goto scaled;
                }
            }
            GP4_GLOBAL(uint32_t, 0x006036d0) = 0;
        }
    }

    GP4_GLOBAL(uint8_t, 0x00625a48) = 3;
    if ((car->Flags8D & 2u) == 0u) {
        c.edx = movsx16(GP4_GLOBAL(uint16_t, 0x00639640));
        if (c.edx == 0u) {
            goto damped;
        }
        c.edx ^= GP4_GLOBAL(uint32_t, 0x00638d20);
        c.edx ^= GP4_GLOBAL(uint32_t, 0x006036dc);
        if ((c.edx & 0x80000000u) != 0u) {
            goto damped;
        }
    }
    c.edx = static_cast<uint32_t>(car->Unknown1A0);
    if (static_cast<int32_t>(c.edx) < GP4_GLOBAL(int32_t, 0x00603938)) {
        c.ecx = (car->Flags71 & 8u) != 0u ? 0xffffffffu : 0u;
        c.ecx ^= GP4_GLOBAL(uint32_t, 0x00638d20);
        c.ecx ^= GP4_GLOBAL(uint32_t, 0x006036dc);
        if ((c.ecx & 0x80000000u) != 0u) {
            goto damped;
        }
    }
    GP4_GLOBAL(uint8_t, 0x00625a48) = 4;
    c.eax = 0;
    goto scaled;

damped:
    c.eax = movsx16(GP4_GLOBAL(uint16_t, 0x00603454));
    mulQ14(c, GP4_GLOBAL(uint32_t, 0x0060395c));

scaled:
    if ((car->FlagsFF & 0x10u) == 0u && GP4_GLOBAL(uint8_t, 0x00639650) == 0u && car->Unknown20F != 0u) {
        c.edx = 0x4000u - (static_cast<uint32_t>(car->Unknown20F) << 7);
        mulQ14(c, c.edx);
    }
    {
        const uint32_t saved = c.eax;
        const uint32_t product = mulShift16(GP4_GLOBAL(uint16_t, 0x0063964c), GP4_GLOBAL(uint16_t, 0x00639648), 2u);
        GP4_GLOBAL(uint16_t, 0x00934dec) = static_cast<uint16_t>(product >> 16);
        setLow16(c.eax, product);
        setLow16(c.edx, product >> 16);
        uint16_t gain = 0x4000;
        if ((car->FlagsFF & 0x10u) == 0u) {
            const uint32_t rate = mulShift16(GP4_GLOBAL(uint16_t, 0x006038bc), GP4_GLOBAL(uint16_t, 0x0060384c), 3u);
            setLow16(c.eax, rate);
            setLow16(c.edx, rate >> 16);
            gain = static_cast<uint16_t>((rate >> 16) + car->UnknownE8);
            if (gain >= 0x4000u) {
                gain = 0x4000;
            }
        }
        setLow16(c.ecx, gain);
        car->UnknownE8 = gain;
        c.eax = saved;
        const uint32_t blended = mulShift16(c.eax, gain, 2u);
        setLow16(c.edx, blended >> 16);
        setLow16(c.eax, (blended >> 16) + GP4_GLOBAL(uint16_t, 0x00934dec));
        goto result;
    }

steady:
    car->UnknownE8 = 0;
    setLow16(c.eax, GP4_GLOBAL(uint16_t, 0x0063964c));
    if ((car->Flags162 & 8u) == 0u) {
        const uint32_t product = mulShift16(c.eax, GP4_GLOBAL(uint16_t, 0x00639648), 2u);
        setLow16(c.edx, product >> 16);
        setLow16(c.eax, product >> 16);
        if ((car->FlagsE5 & 4u) == 0u) {
            const uint32_t saved = c.eax;
            setFlags(c, 0x44u);  // test byte [esi+0xe5], 4 (clear)
            step<0x0040312c>(c);
            GP4_GLOBAL(uint16_t, 0x00639640) = static_cast<uint16_t>(c.eax);
            GP4_GLOBAL(uint16_t, 0x00934dfc) = GP4_GLOBAL(uint16_t, 0x00639640);
            c.eax = saved;
            const uint32_t scaledValue = mulShift16(c.eax, car->UnknownF4, 2u);
            setLow16(c.eax, scaledValue >> 16);
            uint16_t slope = GP4_GLOBAL(uint16_t, 0x00934dfc);
            if ((slope & 0x8000u) != 0u) {
                slope = static_cast<uint16_t>(0u - slope);
            }
            slope = static_cast<uint16_t>(0x4000u - static_cast<uint16_t>(slope << 2));
            const uint32_t decay = mulShift16(slope, car->UnknownF4, 2u);
            uint16_t next = static_cast<uint16_t>(decay >> 16);
            const uint16_t minimum = GP4_GLOBAL(uint16_t, 0x00603962);
            if (static_cast<int16_t>(next) < static_cast<int16_t>(minimum)) {
                next = minimum;
            }
            setLow16(c.edx, next);
            car->UnknownF4 = next;
        }
    }

result:
    c.eax = movsx16(c.eax);
    c.edx = GP4_GLOBAL(uint32_t, 0x00638d20);
    if (c.edx != 0u) {
        c.edx ^= c.eax;
        if ((c.edx & 0x80000000u) != 0u && (car->FlagsE5 & 4u) == 0u && (car->Flags162 & 8u) == 0u) {
            c.eax = 0;
        }
    }
    c.ecx = GP4_GLOBAL(uint32_t, 0x006013ac);
    if ((car->FlagsE6 & 8u) != 0u) {
        uint32_t magnitude = GP4_GLOBAL(uint32_t, 0x00638d20);
        if ((magnitude & 0x80000000u) != 0u) {
            magnitude = 0u - magnitude;
        }
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(magnitude)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006013ac));
        c.edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        c.eax = clampMagnitude(c.eax, c.edx);
    }
    c.eax = clampMagnitude(c.eax, c.ecx);
    if ((car->FlagsE6 & 8u) != 0u) {
        setFlags(c, 0u);  // test byte [esi+0xe6], 8 (set)
        step<0x0041362a>(c);
    }

store:
    car->UnknownB4 = static_cast<uint16_t>(c.eax);
    {
        const uint8_t latched = GP4_GLOBAL(uint8_t, 0x00601ba7);
        if (latched == 0u) {
            car->Flags1CE = static_cast<uint8_t>(car->Flags1CE | 0x20u);
            setFlags(c, logic8(car->Flags1CE));
        } else {
            setFlags(c, subFlags(latched, 0u, 0xffu, 0x80u));
        }
    }
    *r = c;
}
GP4_IMPL(0x004136b1, FUN_004136b1, "regs(ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x420]:layout=car_00438aac+car_004129fa+rec_00412b88+car_004136b1, edi:rec[0x8943a0,0x160,0..11]) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00412af5+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+widths_00852c44+state_004127b3+work_00639204+fixed_0041284a+ranges_004322a6+ring_00405a12+links_00405a12+flags_0042aa47+sign_00639652+state_00412b88+state_00431ffb+state_00412c29+words_00413090+engine_00413606+sel_0041b5b6+links_00405a12+state_004136b1")

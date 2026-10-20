#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline int32_t Word(uint32_t address) {
    return *reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(address));
}

// `test ax, 0xffff; jns; neg ax; shr ax, 2; and eax, 0xfffe`: byte offset into the Q14 table.
static inline uint32_t TableOffset(uint32_t angle) {
    uint32_t a = angle & 0xffffu;
    if ((a & 0x8000u) != 0u) {
        a = (0u - a) & 0xffffu;
    }
    return (a >> 2) & 0xfffeu;
}

// `mov ax, [eax + 0x7c5c5c]; movsx eax, ax`: Q14 cosine/sine word (16384 = 1.0).
static inline int32_t Q14Trig(uint32_t offset) {
    return *reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(0x007c5c5cu + offset));
}

static inline int64_t Mul(int32_t a, int32_t b) {
    return static_cast<int64_t>(a) * static_cast<int64_t>(b);
}

// `shrd eax, edx, 0xe`
static inline uint32_t Q14(int64_t product) {
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

static inline uint32_t High(int64_t product) {
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
}

// FUNCTION: GP4 0x00551227
// Point of a (radius, angle) pair in the per-entry word tables 0x870670 (radius) and 0x871070
// (height), indexed by esi (2 * slot) + 0x14 * entry. eax >= 0 uses entry [0x871c9c] (refreshed by
// 0x005511e8 once, flag 0x40 of 0x871c79), eax < 0 entry [0x871c94] (0x005511a9, flag 0x80, radius
// negated). With a zero blend word (0x874b04 / 0x874b00) the angle is word table 0x871b72 / 0x871a70
// plus 0x875e7c; otherwise radius and height are Q14-blended towards entry [0x871ca0] / [0x871c98]
// and the angle is 0x874afe / 0x874afc. Stores x = cos * r + base in 0x934df0 and y = sin * r + base
// in 0x934df4 (bases 0x875e6c/0x875e74 or 0x875e60/0x875e68) and returns ecx = height << 3 +
// ((x ^ y) & [0x875e78]) + [0x875e70] / [0x875e64]; eax = the masked xor; flags of the final add.
void __cdecl FUN_00551227(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esi;
    uint32_t addend;
    int64_t product;

    if (static_cast<int32_t>(r->eax) >= 0) {
        if ((GP4_GLOBAL(uint8_t, 0x00871c79) & 0x40u) == 0u) {
            gp4::call_regs(0x005511e8, s);
            GP4_GLOBAL(uint8_t, 0x00871c79) |= 0x40u;
        }
        ebx = s.ebx;
        if (GP4_GLOBAL(uint16_t, 0x00874b04) == 0u) {
            const uint32_t entry = GP4_GLOBAL(uint32_t, 0x00871c9c);
            esi = s.esi + entry * 0x14u;
            const int32_t radius = static_cast<int32_t>(static_cast<uint32_t>(Word(esi + 0x00870670u)) << 6);
            const uint32_t angle = GP4_ARRAY(uint16_t, 0x00871b72)[entry] + GP4_GLOBAL(uint16_t, 0x00875e7c);

            product = Mul(Q14Trig(TableOffset(0x4000u - angle)), radius);
            GP4_GLOBAL(uint32_t, 0x00934df0) = Q14(product) + GP4_GLOBAL(uint32_t, 0x00875e6c);
            product = Mul(Q14Trig(TableOffset(angle)), radius);
            GP4_GLOBAL(uint32_t, 0x00934df4) = Q14(product) + GP4_GLOBAL(uint32_t, 0x00875e74);
            edx = High(product);
            ecx = static_cast<uint32_t>(Word(esi + 0x00871070u)) << 3;
        } else {
            ebx = s.esi + GP4_GLOBAL(uint32_t, 0x00871ca0) * 0x14u;
            esi = s.esi + GP4_GLOBAL(uint32_t, 0x00871c9c) * 0x14u;
            const int32_t blend = GP4_GLOBAL(int32_t, 0x00874b04);

            product = Mul(static_cast<int16_t>(Word(ebx + 0x00870670u) - Word(esi + 0x00870670u)), blend);
            const int32_t radius = static_cast<int32_t>((Q14(product) + static_cast<uint32_t>(Word(esi + 0x00870670u))) << 6);

            product = Mul(Q14Trig(TableOffset(0x4000u - GP4_GLOBAL(uint16_t, 0x00874afe))), radius);
            GP4_GLOBAL(uint32_t, 0x00934df0) = Q14(product) + GP4_GLOBAL(uint32_t, 0x00875e6c);
            product = Mul(Q14Trig(TableOffset(GP4_GLOBAL(uint16_t, 0x00874afe))), radius);
            GP4_GLOBAL(uint32_t, 0x00934df4) = Q14(product) + GP4_GLOBAL(uint32_t, 0x00875e74);

            product = Mul(static_cast<int16_t>(Word(ebx + 0x00871070u) - Word(esi + 0x00871070u)), blend);
            edx = static_cast<uint32_t>(Word(esi + 0x00871070u));
            ecx = (Q14(product) + edx) << 3;
        }
        addend = GP4_GLOBAL(uint32_t, 0x00875e70);
    } else {
        if ((GP4_GLOBAL(uint8_t, 0x00871c79) & 0x80u) == 0u) {
            gp4::call_regs(0x005511a9, s);
            GP4_GLOBAL(uint8_t, 0x00871c79) |= 0x80u;
        }
        ebx = s.ebx;
        if (GP4_GLOBAL(uint16_t, 0x00874b00) == 0u) {
            const uint32_t entry = GP4_GLOBAL(uint32_t, 0x00871c94);
            esi = s.esi + entry * 0x14u;
            const int32_t radius = static_cast<int32_t>((0u - static_cast<uint32_t>(Word(esi + 0x00870670u))) << 6);
            const uint32_t angle = GP4_ARRAY(uint16_t, 0x00871a70)[entry] + GP4_GLOBAL(uint16_t, 0x00875e7c);

            product = Mul(Q14Trig(TableOffset(0x4000u - angle)), radius);
            GP4_GLOBAL(uint32_t, 0x00934df0) = Q14(product) + GP4_GLOBAL(uint32_t, 0x00875e60);
            product = Mul(Q14Trig(TableOffset(angle)), radius);
            GP4_GLOBAL(uint32_t, 0x00934df4) = Q14(product) + GP4_GLOBAL(uint32_t, 0x00875e68);
            edx = High(product);
            ecx = static_cast<uint32_t>(Word(esi + 0x00871070u)) << 3;
        } else {
            ebx = s.esi + GP4_GLOBAL(uint32_t, 0x00871c98) * 0x14u;
            esi = s.esi + GP4_GLOBAL(uint32_t, 0x00871c94) * 0x14u;
            const int32_t blend = GP4_GLOBAL(int32_t, 0x00874b00);

            product = Mul(static_cast<int16_t>(Word(ebx + 0x00870670u) - Word(esi + 0x00870670u)), blend);
            const int32_t radius = static_cast<int32_t>(
                (0u - (Q14(product) + static_cast<uint32_t>(Word(esi + 0x00870670u)))) << 6);

            product = Mul(Q14Trig(TableOffset(0x4000u - GP4_GLOBAL(uint16_t, 0x00874afc))), radius);
            GP4_GLOBAL(uint32_t, 0x00934df0) = Q14(product) + GP4_GLOBAL(uint32_t, 0x00875e60);
            product = Mul(Q14Trig(TableOffset(GP4_GLOBAL(uint16_t, 0x00874afc))), radius);
            GP4_GLOBAL(uint32_t, 0x00934df4) = Q14(product) + GP4_GLOBAL(uint32_t, 0x00875e68);

            product = Mul(static_cast<int16_t>(Word(ebx + 0x00871070u) - Word(esi + 0x00871070u)), blend);
            edx = static_cast<uint32_t>(Word(esi + 0x00871070u));
            ecx = (Q14(product) + edx) << 3;
        }
        addend = GP4_GLOBAL(uint32_t, 0x00875e64);
    }

    eax = (GP4_GLOBAL(uint32_t, 0x00934df4) ^ GP4_GLOBAL(uint32_t, 0x00934df0)) & GP4_GLOBAL(uint32_t, 0x00875e78);
    ecx += eax;
    const uint32_t arith = gp4::flags::add32(ecx, addend);
    ecx += addend;

    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    r->ebx = ebx;
    r->esi = esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00551227, FUN_00551227, "regs(eax:i32, esi:u32{=0,2,4,6,8,10,12,14,16,18}, edi:rec[0x8943a0,0x160,0..7]) -> (eax, ecx, edx, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=geom_008943a0+span_00551227")

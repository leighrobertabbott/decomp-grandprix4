#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00414595
void __cdecl FUN_00414595(gp4::Regs* r) {
    uint8_t* other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const int16_t* costab = GP4_ARRAY(int16_t, 0x007c5c5c);

    const uint16_t otherAngle = GP4_FIELD(uint16_t, other, 0x0e);
    const uint16_t selfAngle = GP4_FIELD(uint16_t, self, 0x00);

    // 16-bit angle difference folded to |diff| in 0..0x7fff
    uint16_t ang = static_cast<uint16_t>(otherAngle - selfAngle);
    if (static_cast<int16_t>(ang) < 0) ang = static_cast<uint16_t>(-ang);
    if (ang > 0x4000u) ang = static_cast<uint16_t>(0x8000u - ang);
    if (static_cast<int16_t>(ang) < 0) ang = static_cast<uint16_t>(-ang);

    const uint32_t index = (static_cast<uint32_t>(ang) >> 2) & 0xfffeu;
    const int32_t factor = 0x4000 - static_cast<int32_t>(*reinterpret_cast<const int16_t*>(
                                       reinterpret_cast<const uint8_t*>(costab) + index));
    const int32_t scale = GP4_GLOBAL(int32_t, 0x00601ad0);
    const int64_t product = static_cast<int64_t>(factor) * scale;
    const uint32_t limit = static_cast<uint32_t>(product >> 14) + 0x80u;   // ebp
    const uint32_t productHi = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);

    const uint16_t diff = static_cast<uint16_t>(otherAngle - selfAngle);
    const int32_t lateral = static_cast<int16_t>(GP4_FIELD(uint16_t, other, 0x90));

    uint32_t a;      // left operand of the final cmp
    uint32_t ecx;
    uint32_t eax;
    if ((diff & 0x8000u) == 0) {
        uint16_t c = static_cast<uint16_t>(GP4_FIELD(uint16_t, self, 0x76)
                                           + GP4_FIELD(uint16_t, self, 0x62) + 0x50u);
        ecx = static_cast<uint32_t>(c) - static_cast<uint32_t>(lateral);
        a = ecx;
        eax = static_cast<uint32_t>(lateral);
    } else {
        uint16_t c = static_cast<uint16_t>(GP4_FIELD(uint16_t, self, 0x74)
                                           + GP4_FIELD(uint16_t, self, 0x60) + 0x50u);
        ecx = 0u - static_cast<uint32_t>(c);
        eax = static_cast<uint32_t>(lateral) - ecx;
        a = eax;
    }

    // cmp a, limit; jl -> stc else clc (CF rewritten, other flags from the cmp)
    const uint32_t b = limit;
    const uint32_t res = a - b;
    const uint32_t sf = (res >> 31) & 1u;
    const uint32_t of = (((a ^ b) & (a ^ res)) >> 31) & 1u;
    const uint32_t flags = parityFlag(res)
        | (((a ^ b ^ res) & 0x10u) != 0u ? 0x10u : 0u)
        | (res == 0u ? 0x40u : 0u)
        | (sf ? 0x80u : 0u)
        | (of ? 0x800u : 0u)
        | (sf != of ? 1u : 0u);

    r->eax = eax;
    r->ecx = ecx;
    r->edx = (productHi & 0xffff0000u) | diff;
    r->ebp = limit;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00414595, FUN_00414595, "regs(edi:ptr[0x7e]:bytes, esi:ptr[0x98]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Result of `imul` (32x32 -> edx:eax, signed) followed by `shrd eax, edx, 14`.
struct MulShrd14 {
    uint32_t value;   // eax after the shrd
    uint32_t high;    // edx (high dword of the product)
    uint32_t flags;   // flags of the shrd in the pinned oracle model
};

static inline MulShrd14 mulShrd14(int32_t left, int32_t right) {
    const int64_t product = static_cast<int64_t>(left) * static_cast<int64_t>(right);
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    const uint32_t value = (low >> 14) | (high << 18);
    const uint32_t previous = (low >> 13) | (high << 19);
    MulShrd14 out;
    out.value = value;
    out.high = high;
    // CF = last bit out, AF = 0, OF = bit31(shift by 13 ^ shift by 14).
    out.flags = ((low >> 13) & 1u)
        | parityFlag8(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ value) & 0x80000000u) != 0u ? 0x800u : 0u);
    return out;
}

// Per-car tyre/load scaling for the four 0x300-byte slots of 0x006380d8. With [0x00622424] clear each slot k
// gets, from byte k of the packed dword +0x240, the grip level min(byte >> 4, 11) at +0x10, 0x5d1 times that
// (capped at 0x4000) at +0xd4, and the force at +0x1d8: the car's dword +0x188 + 4k scaled by
// [0x00602a9c] (Q14), run through the 0x44192c force curve, then by 0x4000 or, when bit 6 of the byte at
// edi+0x64 is set, [0x006020ac] (Q14), and by the word +0x10e when bit 0 of the byte +0x8d is set. With the
// flag set all four slots just get the word +0xa2 (times the word +0x10e >> 14 under the same bit).
// ebx ends one past the slots and ecx = 4 (flag clear), ebp = 0.
// FUNCTION: GP4 0x00417b84
void __cdecl FUN_00417b84(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* wheel = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    if (GP4_GLOBAL(uint8_t, 0x00622424) != 0u) {
        uint32_t value = GP4_FIELD(uint16_t, car, 0xa2);
        uint32_t flags = 0x44u;
        if ((GP4_FIELD(uint8_t, car, 0x8d) & 1u) != 0u) {
            const MulShrd14 scaled = mulShrd14(static_cast<int32_t>(value), static_cast<int32_t>(GP4_FIELD(uint16_t, car, 0x10e)));
            value = scaled.value;
            flags = scaled.flags;
            r->edx = scaled.high;
        }
        uint8_t* slots = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x006380d8u));
        GP4_FIELD(uint32_t, slots, 0x1d8) = value;
        GP4_FIELD(uint32_t, slots, 0x4d8) = value;
        GP4_FIELD(uint32_t, slots, 0x7d8) = value;
        GP4_FIELD(uint32_t, slots, 0xad8) = value;
        r->eax = value;
        r->ebx = 0x006380d8u;
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    uint32_t packed = GP4_FIELD(uint32_t, car, 0x240);
    uint32_t value = r->eax;
    uint32_t high = r->edx;
    for (uint32_t slotIndex = 0; slotIndex != 4u; ++slotIndex) {
        uint8_t* slot = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x006380d8u + slotIndex * 0x300u));

        uint32_t level = (packed & 0xffu) >> 4;
        if (level > 0xbu) {
            level = 0xbu;
        }
        GP4_FIELD(uint32_t, slot, 0x10) = level;
        uint32_t cap = level * 0x5d1u;
        if (static_cast<int32_t>(cap) > 0x4000) {
            cap = 0x4000u;
        }
        GP4_FIELD(uint32_t, slot, 0xd4) = cap;

        const MulShrd14 base = mulShrd14(GP4_FIELD(int32_t, car, 0x188 + slotIndex * 4u), GP4_GLOBAL(int32_t, 0x00602a9c));

        gp4::Regs curve = *r;
        curve.st_in = 0;
        curve.st_out = 0;
        curve.eax = base.value;
        curve.ecx = slotIndex;
        curve.ebx = 0x006380d8u + slotIndex * 0x300u;
        gp4::call_regs(0x0044192c, curve);

        int32_t factor = 0x4000;
        if ((GP4_FIELD(uint8_t, wheel, 0x64) & 0x40u) != 0u) {
            factor = GP4_GLOBAL(int32_t, 0x006020ac);
        }
        MulShrd14 scaled = mulShrd14(static_cast<int32_t>(curve.eax), factor);
        if ((GP4_FIELD(uint8_t, car, 0x8d) & 1u) != 0u) {
            scaled = mulShrd14(static_cast<int32_t>(scaled.value), static_cast<int32_t>(GP4_FIELD(uint16_t, car, 0x10e)));
        }
        GP4_FIELD(uint32_t, slot, 0x1d8) = scaled.value;
        value = scaled.value;
        high = scaled.high;
        packed >>= 8;
    }

    r->eax = value;
    r->ebx = 0x00638cd8u;
    r->ecx = 4u;
    r->edx = high;
    r->ebp = packed;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00417b84, FUN_00417b84, "regs(ecx:u32, edx:u32, ebp:u32, esi:ptr[0x41c]:layout=rec_0043259c+rec_004414a8, edi:ptr[0x70]:bytes) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043c1ba+state_004320f2+state_00416fce")

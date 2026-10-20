#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Flags of a byte logic op (`and` / `or`): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit `cmp left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

struct Unknown_004142bb_Car {
    uint8_t Unknown00[0x62];
    uint16_t Unknown62;
    uint8_t Unknown64[0x6f - 0x64];
    uint8_t Flags6F;
    uint8_t Unknown70[0x90 - 0x70];
    uint16_t Unknown90;
    uint8_t Unknown92[0xa2 - 0x92];
    uint16_t UnknownA2;
    uint8_t UnknownA4[0xac - 0xa4];
    int32_t UnknownAC;
    uint16_t UnknownB0;
    uint8_t UnknownB2[0xe5 - 0xb2];
    uint8_t FlagsE5;
    uint8_t UnknownE6[0xff - 0xe6];
    uint8_t FlagsFF;
    uint8_t Unknown100[0x1f6 - 0x100];
    uint8_t Unknown1F6;
    uint8_t Unknown1F7[0x203 - 0x1f7];
    uint8_t Flags203;
    uint8_t Unknown204[0x230 - 0x204];
    uint8_t Flags230;
};

// FUNCTION: GP4 0x004142bb
// Per-car speed-window check. Bit 1 of +0x230 is set (and +0x6f bit 4) while word +0x62 is below
// [0x006224e0] (or [0x006224e4] once set), else cleared. When |word +0x90| - 0x200 >= word +0xb0,
// 0x00413ea0 (ebp = 8) looks for a qualifying car: found sets +0x203 bit 1 (+0x1f6 = 5 the first
// time), makes +0x6f = (+0x6f & 0xfe) | 2 and returns CF = 0. Otherwise (and when the window is
// missed) 0x00414555 decides (CF set: +0xff |= 8, failure); else the value
// (word [0x00934df4], mirrored about 0x8000 when word [0x00934df8] < 0) minus
// (([0x006224d8] - (word +0xa2 * [0x006224dc] >> 14)) * |dword +0xac| >> 22), stored in
// [0x00622508], must be >= [0x006224ec] for success (CF = 0). Failure clears +0xe5 bit 4, CF = 1.
// Registers: eax = the last value computed, edx = the high dword of the last product, ebp = 8
// once 0x00413ea0 was reached. The flags are those of the last byte op or of the final compare.
void __cdecl FUN_004142bb(gp4::Regs* r) {
    Unknown_004142bb_Car* car = reinterpret_cast<Unknown_004142bb_Car*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;
    uint32_t flags;
    uint32_t carry;

    const uint32_t speedWord = car->Unknown62;
    eax = speedWord;
    const int32_t threshold = ((car->Flags230 & 2u) == 0u)
        ? GP4_GLOBAL(int32_t, 0x006224e0)
        : GP4_GLOBAL(int32_t, 0x006224e4);
    if (static_cast<int32_t>(speedWord) >= threshold) {
        car->Flags230 = static_cast<uint8_t>(car->Flags230 & 0xfdu);
    } else {
        car->Flags6F = static_cast<uint8_t>(car->Flags6F | 0x10u);
        car->Flags230 = static_cast<uint8_t>(car->Flags230 | 2u);
    }

    uint16_t window = car->Unknown90;
    if (static_cast<int16_t>(window) < 0) {
        window = static_cast<uint16_t>(0u - window);
    }
    window = static_cast<uint16_t>(window - 0x200u);
    eax = (eax & 0xffff0000u) | window;

    if (static_cast<int16_t>(window) >= static_cast<int16_t>(car->UnknownB0)) {
        ebp = 8;
        gp4::Regs search = *r;
        search.eax = eax;
        search.edx = edx;
        search.ebp = ebp;
        gp4::call_regs(0x00413ea0, search);
        if ((search.eflags & 1u) == 0u) {
            if ((car->Flags203 & 2u) == 0u) {
                car->Flags203 = static_cast<uint8_t>(car->Flags203 | 2u);
                car->Unknown1F6 = 5;
            }
            car->Flags6F = static_cast<uint8_t>(car->Flags6F & 0xfeu);
            car->Flags6F = static_cast<uint8_t>(car->Flags6F | 2u);
            flags = logicFlags8(car->Flags6F);
            carry = 0;
            goto done;
        }
        car->Flags203 = static_cast<uint8_t>(car->Flags203 & 0xfdu);
    }

    {
        gp4::Regs check = *r;
        check.eax = eax;
        check.edx = edx;
        check.ebp = ebp;
        gp4::call_regs(0x00414555, check);
        if ((check.eflags & 1u) != 0u) {
            car->FlagsFF = static_cast<uint8_t>(car->FlagsFF | 8u);
            goto fail;
        }
    }

    {
        const int64_t first = static_cast<int64_t>(static_cast<int32_t>(car->UnknownA2))
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006224dc));
        eax = 0u - static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14);
        eax += GP4_GLOBAL(uint32_t, 0x006224d8);
        edx = static_cast<uint32_t>(car->UnknownAC);
        if (static_cast<int32_t>(edx) < 0) {
            edx = 0u - edx;
        }
        const int64_t second = static_cast<int64_t>(static_cast<int32_t>(eax))
            * static_cast<int64_t>(static_cast<int32_t>(edx));
        edx = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);
        eax = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 22);
        GP4_GLOBAL(uint32_t, 0x00622508) = eax;

        const uint32_t stored = eax;
        eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00934df4)));
        if (GP4_GLOBAL(int16_t, 0x00934df8) < 0) {
            eax = 0u - eax;
            eax += 0x8000u;
        }
        eax -= stored;
        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006224ec);
        flags = subFlags32(eax, limit);
        if (static_cast<int32_t>(eax) >= static_cast<int32_t>(limit)) {
            flags &= ~1u;
            carry = 0;
            goto done;
        }
    }

fail:
    car->FlagsE5 = static_cast<uint8_t>(car->FlagsE5 & 0xefu);
    flags = logicFlags8(car->FlagsE5);
    carry = 1;

done:
    r->eax = eax;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u) | carry;
}
GP4_IMPL(0x004142bb, FUN_004142bb, "regs(edx:u32, ebp:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_0066fe3c+state_00413f28+state_0041104f")

#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 16-bit CMP left, right.
static inline uint32_t cmpFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 0x001u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x8000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Steps one 0xe0-byte animated record (esi): when its sprite table pointer (+0x90) is not -1 and flag
// bit 4 (sequential) or bit 8 (random pick) of +0x98 is set, advances the 1-based frame index +0x94 within
// the table (count dword, then one dword per frame whose low word goes to +0x58); then integrates the
// dword +0x60 (floor 0x400) and the word +0x68 (floor +0x5a) by their rates +0x64 / +0x6a scaled by
// [0x603854] >> 16. Leaves ecx = the table pointer, edx = high half of the last product, eax = the last word
// sum, and the flags of the final 16-bit compare.
// FUNCTION: GP4 0x00444008
void __cdecl FUN_00444008(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t table = GP4_FIELD(uint32_t, rec, 0x90);
    r->ecx = table;
    if (table == 0xffffffffu) {
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;   // cmp ecx, -1 -> zero
        return;
    }

    const uint32_t options = GP4_FIELD(uint32_t, rec, 0x98);
    const uint32_t* header = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(table));
    bool pick = false;
    uint32_t frame = 0u;
    if ((options & 4u) != 0u) {
        frame = GP4_FIELD(uint32_t, rec, 0x94) + 1u;
        pick = true;
    } else if ((options & 8u) != 0u) {
        gp4::Regs sub = *r;
        sub.eax = header[0];
        gp4::call_regs(0x0040150b, sub);
        frame = sub.eax;
        pick = true;
    }
    if (pick) {
        if (static_cast<int32_t>(frame) < 1) {
            frame = 1u;
        }
        if (frame > header[0]) {
            frame = 1u;
        }
        GP4_FIELD(uint32_t, rec, 0x94) = frame;
        GP4_FIELD(uint16_t, rec, 0x58) = *reinterpret_cast<const uint16_t*>(
            static_cast<uintptr_t>(table) + static_cast<uintptr_t>(frame) * 4u);
    }

    const int32_t rate = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00603854));

    const int64_t product0 = static_cast<int64_t>(GP4_FIELD(int32_t, rec, 0x64)) * rate;
    int32_t position = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product0) >> 16))
        + GP4_FIELD(int32_t, rec, 0x60);
    if (position < 0x400) {
        position = 0x400;
    }
    GP4_FIELD(int32_t, rec, 0x60) = position;

    const int64_t product1 = static_cast<int64_t>(GP4_FIELD(int16_t, rec, 0x6a)) * rate;
    const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(product1) >> 16);
    const uint16_t sum = static_cast<uint16_t>(static_cast<uint16_t>(shifted) + GP4_FIELD(uint16_t, rec, 0x68));
    const int16_t floorValue = GP4_FIELD(int16_t, rec, 0x5a);
    const uint32_t flags = cmpFlags16(sum, static_cast<uint16_t>(floorValue));
    const uint16_t stored = static_cast<int16_t>(sum) < floorValue ? static_cast<uint16_t>(floorValue) : sum;
    GP4_FIELD(uint16_t, rec, 0x68) = stored;

    r->eax = (shifted & 0xffff0000u) | stored;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product1) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00444008, FUN_00444008, "regs(esi:rec[0x854414,0xe0,0..0]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0062d1b4+rec_00444008")

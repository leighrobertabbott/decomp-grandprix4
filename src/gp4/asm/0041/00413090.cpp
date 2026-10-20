#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_00413090_Entry {
    uint8_t Unknown00[0x20];
    int32_t Unknown20;
    int32_t Unknown24;
    int32_t Unknown28;
    uint8_t Unknown2C[0x40 - 0x2c];
};

struct Unknown_00413090_Edi {
    uint8_t Unknown00[0xb6];
    uint8_t Unknown00B6;
};

static inline int32_t absolute(int32_t value) {
    return value < 0 ? static_cast<int32_t>(0u - static_cast<uint32_t>(value)) : value;
}

// FUNCTION: GP4 0x00413090
// edi is a 0x160-byte record pointer. Records below the pointer in DAT_00891500 hold a table index at
// +0xb6: index 0 leaves eax untouched (CF clear, the flags of the "or"); otherwise eax = the largest
// absolute value of the dwords +0x20/+0x24/+0x28 of the 0x40-byte entry 0x00626fb8 + index * 0x40 (edx =
// the last absolute value, flags of the last cmp with CF set by stc). Other records give
// eax = |(word at the record four ahead - word at the record) >> 2| (CF set, flags of the test/neg).
void __cdecl FUN_00413090(gp4::Regs* r) {
    const uint32_t record = r->edi;

    if (record < GP4_GLOBAL(uint32_t, 0x00891500)) {
        const uint32_t index = reinterpret_cast<const Unknown_00413090_Edi*>(static_cast<uintptr_t>(record))->Unknown00B6;
        if (index == 0u) {
            // "or ebx, ebx" gives ZF = PF = 1 and clears the rest; clc clears CF.
            r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
            return;
        }
        const Unknown_00413090_Entry* entry = reinterpret_cast<const Unknown_00413090_Entry*>(
            static_cast<uintptr_t>(0x00626fb8u + index * 0x40u));

        int32_t best = absolute(entry->Unknown20);
        const int32_t second = absolute(entry->Unknown24);
        if (!(best >= second)) {
            best = second;
        }
        const int32_t third = absolute(entry->Unknown28);
        const uint32_t flags = subtractionFlags32(static_cast<uint32_t>(best), static_cast<uint32_t>(third));
        if (!(best >= third)) {
            best = third;
        }

        r->eax = static_cast<uint32_t>(best);
        r->edx = static_cast<uint32_t>(third);
        // stc
        r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | 1u;
        return;
    }

    const uint16_t first = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(record));
    uint32_t next = record + 0x580u;
    if (!(next < GP4_GLOBAL(uint32_t, 0x00866ef8))) {
        next = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
    }
    const uint16_t second = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(next));
    const int32_t shifted = static_cast<int32_t>(static_cast<int16_t>(static_cast<uint16_t>(second - first))) >> 2;

    uint32_t flags;
    int32_t magnitude = shifted;
    if (shifted < 0) {
        // neg eax
        magnitude = static_cast<int32_t>(0u - static_cast<uint32_t>(shifted));
        const uint32_t value = static_cast<uint32_t>(shifted);
        const uint32_t result = static_cast<uint32_t>(magnitude);
        flags = (value != 0u ? 1u : 0u) | parityFlag(result) | ((value ^ result) & 0x10u)
            | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u) | (value == 0x80000000u ? 0x800u : 0u);
    } else {
        // test eax, -1
        flags = parityFlag(static_cast<uint32_t>(shifted)) | (shifted == 0 ? 0x40u : 0u);
    }

    r->eax = static_cast<uint32_t>(magnitude);
    r->ecx = (r->ecx & 0xffff0000u) | first;
    // stc
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | 1u;
}
GP4_IMPL(0x00413090, FUN_00413090, "regs(eax:u32, ecx:u32, edx:u32, edi:rec[0x8943a0,0x160,0..29]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+words_00413090")

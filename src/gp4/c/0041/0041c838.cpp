#include <gp4/gp4.h>

// Record reached through the pointer globals 0x0060203c / 0x00602050.
struct Unknown_0041c838 {
    uint8_t Unknown00[6];
    int16_t Unknown06;
    uint8_t Unknown08[0x48 - 0x08];
    uint16_t Unknown48;
    uint8_t Unknown4a[0x60 - 0x4a];
    uint16_t Unknown60;
    uint16_t Unknown62;
    uint8_t Unknown64[0x74 - 0x64];
    uint16_t Unknown74;
    uint16_t Unknown76;
    uint8_t Unknown78[0xd8 - 0x78];
    int16_t UnknownD8;
    int16_t UnknownDA;
    uint8_t UnknownDC[0xe0 - 0xdc];
    int16_t UnknownE0;
    int16_t UnknownE2;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// add left, right
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (((left ^ result) & (right ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (result < left ? 1u : 0u);
}

// One of the two symmetric halves: |x| / scale as a 0..0x4000 fraction, then
// a blend of the record's words. ecx keeps what the original leaves in it.
static inline uint32_t blendHalf(uint32_t recordAddress, uint32_t x, int32_t selector, uint32_t* ecx) {
    const Unknown_0041c838* rec = reinterpret_cast<const Unknown_0041c838*>(static_cast<uintptr_t>(recordAddress));
    const bool negative = static_cast<int32_t>(x) < 0;

    uint32_t scale;
    if (selector == 0) {
        scale = (static_cast<uint32_t>(negative ? rec->Unknown60 : rec->Unknown62) + 0x50u) << 8;
    } else if (selector < 0) {
        scale = rec->Unknown74;
        if ((rec->Unknown48 & 0x200u) == 0u) scale -= 0x50u;
        scale <<= 8;
    } else {
        scale = rec->Unknown76;
        if ((rec->Unknown48 & 0x100u) == 0u) scale -= 0x50u;
        scale <<= 8;
    }
    if (static_cast<int32_t>(scale) < 0x8000) scale = 0x8000u;

    const uint32_t magnitude = negative ? 0u - x : x;
    int32_t remainder;
    int32_t fraction = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(magnitude)) << 14,
                                   static_cast<int32_t>(scale), &remainder);
    if (fraction > 0x4000) fraction = 0x4000;
    if (fraction < 0) fraction = 0;

    const uint32_t offset = static_cast<uint32_t>(static_cast<int32_t>(rec->Unknown06)) << 8;
    if (selector == 0) {
        *ecx = x;
        const int32_t level = negative ? rec->UnknownD8 : rec->UnknownDA;
        const int64_t product = static_cast<int64_t>(level) * static_cast<int64_t>(fraction);
        return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 6) + offset;
    }
    const uint32_t from = static_cast<uint32_t>(static_cast<int32_t>(selector < 0 ? rec->UnknownD8 : rec->UnknownDA)) << 8;
    const uint32_t to = static_cast<uint32_t>(static_cast<int32_t>(selector < 0 ? rec->UnknownE0 : rec->UnknownE2)) << 8;
    *ecx = from;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(to - from)) * static_cast<int64_t>(fraction);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + from + offset;
}

// FUNCTION: GP4 0x0041c838
void __cdecl FUN_0041c838(gp4::Regs* r) {
    uint32_t ecx = r->ecx;

    const uint32_t first = blendHalf(GP4_GLOBAL(uint32_t, 0x0060203c), GP4_GLOBAL(uint32_t, 0x0060202c),
                                     GP4_GLOBAL(int32_t, 0x00602038), &ecx);
    GP4_GLOBAL(uint32_t, 0x00602034) = first;
    const uint32_t secondRecord = GP4_GLOBAL(uint32_t, 0x00602050);
    const uint32_t second = blendHalf(secondRecord, GP4_GLOBAL(uint32_t, 0x00602040),
                                      GP4_GLOBAL(int32_t, 0x0060204c), &ecx);
    GP4_GLOBAL(uint32_t, 0x00602048) = second;

    const uint32_t total = GP4_GLOBAL(uint32_t, 0x00602030);
    int32_t span = static_cast<int32_t>(total - GP4_GLOBAL(uint32_t, 0x00602044));
    if (span < 0x8000) span = 0x8000;
    GP4_GLOBAL(int32_t, 0x00602058) = span;

    int32_t remainder;
    int32_t weight = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(total)) << 14, span, &remainder);
    if (weight < GP4_GLOBAL(int32_t, 0x00602010)) weight = GP4_GLOBAL(int32_t, 0x00602010);
    if (weight > GP4_GLOBAL(int32_t, 0x00602014)) weight = GP4_GLOBAL(int32_t, 0x00602014);
    GP4_GLOBAL(int32_t, 0x00602054) = weight;

    const uint32_t low = GP4_GLOBAL(uint32_t, 0x00602034);
    const uint32_t high = GP4_GLOBAL(uint32_t, 0x00602048);
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(high - low)) * static_cast<int64_t>(weight);
    const uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const uint32_t result = scaled + low;

    r->eax = result;
    r->ecx = ecx;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->edi = secondRecord;
    r->ebp = static_cast<uint32_t>(weight);
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(scaled, low, result);
}
GP4_IMPL(0x0041c838, FUN_0041c838, "regs() -> (eax, ecx, edx, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0041c838")

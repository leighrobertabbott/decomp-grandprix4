#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t compare32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Five input bytes at ebx (the sample the routine reads).
struct Unknown_0043bee0_Input {
    uint8_t Unknown00;
    uint8_t Unknown01;
    uint8_t Unknown02;
    uint8_t Unknown03;
    uint8_t Unknown04;
};

// FUNCTION: GP4 0x0043bee0
// Publishes the sample's byte +1 (DAT_00622834), min(byte +3, byte +1) (DAT_00602fa4) and byte +4
// clamped to 0 below 0x10 and to 16 * table[byte+1 >> 4] (table 0x937dd0) (DAT_00622838), then for
// each of six channels (DAT_00622830 = 0..5) looks the channel up with 0x0041ef48 at
// max(max(16 * channel + byte+1 - 0x50, byte+0) - DAT_00622838, DAT_00602fa4) and accumulates
// result * gain[channel] (gains at 0x622888). Returns eax = min(0xbf, (sum + 0x2000 : high of the last
// product) >> 14) and edx = DAT_00622838; ebp, ecx, edi and esi are preserved and the flags are those
// of `cmp eax, 0xbf`.
void __cdecl FUN_0043bee0(gp4::Regs* r) {
    const Unknown_0043bee0_Input* in = reinterpret_cast<const Unknown_0043bee0_Input*>(static_cast<uintptr_t>(r->ebx));

    const int32_t position = in->Unknown01;
    GP4_GLOBAL(int32_t, 0x00622834) = position;
    int32_t lowBound = in->Unknown03;
    if (lowBound > position) lowBound = position;
    GP4_GLOBAL(int32_t, 0x00602fa4) = lowBound;

    const int32_t base = in->Unknown00;
    int32_t limit = static_cast<int32_t>(GP4_ARRAY(const uint8_t, 0x00937dd0)[static_cast<uint32_t>(position) >> 4]) << 4;
    int32_t offset = in->Unknown04;
    if (offset < 0x10) offset = 0;
    if (offset > limit) offset = limit;
    GP4_GLOBAL(int32_t, 0x00622838) = offset;

    const int32_t* gain = reinterpret_cast<const int32_t*>(GP4_ARRAY(uint8_t, 0x00622888));
    uint32_t sum = 0;
    uint32_t high = 0;
    for (uint32_t channel = 0; channel != 6u; ++channel) {
        GP4_GLOBAL(uint32_t, 0x00622830) = channel;

        int32_t sample = static_cast<int32_t>(channel << 4) + GP4_GLOBAL(int32_t, 0x00622834) - 0x50;
        if (sample < base) sample = base;
        sample -= GP4_GLOBAL(int32_t, 0x00622838);
        if (sample < GP4_GLOBAL(int32_t, 0x00602fa4)) sample = GP4_GLOBAL(int32_t, 0x00602fa4);

        gp4::Regs call = *r;
        call.ebp = static_cast<uint32_t>(sample);
        call.ecx = channel;
        gp4::call_regs(0x0041ef48, call);

        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(call.eax)) * gain[channel];
        sum += static_cast<uint32_t>(product);
        high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    }

    const uint32_t shifted = static_cast<uint32_t>(((static_cast<uint64_t>(high) << 32) | (sum + 0x2000u)) >> 14);
    const uint32_t flags = compare32Flags(shifted, 0xbfu);
    r->eax = static_cast<int32_t>(shifted) > 0xbf ? 0xbfu : shifted;
    r->edx = GP4_GLOBAL(uint32_t, 0x00622838);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043bee0, FUN_0043bee0, "regs(ebx:ptr[0x10]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

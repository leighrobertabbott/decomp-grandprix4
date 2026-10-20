#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 16-bit `cmp left, right`.
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) >> 4);
}

// Q14 cosine of a 16-bit angle from the word table at 0x007c5c5c (byte offset (|a| >> 2) & 0xfffe).
static inline int16_t cosQ14(uint16_t angle) {
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    const uint32_t offset = (static_cast<uint32_t>(angle) >> 2) & 0xfffeu;
    return GP4_ARRAY(int16_t, 0x007c5c5c)[offset >> 1];
}

// Clamps a signed 16-bit value to +-limit (the magnitude is compared unsigned).
static inline uint16_t clampSigned16(uint16_t value, uint16_t limit) {
    if (static_cast<int16_t>(value) < 0) {
        value = static_cast<uint16_t>(0u - value);
        if (value >= limit) {
            value = limit;
        }
        return static_cast<uint16_t>(0u - value);
    }
    if (value >= limit) {
        value = limit;
    }
    return value;
}

// esi and ebx are two car records (0x66fef0, stride 0x41c). The offset of ebx from esi (dwords +0
// and +4, >> 8) is rotated into esi's frame by esi's heading word +0x18 (Q14 cos/sin table), after
// 0x004377a4 projected it; the two rotated words are clamped to +-0x200 (the second after adding a
// term of the heading difference word +0x18(ebx) - +0x18(esi), clamped to +-0x800, >> 2). With
// ebx's word +0x9a as a scale, they form a term clamped to +-0x3fff that is scaled by the word
// [0x629dc4] (>> 3) and added to esi's dword +0xac and subtracted from its dword +0x158; when its
// magnitude >> 11 reaches the word [0x629dc8] (signed) 0x00416387 runs on esi. The scratch words
// at 0x934de4..0x934dfc keep the intermediates.
// FUNCTION: GP4 0x0043747c
void __cdecl FUN_0043747c(gp4::Regs* r) {
    uint8_t* self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));

    const uint16_t heading = GP4_FIELD(uint16_t, self, 0x18);
    GP4_GLOBAL(uint16_t, 0x00934df8) = GP4_FIELD(uint16_t, other, 0x9a);
    GP4_GLOBAL(uint16_t, 0x00934dfc) = static_cast<uint16_t>(GP4_FIELD(uint16_t, other, 0x18) - heading);
    const uint16_t cosHeading = static_cast<uint16_t>(cosQ14(heading));
    const uint16_t sinHeading = static_cast<uint16_t>(cosQ14(static_cast<uint16_t>(0x4000u - heading)));
    GP4_GLOBAL(uint16_t, 0x00934de8) = cosHeading;
    GP4_GLOBAL(uint16_t, 0x00934dec) = sinHeading;
    GP4_GLOBAL(uint32_t, 0x00934df0) = static_cast<uint32_t>(
        static_cast<int32_t>(GP4_FIELD(uint32_t, other, 0x0) - GP4_FIELD(uint32_t, self, 0x0)) >> 8);
    const uint32_t offsetY = static_cast<uint32_t>(
        static_cast<int32_t>(GP4_FIELD(uint32_t, other, 0x4) - GP4_FIELD(uint32_t, self, 0x4)) >> 8);
    GP4_GLOBAL(uint32_t, 0x00934df4) = offsetY;

    gp4::Regs project = *r;
    project.st_in = 0;
    project.st_out = 0;
    project.st_xin_mask = 0;
    project.st_xout_mask = 0;
    project.eax = offsetY;
    project.edx = sinHeading;
    gp4::call_regs(0x004377a4, project);

    const int32_t x = GP4_GLOBAL(int16_t, 0x00934df0);
    const int32_t y = GP4_GLOBAL(int16_t, 0x00934df4);
    const int32_t cosine = GP4_GLOBAL(int16_t, 0x00934de8);
    const int32_t sine = GP4_GLOBAL(int16_t, 0x00934dec);
    const uint32_t rotatedX = static_cast<uint32_t>(x * cosine) - static_cast<uint32_t>(y * sine);
    const uint32_t rotatedY = static_cast<uint32_t>(y * cosine) + static_cast<uint32_t>(x * sine);
    GP4_GLOBAL(uint16_t, 0x00934df2) = static_cast<uint16_t>(rotatedX >> 16);
    GP4_GLOBAL(uint16_t, 0x00934df6) = static_cast<uint16_t>(rotatedY >> 16);
    uint16_t frameX = static_cast<uint16_t>(rotatedX >> 14);
    uint16_t frameY = static_cast<uint16_t>(rotatedY >> 14);

    const uint16_t delta = GP4_GLOBAL(uint16_t, 0x00934dfc);
    uint16_t turn = static_cast<int16_t>(delta) < 0 ? static_cast<uint16_t>(0u - delta) : delta;
    if (static_cast<int16_t>(frameX ^ delta) >= 0) {
        turn = static_cast<uint16_t>(0u - turn);
    }
    turn = clampSigned16(turn, 0x800u);
    frameY = static_cast<uint16_t>(frameY + static_cast<uint16_t>(static_cast<int16_t>(turn) >> 2));
    frameY = clampSigned16(frameY, 0x200u);
    frameX = clampSigned16(frameX, 0x200u);
    GP4_GLOBAL(uint16_t, 0x00934df4) = frameY;
    GP4_GLOBAL(uint16_t, 0x00934df0) = frameX;

    const int32_t scale = GP4_GLOBAL(int16_t, 0x00934df8);
    const uint16_t first = static_cast<uint16_t>(static_cast<uint32_t>(cosQ14(delta) * scale) >> 14);
    const int32_t difference = static_cast<int16_t>(static_cast<uint16_t>(GP4_FIELD(uint16_t, self, 0x9a) - first));
    const uint32_t partial = static_cast<uint32_t>((difference * static_cast<int16_t>(frameX)) >> 2);
    GP4_GLOBAL(uint16_t, 0x00934de4) = static_cast<uint16_t>(partial);
    GP4_GLOBAL(uint16_t, 0x00934de6) = static_cast<uint16_t>(partial >> 16);
    const int32_t second = static_cast<int16_t>(static_cast<uint16_t>(
        static_cast<uint32_t>(cosQ14(static_cast<uint16_t>(0x4000u - delta)) * scale) >> 14));
    const uint32_t sum = static_cast<uint32_t>(second * static_cast<int16_t>(frameY)) + partial;

    uint32_t term = static_cast<uint32_t>(static_cast<int32_t>(sum) >> 8);
    if (static_cast<int32_t>(term) < 0) {
        term = 0u - term;
        if (term >= 0x3fffu) {
            term = 0x3fffu;
        }
        term = 0u - term;
    } else if (term >= 0x3fffu) {
        term = 0x3fffu;
    }

    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(term))
        * static_cast<int64_t>(GP4_GLOBAL(int16_t, 0x00629dc4));
    const uint32_t force = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 3);
    GP4_FIELD(uint32_t, self, 0xac) += force;
    GP4_FIELD(uint32_t, self, 0x158) += 0u - force;
    uint32_t magnitude = 0u - force;
    if (static_cast<int32_t>(magnitude) < 0) {
        magnitude = 0u - magnitude;
    }
    magnitude >>= 11;

    r->eax = magnitude;
    r->ecx = (r->ecx & 0xffff0000u) | first;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);

    const uint16_t threshold = GP4_GLOBAL(uint16_t, 0x00629dc8);
    if (static_cast<int16_t>(static_cast<uint16_t>(magnitude)) < static_cast<int16_t>(threshold)) {
        r->eflags = (r->eflags & ~0x8d5u) | subFlags16(magnitude, threshold);
        return;
    }

    gp4::Regs mark = *r;
    mark.st_in = 0;
    mark.st_out = 0;
    mark.st_xin_mask = 0;
    mark.st_xout_mask = 0;
    gp4::call_regs(0x00416387, mark);
    r->eflags = mark.eflags;
}
GP4_IMPL(0x0043747c, FUN_0043747c, "regs(ecx:u32, ebx:rec[0x66fef0,0x41c,0..21], esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+sel_004377a4")

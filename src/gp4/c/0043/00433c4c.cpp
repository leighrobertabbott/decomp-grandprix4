#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of the 32-bit `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// (a * b) >> 14 as `imul; shrd eax, edx, 14` leaves it in eax.
static inline uint32_t scaleQ14(int32_t a, int32_t b) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> 14);
}

struct Unknown_00433c4c_Record {
    uint8_t Unknown00[0xe4];
    int32_t UnknownE4;
    uint8_t UnknownE8[8];
    int32_t UnknownF0;
    uint8_t UnknownF4[4];
    int32_t UnknownF8;
};

// Table lookup of the Q14 cosine/sine words (0x007c5c5c) for a signed 16-bit angle: |angle| >> 2,
// masked to an even byte offset.
static inline int32_t sineWord(uint16_t magnitude) {
    return GP4_ARRAY(int16_t, 0x007c5c5c)[(static_cast<uint32_t>(magnitude >> 2) & 0xfffeu) >> 1];
}

// FUNCTION: GP4 0x00433c4c
// Position of the car's reference point on its segment record edi (0x0043207c, +0x160 more from
// 0x00432095 unless the mode byte 0x007c2ea9 is 0x80). Any other mode returns the lane position of
// 0x0043353e. In mode 0x80 the record's angle (the high word of dword +0xe4) turns the offset vector
// (0x00627874 << 8, mirrored by the sign of 0x00868582, and 0x00627878) through the Q14 table, and
// eax / edx = record dwords +0xf0 / +0xf8 plus that rotated vector, ecx = record dword +0xe4. ebx,
// ebp and edi are restored; the flags are those of the final `add edx, ebp`.
void __cdecl FUN_00433c4c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x0043207c, c);
    if (GP4_GLOBAL(uint8_t, 0x007c2ea9) != 0x80u) {
        gp4::call_regs(0x00432095, c);
    }
    c.edi = c.eax;

    if (GP4_GLOBAL(uint8_t, 0x007c2ea9) != 0x80u) {
        gp4::call_regs(0x0043353e, c);
        r->eax = c.eax;
        r->ecx = c.ecx;
        r->edx = c.edx;
        r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
        return;
    }

    const Unknown_00433c4c_Record* record = reinterpret_cast<const Unknown_00433c4c_Record*>(static_cast<uintptr_t>(c.edi));
    const uint16_t angle = static_cast<uint16_t>(record->UnknownE4 >> 16);

    uint16_t complement = static_cast<uint16_t>(static_cast<uint16_t>(0u - angle) + 0x4000u);
    if (static_cast<int16_t>(complement) < 0) {
        complement = static_cast<uint16_t>(0u - complement);
    }
    const int32_t negSine = 0 - sineWord(complement);

    uint16_t magnitude = angle;
    if (static_cast<int16_t>(angle) < 0) {
        magnitude = static_cast<uint16_t>(0u - angle);
    }
    const int32_t cosine = sineWord(magnitude);

    int32_t along = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00627874) << 8);
    if (GP4_GLOBAL(int8_t, 0x00868582) < 0) {
        along = 0 - along;
    }
    const int32_t across = GP4_GLOBAL(int32_t, 0x00627878);

    const uint32_t x = scaleQ14(along, cosine) - scaleQ14(across, negSine);
    const uint32_t y = scaleQ14(across, cosine) + scaleQ14(along, negSine);

    r->eax = static_cast<uint32_t>(record->UnknownF0) + x;
    r->edx = static_cast<uint32_t>(record->UnknownF8) + y;
    r->ecx = static_cast<uint32_t>(record->UnknownE4);
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(static_cast<uint32_t>(record->UnknownF8), y);
}
GP4_IMPL(0x00433c4c, FUN_00433c4c, "regs(esi:ptr[0x273]:layout=car_00431ffb) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00431ffb+state_004059ef+geom_008943a0+rec_00433c4c")

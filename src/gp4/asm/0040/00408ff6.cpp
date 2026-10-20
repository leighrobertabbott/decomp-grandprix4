#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `left - right` (cmp / sub / neg = 0 - x).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul ; shrd eax, edx, 14: low dword of (a * b) >> 14.
static inline int32_t mulShr14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
}

// Symmetric clamp with the original's unsigned compare: sign(value) * min(|value|, limit).
static inline int32_t clampSymmetric(int32_t value, uint32_t limit) {
    if (value < 0) {
        uint32_t magnitude = 0u - static_cast<uint32_t>(value);
        if (!(magnitude < limit)) {
            magnitude = limit;
        }
        return static_cast<int32_t>(0u - magnitude);
    }
    uint32_t magnitude = static_cast<uint32_t>(value);
    if (!(magnitude < limit)) {
        magnitude = limit;
    }
    return static_cast<int32_t>(magnitude);
}

// Fields of the record (esi) this routine reads.
struct Unknown_00408ff6_Body {
    uint8_t Unknown00[0xac];
    int32_t UnknownAC;
    uint8_t UnknownB0[0xe5 - 0xb0];
    uint8_t FlagsE5;
    uint8_t UnknownE6[0x288 - 0xe6];
    int32_t Unknown288;
};

// FUNCTION: GP4 0x00408ff6
// Three-channel smoothing filter on the words at 0x00602974 / 0x00602976 / 0x00602978. With
// A = (dword +0xac, or +0x288 when bit 0 of +0xe5 is clear) >> 7 scaled by [0x0062246d] (>> 14):
//   w76 = clamp((A + w76) >> 1, +/-[0x602964]);
//   w74 = clamp(((-|A|) * [0x622471] >> 14 + w74) >> 1, +/-[0x602968]);
//   w78 = ((A * [0x622475] >> 14) + w78) >> 1, limited to +/-[0x60296c] or, when it has the same
//   sign as the new w76, to [0x60296c] -/+ (w76 * [0x602970] >> 14).
// Returns eax = w74, edx = w76, ecx = w78 (sign-extended). The flags are those of the last
// compare / negate on the w78 path. ebp is restored.
void __cdecl FUN_00408ff6(gp4::Regs* r) {
    const Unknown_00408ff6_Body* body = reinterpret_cast<const Unknown_00408ff6_Body*>(static_cast<uintptr_t>(r->esi));

    const int32_t source = (body->FlagsE5 & 1u) == 0u ? body->Unknown288 : body->UnknownAC;
    const int32_t drive = mulShr14(source >> 7, GP4_GLOBAL(int32_t, 0x0062246d));

    // channel 0x602976
    const int32_t sum76 = static_cast<int32_t>(static_cast<uint32_t>(drive) + static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00602976))));
    const int16_t word76 = static_cast<int16_t>(clampSymmetric(sum76 >> 1, GP4_GLOBAL(uint32_t, 0x00602964)));
    GP4_GLOBAL(int16_t, 0x00602976) = word76;

    // channel 0x602974
    int32_t magnitude = drive;
    if (magnitude < 0) {
        magnitude = static_cast<int32_t>(0u - static_cast<uint32_t>(magnitude));
    }
    const int32_t falling = mulShr14(static_cast<int32_t>(0u - static_cast<uint32_t>(magnitude)), GP4_GLOBAL(int32_t, 0x00622471));
    const int32_t sum74 = static_cast<int32_t>(static_cast<uint32_t>(falling) + static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00602974))));
    const int16_t word74 = static_cast<int16_t>(clampSymmetric(sum74 >> 1, GP4_GLOBAL(uint32_t, 0x00602968)));
    GP4_GLOBAL(int16_t, 0x00602974) = word74;

    // channel 0x602978
    const int32_t rising = mulShr14(drive, GP4_GLOBAL(int32_t, 0x00622475));
    const int32_t sum78 = static_cast<int32_t>(static_cast<uint32_t>(rising) + static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00602978))));
    const int32_t value78 = sum78 >> 1;
    const int32_t current76 = static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00602976));
    const uint32_t limit78 = GP4_GLOBAL(uint32_t, 0x0060296c);

    int32_t result;
    uint32_t flags;
    if (value78 >= 0 && current76 >= 0) {
        const int32_t cap = static_cast<int32_t>(limit78 - static_cast<uint32_t>(mulShr14(current76, GP4_GLOBAL(int32_t, 0x00602970))));
        flags = subFlags32(static_cast<uint32_t>(value78), static_cast<uint32_t>(cap));
        result = value78 > cap ? cap : value78;
    } else if (value78 < 0 && current76 < 0) {
        const int32_t floorValue = static_cast<int32_t>((0u - limit78) - static_cast<uint32_t>(mulShr14(current76, GP4_GLOBAL(int32_t, 0x00602970))));
        flags = subFlags32(static_cast<uint32_t>(value78), static_cast<uint32_t>(floorValue));
        result = value78 < floorValue ? floorValue : value78;
    } else if (value78 >= 0) {
        flags = subFlags32(static_cast<uint32_t>(value78), limit78);
        result = static_cast<uint32_t>(value78) < limit78 ? value78 : static_cast<int32_t>(limit78);
    } else {
        uint32_t clipped = 0u - static_cast<uint32_t>(value78);
        if (!(clipped < limit78)) {
            clipped = limit78;
        }
        flags = subFlags32(0u, clipped);
        result = static_cast<int32_t>(0u - clipped);
    }
    const int16_t word78 = static_cast<int16_t>(result);
    GP4_GLOBAL(int16_t, 0x00602978) = word78;

    r->eax = static_cast<uint32_t>(static_cast<int32_t>(word74));
    r->edx = static_cast<uint32_t>(static_cast<int32_t>(word76));
    r->ecx = static_cast<uint32_t>(static_cast<int32_t>(word78));
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x00408ff6, FUN_00408ff6, "regs(esi:ptr[0x290]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

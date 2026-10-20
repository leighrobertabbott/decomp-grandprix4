#include <gp4/gp4.h>

struct Unknown_004065c3 {
    uint8_t Unknown00[0x118];
    int32_t Pos118;
    int32_t Pos11c;
    uint8_t Unknown120[0xc];
    uint16_t Angle12c;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Q14 table word at byte offset (|x| >> 2) & 0xfffe of the table at 0x007c5c5c (16384 = 1.0).
static inline int32_t q14Lookup(uint16_t index) {
    return GP4_ARRAY(int16_t, 0x007c5c5c)[((index >> 2) & 0xfffeu) >> 1];
}

// (a * b) >> 14 as imul + shrd eax, edx, 14 leave it in eax.
static inline int32_t mulQ14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * b;
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
}

static inline int32_t wrapSub(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint32_t>(a) - static_cast<uint32_t>(b));
}

static inline int32_t wrapAdd(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint32_t>(a) + static_cast<uint32_t>(b));
}

// pushal ... popal routine, the mirror of 0x004066be for the point at (+0x118, +0x11c) and the
// direction angle +0x12c: the signed distance of the reference point (0x006278c0, 0x006278c8) from the
// line, with the opposite sign, stored at 0x00629edc[ebx]; when it is not negative the foot of the
// perpendicular goes to 0x00629ee4/0x00629eec[ebx] and the difference vector to 0x00629ef4/0x00629efc[ebx],
// CF = 1. A negative distance stores nothing more and gives CF = 0. All registers are restored.
// FUNCTION: GP4 0x004065c3
void __cdecl FUN_004065c3(gp4::Regs* r) {
    const Unknown_004065c3* body = reinterpret_cast<const Unknown_004065c3*>(static_cast<uintptr_t>(r->edi));
    const uint32_t slot = r->ebx;

    const uint16_t angle = body->Angle12c;
    uint16_t complement = static_cast<uint16_t>(static_cast<uint16_t>(0u - angle) + 0x4000u);
    if (static_cast<int16_t>(complement) < 0) {
        complement = static_cast<uint16_t>(0u - complement);
    }
    const int32_t first = q14Lookup(complement);

    uint16_t magnitude = angle;
    if (static_cast<int16_t>(magnitude) < 0) {
        magnitude = static_cast<uint16_t>(0u - magnitude);
    }
    const int32_t second = q14Lookup(magnitude);

    const int32_t refX = GP4_GLOBAL(int32_t, 0x006278c0);
    const int32_t refZ = GP4_GLOBAL(int32_t, 0x006278c8);

    const int32_t part = mulQ14(wrapSub(refZ, body->Pos11c), first);
    const int32_t distance = wrapSub(0, wrapSub(mulQ14(wrapSub(refX, body->Pos118), second), part));
    GP4_ARRAY(int32_t, 0x00629edc)[slot] = distance;

    if (distance < 0) {
        // last flag writer "or eax, eax" (negative); clc
        r->eflags = (r->eflags & ~0x8d5u) | 0x80u | parityFlag(static_cast<uint32_t>(distance));
        return;
    }

    const int32_t along = wrapAdd(mulQ14(wrapSub(refX, body->Pos118), first), mulQ14(wrapSub(refZ, body->Pos11c), second));
    const int32_t footX = wrapAdd(mulQ14(along, first), body->Pos118);
    GP4_ARRAY(int32_t, 0x00629ee4)[slot] = footX;
    const int32_t footZ = wrapAdd(mulQ14(along, second), body->Pos11c);
    GP4_ARRAY(int32_t, 0x00629eec)[slot] = footZ;
    GP4_ARRAY(int32_t, 0x00629ef4)[slot] = wrapSub(refX, footX);

    // last flag writer: eax = refZ - footZ (32-bit sub), then stc
    const uint32_t left = static_cast<uint32_t>(refZ);
    const uint32_t right = static_cast<uint32_t>(footZ);
    const uint32_t diff = left - right;
    GP4_ARRAY(int32_t, 0x00629efc)[slot] = static_cast<int32_t>(diff);
    const uint32_t flags = 1u
        | parityFlag(diff)
        | ((left ^ right ^ diff) & 0x10u)
        | (diff == 0u ? 0x40u : 0u)
        | ((diff >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ diff)) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004065c3, FUN_004065c3, "regs(edi:ptr[0x136]:bytes, ebx:u32[0..1]) -> (cf, pf, af, zf, sf, of); globals=layout=state_004067ea")

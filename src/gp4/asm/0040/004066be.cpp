#include <gp4/gp4.h>

struct Unknown_004066be {
    uint8_t Unknown00[0x120];
    int32_t Pos120;
    int32_t Pos124;
    uint8_t Unknown128[6];
    uint16_t Angle12e;
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

// pushal ... popal routine: finds the foot of the perpendicular from the reference point
// (0x006278c0, 0x006278c8) onto the line through (+0x120, +0x124) with direction angle +0x12e,
// storing the signed distance at 0x00629edc[ebx], the foot at 0x00629ee4/0x00629eec[ebx] and
// the difference vector at 0x00629ef4/0x00629efc[ebx]. CF = 1 when the foot was stored; CF = 0
// (and nothing else stored) when the distance is negative. All registers are restored.
// FUNCTION: GP4 0x004066be
void __cdecl FUN_004066be(gp4::Regs* r) {
    const Unknown_004066be* body = reinterpret_cast<const Unknown_004066be*>(static_cast<uintptr_t>(r->edi));
    const uint32_t slot = r->ebx;

    const uint16_t angle = body->Angle12e;
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

    int32_t part = mulQ14(static_cast<int32_t>(static_cast<uint32_t>(refZ) - static_cast<uint32_t>(body->Pos124)), first);
    int32_t distance = static_cast<int32_t>(
        static_cast<uint32_t>(mulQ14(static_cast<int32_t>(static_cast<uint32_t>(refX) - static_cast<uint32_t>(body->Pos120)), second)) -
        static_cast<uint32_t>(part));
    GP4_ARRAY(int32_t, 0x00629edc)[slot] = distance;

    if (distance < 0) {
        // last flag writer "or eax, eax" (negative); clc
        r->eflags = (r->eflags & ~0x8d5u) | 0x80u | parityFlag(static_cast<uint32_t>(distance));
        return;
    }

    int32_t along = static_cast<int32_t>(
        static_cast<uint32_t>(mulQ14(static_cast<int32_t>(static_cast<uint32_t>(refX) - static_cast<uint32_t>(body->Pos120)), first)) +
        static_cast<uint32_t>(mulQ14(static_cast<int32_t>(static_cast<uint32_t>(refZ) - static_cast<uint32_t>(body->Pos124)), second)));
    const int32_t footX = static_cast<int32_t>(static_cast<uint32_t>(mulQ14(along, first)) + static_cast<uint32_t>(body->Pos120));
    GP4_ARRAY(int32_t, 0x00629ee4)[slot] = footX;
    const int32_t footZ = static_cast<int32_t>(static_cast<uint32_t>(mulQ14(along, second)) + static_cast<uint32_t>(body->Pos124));
    GP4_ARRAY(int32_t, 0x00629eec)[slot] = footZ;
    GP4_ARRAY(int32_t, 0x00629ef4)[slot] = static_cast<int32_t>(static_cast<uint32_t>(refX) - static_cast<uint32_t>(footX));

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
GP4_IMPL(0x004066be, FUN_004066be, "regs(edi:ptr[0x136]:bytes, ebx:u32[0..1]) -> (cf, pf, af, zf, sf, of); globals=layout=state_004067ea")

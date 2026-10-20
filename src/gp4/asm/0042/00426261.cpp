#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `cmp left, right`.
static inline uint32_t cmp32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul + shrd eax, edx, 14: bits 14..45 of the signed 64-bit product.
static inline uint32_t mulShr14(uint32_t a, uint32_t b) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// edx after a one-operand imul: the high 32 bits of the signed product.
static inline uint32_t mulHigh(uint32_t a, uint32_t b) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
}

struct Unknown_00426261 {
    uint8_t Unknown00[0x66];
    int16_t Unknown66;
    uint8_t Unknown68[0xa2 - 0x68];
    uint16_t UnknownA2;
    uint8_t UnknownA4[0x26b - 0xa4];
    uint8_t Flags26B;
};

struct Unknown_00426261_Segment {
    uint8_t Unknown00[0x51];
    uint8_t Flags51;
    uint8_t Unknown52[0x71 - 0x52];
    uint8_t Unknown71;
    uint8_t Unknown72[0x7a - 0x72];
    uint16_t Unknown7A;
};

// FUNCTION: GP4 0x00426261
// Limit of a car's travel measured in 0x4000 units against a track segment (ebx) and the car
// body (esi): eax = 0 when body flag +0x26b.7 or segment flag +0x51.7 is set, or when the entry of
// the table 0x00675a82 (stride 10) selected by word +0x7a of the segment (Unknown71 - 6) records
// ahead, wrapped by the pit/lap bounds 0x891500 / 0x866ef8 / 0x866ecc / 0x866ec4, has a zero
// byte +1; otherwise eax = clamp(((0x4000 - [esi+0xa2]) * FUN_00426216 >> 14) * ((Unknown71 << 14) -
// [esi+0x66]) >> 14, 0, (Unknown71 << 14) - [esi+0x66]). edx is the last imul high word (or the table
// pointer / the unchanged input on the early exits); the flags are those of the last flag writer.
void __cdecl FUN_00426261(gp4::Regs* r) {
    Unknown_00426261* body = reinterpret_cast<Unknown_00426261*>(static_cast<uintptr_t>(r->esi));
    const uint32_t segmentAddress = r->ebx;
    const Unknown_00426261_Segment* segment = reinterpret_cast<const Unknown_00426261_Segment*>(static_cast<uintptr_t>(segmentAddress));

    uint32_t eax = 0;
    uint32_t edx = r->edx;
    uint32_t flags;

    if ((body->Flags26B & 0x80u) != 0u) {
        flags = parityFlag8(0x80u) | 0x80u;
    } else if ((segment->Flags51 & 0x80u) != 0u) {
        flags = parityFlag8(0x80u) | 0x80u;
    } else {
        const uint32_t step = (static_cast<uint32_t>(segment->Unknown71) - 6u) * 0x160u;
        uint32_t edi = segmentAddress;
        if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
            edi += step;
            if (!(edi < GP4_GLOBAL(uint32_t, 0x00866ecc))) {
                edi -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            }
        } else {
            edi += step;
            if (!(edi < GP4_GLOBAL(uint32_t, 0x00866ef8))) {
                edi = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
            }
        }
        const uint32_t word = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(edi + 0x7au));
        edx = word * 10u + 0x00675a82u;
        const uint32_t entry = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(edx + 1u));
        if (entry == 0u) {
            flags = parityFlag8(0u) | 0x40u;    // or eax,eax with eax = 0
        } else {
            int32_t limit = static_cast<int32_t>(0x4000u - body->UnknownA2);
            if (limit < 0) limit = 0;

            gp4::Regs call = *r;
            call.eax = static_cast<uint32_t>(limit);
            gp4::call_regs(0x00426216, call);

            const uint32_t first = mulShr14(static_cast<uint32_t>(limit), call.edx);
            const uint32_t ebp = (static_cast<uint32_t>(segment->Unknown71) << 14) - static_cast<uint32_t>(static_cast<int32_t>(body->Unknown66));
            uint32_t result = mulShr14(first, ebp);
            edx = mulHigh(first, ebp);
            if (static_cast<int32_t>(result) < 0) result = 0;
            flags = cmp32Flags(result, ebp);
            if (static_cast<int32_t>(result) > static_cast<int32_t>(ebp)) result = ebp;
            eax = result;
        }
    }

    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00426261, FUN_00426261, "regs(edx:u32, esi:ptr[0x273]:bytes, ebx:rec[0x8943a0,0x160,0..25]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+ring_0040c0ae")

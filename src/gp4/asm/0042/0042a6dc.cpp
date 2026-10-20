#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `left - right` (sub / cmp / neg when left is 0).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_0042a6dc_Body {
    uint8_t Unknown00[0x9a];
    int16_t Unknown9A;
    uint8_t Unknown9C;
    uint8_t Unknown9D[0xd0 - 0x9d];
    uint32_t UnknownD0;
    uint8_t UnknownD4[0xe2 - 0xd4];
    uint16_t UnknownE2;
};

// FUNCTION: GP4 0x0042a6dc
// esi / ebx = the two bodies. ax = difference of their words +0xe2 (or the 0x0040dc8a value when
// either has bit 1 of +0x9c set) minus the words 0x00603ba0 and 0x00603ba4. When negative:
// ecx = -word(0x00623e04). Otherwise ecx = |(+0xd0 difference >> 4)| / max(2 * ax, 1) * [0x00603854]
// >> 16, capped at word(0x00623e04) and negated when the +0x9a difference (stored to 0x00934dec) is
// negative, then passed through 0x00426100 (eax = result, ecx = eax). The upper half of eax is kept
// on the negative path; edx and the flags come from the last writer (neg or the 0x00426100 call).
void __cdecl FUN_0042a6dc(gp4::Regs* r) {
    const Unknown_0042a6dc_Body* self = reinterpret_cast<const Unknown_0042a6dc_Body*>(static_cast<uintptr_t>(r->esi));
    const Unknown_0042a6dc_Body* other = reinterpret_cast<const Unknown_0042a6dc_Body*>(static_cast<uintptr_t>(r->ebx));

    uint16_t ax;
    if ((self->Unknown9C & 2u) == 0u && (other->Unknown9C & 2u) == 0u) {
        ax = static_cast<uint16_t>(other->UnknownE2 - self->UnknownE2);
    } else {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x0040dc8a, call);
        ax = static_cast<uint16_t>(call.eax);
    }

    ax = static_cast<uint16_t>(ax - GP4_GLOBAL(uint16_t, 0x00603ba0));
    if ((ax & 0x8000u) != 0u) {
        const uint32_t limit = GP4_GLOBAL(uint16_t, 0x00623e04);
        r->ecx = 0u - limit;
        r->eax = (r->eax & 0xffff0000u) | ax;
        r->eflags = (r->eflags & ~0x8d5u) | subFlags32(0u, limit);
        return;
    }

    ax = static_cast<uint16_t>(ax - GP4_GLOBAL(uint16_t, 0x00603ba4));
    const int32_t doubled = static_cast<int32_t>(static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(ax))) << 1);
    const int32_t positionDelta = static_cast<int32_t>(other->Unknown9A) - static_cast<int32_t>(self->Unknown9A);
    GP4_GLOBAL(int32_t, 0x00934dec) = positionDelta;

    int32_t divisor = doubled;
    if (divisor < 1) {
        divisor = 1;
    }
    int32_t distance = static_cast<int32_t>(other->UnknownD0 - self->UnknownD0) >> 4;
    if (distance < 0) {
        distance = static_cast<int32_t>(0u - static_cast<uint32_t>(distance));
    }
    const uint32_t quotient = static_cast<uint32_t>(distance) / static_cast<uint32_t>(divisor);
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(quotient)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854));
    int32_t value = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 16));
    const int32_t cap = static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00623e04));
    if (value > cap) {
        value = cap;
    }
    if (GP4_GLOBAL(int32_t, 0x00934dec) < 0) {
        value = static_cast<int32_t>(0u - static_cast<uint32_t>(value));
    }

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = static_cast<uint32_t>(value);
    call.ecx = static_cast<uint32_t>(value);
    call.edx = static_cast<uint32_t>(cap);
    gp4::call_regs(0x00426100, call);

    r->eax = call.eax;
    r->ecx = call.eax;
    r->edx = call.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0042a6dc, FUN_0042a6dc, "regs(eax:u32, esi:ptr[0x225]:bytes, ebx:ptr[0x225]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00426091")

#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit logic operation (`xor`): CF = OF = AF = 0.
static inline uint32_t logicFlags16(uint16_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x8000u) != 0u ? 0x80u : 0u);
}

// Flags of a 16-bit `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

struct Unknown_004255d4_Car {
    uint8_t Unknown00[0x9a];
    uint16_t Unknown9A;
    uint8_t Flags9C;
    uint8_t Unknown9D[0xda - 0x9d];
    uint16_t UnknownDA;
    uint8_t UnknownDC[0xe2 - 0xdc];
    uint16_t UnknownE2;
    uint8_t UnknownE4[0x100 - 0xe4];
    uint16_t Unknown100;
    uint8_t Unknown102[0x21e - 0x102];
    uint16_t Unknown21E;
};

// 16-bit `idiv cx` of dx:ax, guarded as the original does: the division runs only when dx is inside
// the half-divisor band (|quotient| < 0x8000, divisor nonzero); otherwise the quotient saturates to
// +-0x7fff by the sign of the divisor and dx keeps the dividend's high word.
static inline uint16_t guardedDivide(uint16_t* dx, uint16_t ax, uint16_t cx) {
    int16_t half = static_cast<int16_t>(static_cast<int16_t>(cx) >> 1);
    if (half < 0) {
        half = static_cast<int16_t>(half + 1);
    }
    const int16_t high = static_cast<int16_t>(*dx);
    bool divide;
    if (high < 0) {
        const int16_t bound = half >= 0 ? static_cast<int16_t>(-half) : half;
        if (high > bound) {
            divide = true;
        } else if (high < bound) {
            divide = false;
        } else {
            divide = ax != 0u;
        }
    } else {
        const int16_t bound = half < 0 ? static_cast<int16_t>(-half) : half;
        divide = high < bound;
    }

    if (divide) {
        const int32_t dividend = static_cast<int32_t>((static_cast<uint32_t>(*dx) << 16) | ax);
        const int32_t divisor = static_cast<int16_t>(cx);
        *dx = static_cast<uint16_t>(dividend % divisor);
        return static_cast<uint16_t>(dividend / divisor);
    }
    return static_cast<int16_t>(cx) < 0 ? static_cast<uint16_t>(0x8001u) : static_cast<uint16_t>(0x7fffu);
}

// ebx = the other car, esi = this car. Clears CF (no result) when the closing term
// ([ebx+0x100] - [ebx+0x9a]) disagrees in sign with [ebx+0xda] or the gap (+0xe2 difference, or
// 0x0040dc8a when either car has bit 1 of +0x9c) is below [0x00603b66]. Otherwise a clamped
// ratio, floored at [0x00623e02], goes through 0x00426100 into word [esi+0x21e] and CF is set.
// FUNCTION: GP4 0x004255d4
void __cdecl FUN_004255d4(gp4::Regs* r) {
    const Unknown_004255d4_Car* other = reinterpret_cast<const Unknown_004255d4_Car*>(static_cast<uintptr_t>(r->ebx));
    Unknown_004255d4_Car* car = reinterpret_cast<Unknown_004255d4_Car*>(static_cast<uintptr_t>(r->esi));

    const uint16_t closing = static_cast<uint16_t>(other->Unknown100 - other->Unknown9A);
    uint32_t square = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(closing))
        * static_cast<int32_t>(static_cast<int16_t>(closing)));
    if (static_cast<int16_t>(closing) < 0) {
        square = 0u - square;
    }
    uint16_t ax = static_cast<uint16_t>(square);
    uint16_t dx = static_cast<uint16_t>(square >> 16);
    const uint16_t sign = static_cast<uint16_t>(closing ^ other->UnknownDA);
    if (static_cast<int16_t>(sign) < 0) {
        r->eax = (r->eax & 0xffff0000u) | ax;
        r->edx = (r->edx & 0xffff0000u) | dx;
        r->ecx = (r->ecx & 0xffff0000u) | sign;
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags16(sign);
        return;
    }

    const int64_t scaled = static_cast<int64_t>(static_cast<int16_t>(other->UnknownDA))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00623df6));
    const uint32_t divisor = static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 10);
    ax = guardedDivide(&dx, ax, static_cast<uint16_t>(divisor));

    uint16_t ratio = static_cast<uint16_t>(0u - static_cast<uint16_t>(static_cast<int16_t>(ax) >> 4));
    if (static_cast<int16_t>(ratio) < 0) {
        uint16_t magnitude = static_cast<uint16_t>(0u - ratio);
        if (magnitude >= 0x4000u) {
            magnitude = 0x4000u;
        }
        ratio = static_cast<uint16_t>(0u - magnitude);
    } else if (ratio >= 0x4000u) {
        ratio = 0x4000u;
    }
    r->eax = (r->eax & 0xffff0000u) | ratio;
    r->ecx = divisor;
    r->edx = (r->edx & 0xffff0000u) | dx;

    uint16_t gap;
    if ((car->Flags9C & 2u) == 0u && (other->Flags9C & 2u) == 0u) {
        gap = static_cast<uint16_t>(other->UnknownE2 - car->UnknownE2);
    } else {
        const uint32_t savedEax = r->eax;
        gp4::call_regs(0x0040dc8a, *r);
        gap = static_cast<uint16_t>(r->eax);
        r->eax = savedEax;
    }
    r->edx = (r->edx & 0xffff0000u) | gap;
    const uint16_t minimum = GP4_GLOBAL(uint16_t, 0x00603b66);
    if (static_cast<int16_t>(gap) < static_cast<int16_t>(minimum)) {
        r->eflags = (r->eflags & ~0x8d5u) | (subFlags16(gap, minimum) & ~1u);
        return;
    }

    const uint16_t span = static_cast<uint16_t>(static_cast<uint16_t>(ratio + gap) << 1);
    r->ecx = (r->ecx & 0xffff0000u) | span;
    const uint16_t approach = static_cast<uint16_t>(other->Unknown100 - car->Unknown9A);
    const int32_t approachSquare = static_cast<int32_t>(static_cast<int16_t>(approach))
        * static_cast<int32_t>(static_cast<int16_t>(approach));
    const uint32_t shifted = static_cast<uint32_t>(approachSquare >> 4);
    dx = static_cast<uint16_t>(shifted >> 16);
    ax = guardedDivide(&dx, static_cast<uint16_t>(shifted), span);

    const int32_t product = static_cast<int32_t>(static_cast<int16_t>(ax))
        * static_cast<int32_t>(static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00603854)));
    dx = static_cast<uint16_t>(static_cast<uint32_t>(product) >> 16);
    if (static_cast<int16_t>(approach) < 0) {
        dx = static_cast<uint16_t>(0u - dx);
    }
    r->edx = (r->edx & 0xffff0000u) | dx;
    int16_t value = static_cast<int16_t>(dx);
    const int16_t floor = GP4_GLOBAL(int16_t, 0x00623e02);
    if (value < floor) {
        value = floor;
    }
    r->eax = static_cast<uint32_t>(static_cast<int32_t>(value));
    gp4::call_regs(0x00426100, *r);
    car->Unknown21E = static_cast<uint16_t>(r->eax);
    r->eflags |= gp4::CF;
}
GP4_IMPL(0x004255d4, FUN_004255d4, "regs(eax:u32, ecx:u32, edx:u32, ebx:ptr[0x228]:layout=other_004255d4, esi:ptr[0x228]:layout=rec_00426091) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); cases=400; globals=fuzz:i32+layout=state_00426091+cache_00426091+gain_004255d4")

#include <gp4/gp4.h>

enum : uint32_t { kArithFlags = 0x8d5u };

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Flags of `test byte, imm` (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline uint8_t* record(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

// low 32 bits of ((int64)a * b) >> shift (imul; shrd eax, edx, shift)
static inline uint32_t mulShift(uint32_t a, uint32_t b, int shift) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> shift);
}

// Per-car preparation: 0x436e51 when byte +0x7a bit 6 is set, 0x4048da (edi = segment +0x1c)
// unless byte +0xe5 bit 0 is set.
static inline void prepareCar(gp4::Regs& s) {
    if ((GP4_FIELD(uint8_t, record(s.esi), 0x7a) & 0x40u) != 0u) {
        gp4::call_regs(0x00436e51, s);
    }
    if ((GP4_FIELD(uint8_t, record(s.esi), 0xe5) & 1u) == 0u) {
        s.edi = GP4_FIELD(uint32_t, record(s.esi), 0x1c);
        gp4::call_regs(0x004048da, s);
    }
}

// Separate call sites (MSVC otherwise selects the address and makes one call).
static __declspec(noinline) void callCarUpdate0040f6d0(gp4::Regs& s) {
    gp4::call_regs(0x0040f6d0, s);
}
static __declspec(noinline) void callCarUpdate0043624f(gp4::Regs& s) {
    gp4::call_regs(0x0043624f, s);
}

// Per-car follow-up after a contact: 0x40496c, then 0x40f6d0 (byte +0xe5 bit 0 clear) or 0x43624f,
// then 0x409467, 0x43783c, 0x437846, and bits 7, 5 and 6 of byte +0xe5 are set.
static inline void finishCar(gp4::Regs& s) {
    gp4::call_regs(0x0040496c, s);
    if ((GP4_FIELD(uint8_t, record(s.esi), 0xe5) & 1u) == 0u) {
        callCarUpdate0040f6d0(s);
    } else {
        callCarUpdate0043624f(s);
    }
    gp4::call_regs(0x00409467, s);
    gp4::call_regs(0x0043783c, s);
    gp4::call_regs(0x00437846, s);
    uint8_t* const car = record(s.esi);
    GP4_FIELD(uint8_t, car, 0xe5) |= 0x80u;
    GP4_FIELD(uint8_t, car, 0xe5) |= 0x20u;
    GP4_FIELD(uint8_t, car, 0xe5) |= 0x40u;
}

static inline void swapCars(gp4::Regs& s) {
    const uint32_t t = s.esi;
    s.esi = s.ebx;
    s.ebx = t;
}

// FUNCTION: GP4 0x00436f5f
// pushal/popal routine for a pair of car records, esi = A and ebx = B (0x66fef0 array). Nothing
// happens when either car has bit 0 or 1 of byte +0x25f or bit 2 of byte +0xcc set, or when their
// words +0x27e differ by 0x3a8 or more. Otherwise the pair is published at 0x629e04/0x629e00
// (0x629e08 cleared), both cars are prepared, and 0x436e82 / 0x436970 test the pair; without a
// contact (CF clear) byte 0x629f30 is cleared and 0x436ecb runs. On a contact, 0x438878 runs, the
// dword pairs +0xa4 and +0xa8 of A and B pass through 0x436df3, then 0x43747c (A, B and swapped),
// 0x4373fe, 0x4371f3 and 0x438a41; the step k = (max([0x629e68], 0) * [0x603854]) >> 14 moves both
// positions +0/+4 by (+0xa4/+0xa8 * k) >> 24 and adds the high dword of A.+0xac * (k & 0xffff) to
// A.+0xc. 0x436eee marks both cars (A, B and swapped); when either is the car [0x679d30],
// [0x852dbc] = [0x66d814] + 500. Then 0x436ecb, the per-car follow-up for A and B, and 0x437859.
// Every GPR is restored; the flags are those of the last flag writer on the path taken.
void __cdecl FUN_00436f5f(gp4::Regs* r) {
    uint8_t* const carA = record(r->esi);
    uint8_t* const carB = record(r->ebx);

    uint32_t result = GP4_FIELD(uint8_t, carA, 0x25f) & 2u;
    if (result == 0u) {
        result = GP4_FIELD(uint8_t, carB, 0x25f) & 2u;
    }
    if (result == 0u) {
        result = GP4_FIELD(uint8_t, carA, 0x25f) & 1u;
    }
    if (result == 0u) {
        result = GP4_FIELD(uint8_t, carB, 0x25f) & 1u;
    }
    if (result == 0u) {
        result = GP4_FIELD(uint8_t, carA, 0xcc) & 4u;
    }
    if (result == 0u) {
        result = GP4_FIELD(uint8_t, carB, 0xcc) & 4u;
    }
    if (result != 0u) {
        r->eflags = (r->eflags & ~kArithFlags) | testFlags8(result);
        return;
    }

    GP4_GLOBAL(uint32_t, 0x00629e04) = r->esi;
    GP4_GLOBAL(uint32_t, 0x00629e00) = r->ebx;
    GP4_GLOBAL(uint32_t, 0x00629e08) = 0;

    const int32_t levelB = GP4_FIELD(int16_t, carB, 0x27e);
    int32_t gap = static_cast<int32_t>(GP4_FIELD(int16_t, carA, 0x27e)) - levelB;
    if (gap < 0) {
        gap = -gap;
    }
    const uint32_t gapFlags = cmpFlags32(static_cast<uint32_t>(gap), 0x3a8u);
    if (gap >= 0x3a8) {
        r->eflags = (r->eflags & ~kArithFlags) | gapFlags;
        return;
    }

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.eax = static_cast<uint32_t>(gap);
    s.edx = static_cast<uint32_t>(levelB);
    s.eflags = (s.eflags & ~kArithFlags) | gapFlags;

    prepareCar(s);
    s.esi = GP4_GLOBAL(uint32_t, 0x00629e00);
    prepareCar(s);

    s.esi = GP4_GLOBAL(uint32_t, 0x00629e04);
    s.ebx = GP4_GLOBAL(uint32_t, 0x00629e00);
    s.edi = GP4_FIELD(uint32_t, record(s.esi), 0x1c);
    gp4::call_regs(0x00436e82, s);
    gp4::call_regs(0x00436970, s);
    if ((s.eflags & 1u) == 0u) {
        GP4_GLOBAL(uint8_t, 0x00629f30) = 0;
        s.esi = GP4_GLOBAL(uint32_t, 0x00629e04);
        s.ebx = GP4_GLOBAL(uint32_t, 0x00629e00);
        gp4::call_regs(0x00436ecb, s);
        r->eflags = (r->eflags & ~kArithFlags) | (s.eflags & kArithFlags);
        return;
    }

    gp4::call_regs(0x00438878, s);
    s.eax = GP4_FIELD(uint32_t, record(s.esi), 0xa4);
    s.edx = GP4_FIELD(uint32_t, record(s.ebx), 0xa4);
    gp4::call_regs(0x00436df3, s);
    GP4_FIELD(uint32_t, record(s.esi), 0xa4) = s.eax;
    GP4_FIELD(uint32_t, record(s.ebx), 0xa4) = s.edx;
    s.eax = GP4_FIELD(uint32_t, record(s.esi), 0xa8);
    s.edx = GP4_FIELD(uint32_t, record(s.ebx), 0xa8);
    gp4::call_regs(0x00436df3, s);
    GP4_FIELD(uint32_t, record(s.esi), 0xa8) = s.eax;
    GP4_FIELD(uint32_t, record(s.ebx), 0xa8) = s.edx;

    gp4::call_regs(0x0043747c, s);
    swapCars(s);
    gp4::call_regs(0x0043747c, s);
    swapCars(s);
    gp4::call_regs(0x004373fe, s);
    gp4::call_regs(0x004371f3, s);
    gp4::call_regs(0x00438a41, s);

    int32_t amount = GP4_GLOBAL(int32_t, 0x00629e68);
    if (amount < 0) {
        amount = 0;
    }
    const uint32_t step = mulShift(static_cast<uint32_t>(amount), GP4_GLOBAL(uint32_t, 0x00603854), 14);
    {
        uint8_t* const a = record(s.esi);
        uint8_t* const b = record(s.ebx);
        GP4_FIELD(uint32_t, a, 0) += mulShift(GP4_FIELD(uint32_t, a, 0xa4), step, 24);
        GP4_FIELD(uint32_t, a, 4) += mulShift(GP4_FIELD(uint32_t, a, 0xa8), step, 24);
        GP4_FIELD(uint32_t, b, 0) += mulShift(GP4_FIELD(uint32_t, b, 0xa4), step, 24);
        GP4_FIELD(uint32_t, b, 4) += mulShift(GP4_FIELD(uint32_t, b, 0xa8), step, 24);
        const uint32_t low = step & 0xffffu;
        const int64_t spin = static_cast<int64_t>(GP4_FIELD(int32_t, a, 0xac)) * static_cast<int64_t>(low);
        const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(spin) >> 32);
        GP4_FIELD(uint32_t, a, 0xc) += high;
        s.eax = static_cast<uint32_t>(static_cast<uint64_t>(spin) >> 8);
        s.edx = high;
        s.ecx = low;
    }

    s.esi = GP4_GLOBAL(uint32_t, 0x00629e04);
    s.ebx = GP4_GLOBAL(uint32_t, 0x00629e00);
    gp4::call_regs(0x00436eee, s);
    swapCars(s);
    gp4::call_regs(0x00436eee, s);
    swapCars(s);
    if (s.esi == GP4_GLOBAL(uint32_t, 0x00679d30) || s.ebx == GP4_GLOBAL(uint32_t, 0x00679d30)) {
        GP4_GLOBAL(uint32_t, 0x00852dbc) = GP4_GLOBAL(uint32_t, 0x0066d814) + 500u;
    }

    s.edi = GP4_FIELD(uint32_t, record(s.esi), 0x1c);
    gp4::call_regs(0x00436ecb, s);
    finishCar(s);
    s.esi = GP4_GLOBAL(uint32_t, 0x00629e00);
    s.edi = GP4_FIELD(uint32_t, record(s.esi), 0x1c);
    finishCar(s);
    gp4::call_regs(0x00437859, s);

    r->eflags = (r->eflags & ~kArithFlags) | (s.eflags & kArithFlags);
}
GP4_IMPL(0x00436f5f, FUN_00436f5f, "regs(esi:rec[0x66fef0,0x41c,0..21], ebx:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+trackgeo_008943a0+tbl_00868598+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+pitw_00409467+ranges_004322a6+state_0040705f+frac_0040705f+state_00407af6+state_004381ab+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+state_0040759e+trackfar_008943a0+sel_004377a4+state_00437888+pair_00437859+carst_0040f6d0+cars_00444d60+pair_00436970+state_00418756")

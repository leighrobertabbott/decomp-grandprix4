#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `add dst, src` (CF PF AF ZF SF OF).
static inline uint32_t addFlags32(uint32_t dst, uint32_t src) {
    const uint32_t result = dst + src;
    return (result < dst ? 0x01u : 0u)
        | parityFlag8(result)
        | (((dst ^ src ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(dst ^ src) & (dst ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Starts a debris object for the car in esi: takes the first free slot (0x4247fa; CF set and nothing
// done when there is none), marks it used (1) in [0x63bcfc + slot], stores the car's object number
// (byte +0x202) to [0x63bd3c + slot], clears the 0x78-byte object record 0x63bd7c + slot * 0x78 (0x424865),
// fills its slot (+0x6d), car number (+0x6c), "all" mask (+0x74) and flag bit 0 (+0x6f), runs 0x424878 with
// the mask [0x639634] and stamps the due time +0x60 with the clock [0x66d814] plus 30000 (byte 0x7c2e75
// zero), 10000 (car byte +0x70 below 2) or 5000. pushal/popal restore every register; clc replaces CF in
// the flags of the last add, stc in the flags of the failed slot search.
// FUNCTION: GP4 0x00424d65
void __cdecl FUN_00424d65(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs search = *r;
    search.st_in = 0;
    search.st_out = 0;
    gp4::call_regs(0x004247fa, search);
    if ((search.eflags & 1u) != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | (search.eflags & 0x8d5u);
        return;
    }

    const uint32_t slot = search.eax;
    GP4_ARRAY(uint8_t, 0x0063bcfc)[slot] = 1;
    GP4_ARRAY(uint8_t, 0x0063bd3c)[slot] = GP4_FIELD(uint8_t, car, 0x202);

    uint8_t* object = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x0063bd7cu + slot * 0x78u));

    gp4::Regs clear = search;
    clear.eax = slot;
    clear.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(object));
    gp4::call_regs(0x00424865, clear);

    GP4_FIELD(uint8_t, object, 0x6d) = static_cast<uint8_t>(clear.eax);
    GP4_FIELD(uint8_t, object, 0x6c) = GP4_FIELD(uint8_t, car, 0x202);
    GP4_FIELD(uint32_t, object, 0x74) = 0xffffffffu;
    GP4_FIELD(uint8_t, object, 0x6f) |= 1;

    gp4::Regs start = clear;
    start.eax = GP4_GLOBAL(uint32_t, 0x00639634);
    gp4::call_regs(0x00424878, start);

    uint32_t extra;
    if (GP4_GLOBAL(uint8_t, 0x007c2e75) == 0u) {
        extra = 0x7530u;
    } else if (static_cast<int8_t>(GP4_FIELD(uint8_t, car, 0x70)) < 2) {
        extra = 0x2710u;
    } else {
        extra = 0x1388u;
    }
    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
    GP4_FIELD(uint32_t, object, 0x60) = clock;
    GP4_FIELD(uint32_t, object, 0x60) = clock + extra;
    r->eflags = (r->eflags & ~0x8d5u) | (addFlags32(clock, extra) & ~1u);
}
GP4_IMPL(0x00424d65, FUN_00424d65, "regs(esi:ptr[0x41c]:layout=car_0041e500+rec_00420e1d+car_00424878+car_00424cf5) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bytes_0063bcfc+state_0042458b+state_00420e1d+blk_00424553+state_0066d8c0+state_0041bce9+state_00444517+state_00424a21+state_0043c4d9+mask_00639634")

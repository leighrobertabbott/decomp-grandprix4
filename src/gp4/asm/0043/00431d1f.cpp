#include <gp4/gp4.h>

struct Unknown_00431d1f_Car {
    uint8_t Unknown00[0x7c];
    uint8_t CarNumber7C;
    uint8_t Unknown7D[2];
    uint8_t Level7F;
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of "cmp left, right" on a dword.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00431d1f
// pushal wrapper: steps the level byte +0x7f (0..5) of the car in esi by the signed amount in eax,
// using the per-car limit byte 0x0066d94c[car] (0xff = no limit): with a limit, a step of +1 jumps
// below level 2 straight to 2, other steps only act above level 2 or exactly at level 2 (which
// takes the limit); levels below 2 return at once (flags of that compare). Otherwise the clamped
// level selects a dword of the table 0x006f2cc4 which 0x004411f7 turns into the best column,
// stored at 0x0066d936[car]. popal restores every register, so the flags are the last writer's.
void __cdecl FUN_00431d1f(gp4::Regs* r) {
    Unknown_00431d1f_Car* car = reinterpret_cast<Unknown_00431d1f_Car*>(static_cast<uintptr_t>(r->esi));
    const int32_t step = static_cast<int32_t>(r->eax);

    int32_t slot = static_cast<int32_t>(car->CarNumber7C & 0x3fu) - 1;
    if (slot > 0x15) {
        slot = 0x15;
    }
    const int32_t limit = static_cast<int8_t>(GP4_ARRAY(uint8_t, 0x0066d94c)[slot]);
    int32_t level = car->Level7F;

    if (limit == -1) {
        level += step;
    } else if (step == 1) {
        if (level >= 2) {
            level += step;
        } else {
            level = 2;
        }
    } else if (level > 2) {
        level += step;
    } else if (level != 2) {
        r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(static_cast<uint32_t>(level), 2u);
        return;
    } else {
        level = limit;
    }

    if (level < 0) {
        level = 0;
    }
    if (level > 5) {
        level = 5;
    }
    car->Level7F = static_cast<uint8_t>(level);

    int32_t row = (static_cast<uint8_t>(car->CarNumber7C - 1u)) & 0x3f;
    if (row > 0x15) {
        row = 0x15;
    }

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.ebp = r->eax;
    call.ecx = static_cast<uint32_t>(limit);
    call.eax = static_cast<uint32_t>(row);
    call.edx = GP4_ARRAY(uint32_t, 0x006f2cc4)[car->Level7F];
    gp4::call_regs(0x004411f7, call);
    GP4_ARRAY(uint8_t, 0x0066d936)[call.eax] = static_cast<uint8_t>(call.edx);

    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00431d1f, FUN_00431d1f, "regs(eax:i32[-3..3], esi:ptr[0x87]:layout=lvl_00431d1f) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=counter_0040f0dd+tbl_0066d94c+levels_006f2cc4")

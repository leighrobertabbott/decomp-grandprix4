#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF PF AF ZF SF OF) of "cmp left, right".
static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00418756
// Only for the player's car record (esi == [0x00679d30]): stores the signed speed
// |[0x00623f10]| (negated when the dword +0xac of the record is negative) in 0x00623ea0, the
// quotient (record dword +0x2a8 << 24) / [0x00603854] in 0x00623ea8, clears 0x00623ea4 /
// 0x00623eac / 0x00623eb0 / 0x00623eb4 and runs 0x0041b035 then 0x0041b0ed with the registers as
// left by the idiv (edx = remainder). Flags are those of the "cmp esi, [0x679d30]" or the callees.
void __cdecl FUN_00418756(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    r->eflags = (r->eflags & ~0x8d5u) | compareFlags32(r->esi, GP4_GLOBAL(uint32_t, 0x00679d30));
    if (r->esi != GP4_GLOBAL(uint32_t, 0x00679d30)) {
        return;
    }

    uint32_t speed = GP4_GLOBAL(uint32_t, 0x00623f10);
    if ((speed & 0x80000000u) != 0u) {
        speed = 0u - speed;
    }
    if ((GP4_FIELD(uint32_t, car, 0xac) & 0x80000000u) != 0u) {
        speed = 0u - speed;
    }
    GP4_GLOBAL(uint32_t, 0x00623ea0) = speed;
    GP4_GLOBAL(uint32_t, 0x00623ea4) = 0;

    const int32_t value = GP4_FIELD(int32_t, car, 0x2a8);
    int32_t remainder;
    const int32_t quotient =
        gp4::idiv64(static_cast<int64_t>(value) << 24, GP4_GLOBAL(int32_t, 0x00603854), &remainder);
    GP4_GLOBAL(int32_t, 0x00623ea8) = quotient;
    GP4_GLOBAL(uint32_t, 0x00623eac) = 0;
    GP4_GLOBAL(uint32_t, 0x00623eb0) = 0;
    GP4_GLOBAL(uint32_t, 0x00623eb4) = 0;

    r->eax = static_cast<uint32_t>(quotient);
    r->edx = static_cast<uint32_t>(remainder);
    r->eflags = (r->eflags & ~0x8d5u) | compareFlags32(r->esi, GP4_GLOBAL(uint32_t, 0x00679d30));
    if (r->esi == GP4_GLOBAL(uint32_t, 0x00679d30)) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x0041b035, call);
        gp4::call_regs(0x0041b0ed, call);
        r->eax = call.eax;
        r->ebx = call.ebx;
        r->ecx = call.ecx;
        r->edx = call.edx;
        r->esi = call.esi;
        r->edi = call.edi;
        r->ebp = call.ebp;
        r->eflags = call.eflags;
    }
}
GP4_IMPL(0x00418756, FUN_00418756, "regs(ebx:u32, ecx:u32, ebp:u32, esi:ptr[0x420]:layout=car_0042ba87+rec_00418c68+car_00418756) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041b035+state_00418756")

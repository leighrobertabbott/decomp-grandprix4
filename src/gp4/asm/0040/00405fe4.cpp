#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right` / `sub left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Hands the registers and flags a called routine leaves back to the caller's register file.
static inline void takeResult(gp4::Regs* r, const gp4::Regs& call) {
    r->eax = call.eax;
    r->ebx = call.ebx;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->esi = call.esi;
    r->edi = call.edi;
    r->ebp = call.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}

// `sub edi, 0x160` and its flags: one step back through the 0x160-byte records.
static inline void stepBack(gp4::Regs* r, uint32_t segment) {
    r->edi = segment - 0x160u;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags32(segment, 0x160u);
}

// FUNCTION: GP4 0x00405fe4
// Track-segment ring step backwards for the record in edi (records of 0x160 bytes at 0x8943a0,
// esi = the car), the mirror of 0x00405f9e. Below the pit start [0x891500]: the lap start
// [0x891504] wraps to the lap end [0x866ecc] minus one record, and the pit exit [0x866f04] enters
// the pit end [0x866ef8] - 0x160 through 0x405e6e when bit 7 of byte +0x71 of the car is set. At or
// above it: the pit start itself goes to the pit entry [0x866f00] - 0x160 through 0x405efa. Any
// other record just steps back by 0x160. The flags are those of the last sub / test or of the
// called routine.
void __cdecl FUN_00405fe4(gp4::Regs* r) {
    const uint32_t segment = r->edi;
    const uint32_t pitStart = GP4_GLOBAL(uint32_t, 0x00891500);

    if (segment < pitStart) {
        if (segment == GP4_GLOBAL(uint32_t, 0x00891504)) {
            stepBack(r, GP4_GLOBAL(uint32_t, 0x00866ecc));
            return;
        }
        if (segment == GP4_GLOBAL(uint32_t, 0x00866f04)) {
            uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
            const uint32_t entered = GP4_FIELD(uint8_t, car, 0x71) & 0x80u;
            if (entered != 0u) {
                stepBack(r, GP4_GLOBAL(uint32_t, 0x00866ef8));
                gp4::Regs call = *r;
                call.st_in = 0;
                call.st_out = 0;
                gp4::call_regs(0x00405e6e, call);
                takeResult(r, call);
                return;
            }
        }
        stepBack(r, segment);
        return;
    }

    if (segment == pitStart) {
        stepBack(r, GP4_GLOBAL(uint32_t, 0x00866f00));
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x00405efa, call);
        takeResult(r, call);
        return;
    }
    stepBack(r, segment);
}
GP4_IMPL(0x00405fe4, FUN_00405fe4, "regs(esi:ptr[0x420]:bytes, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+state_00403800+ranges_004322a6+ring_00405a12+links_00405a12+state_004059ef+div_00403bb6")

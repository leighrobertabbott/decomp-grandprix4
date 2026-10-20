#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of a byte test result: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
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

// FUNCTION: GP4 0x00405f9e
// Track-segment ring step for the record in edi (records of 0x160 bytes at 0x8943a0, esi = the
// car). Below the pit start [0x891500]: the lap end [0x866ecc] wraps to the first record, and the
// pit entry [0x866f00] enters the pit through 0x405e6e when bit 7 of byte +0x71 of the car is set.
// At or above it: the pit end [0x866ef8] leaves through 0x405efa to the pit exit [0x866f04]. The
// flags are those of the last compare / test, or of the called routine.
void __cdecl FUN_00405f9e(gp4::Regs* r) {
    const uint32_t segment = r->edi;
    const uint32_t pitStart = GP4_GLOBAL(uint32_t, 0x00891500);

    if (segment < pitStart) {
        const uint32_t lapEnd = GP4_GLOBAL(uint32_t, 0x00866ecc);
        if (segment == lapEnd) {
            r->edi = 0x008943a0u;
            r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(segment, lapEnd);
            return;
        }
        const uint32_t pitEntry = GP4_GLOBAL(uint32_t, 0x00866f00);
        if (segment != pitEntry) {
            r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(segment, pitEntry);
            return;
        }
        uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
        const uint32_t entered = GP4_FIELD(uint8_t, car, 0x71) & 0x80u;
        if (entered == 0u) {
            r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(entered);
            return;
        }
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.edi = pitStart;
        gp4::call_regs(0x00405e6e, call);
        takeResult(r, call);
        return;
    }

    const uint32_t pitEnd = GP4_GLOBAL(uint32_t, 0x00866ef8);
    if (segment != pitEnd) {
        r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(segment, pitEnd);
        return;
    }
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.edi = GP4_GLOBAL(uint32_t, 0x00866f04);
    gp4::call_regs(0x00405efa, call);
    takeResult(r, call);
}
GP4_IMPL(0x00405f9e, FUN_00405f9e, "regs(esi:ptr[0x420]:bytes, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=state_00403800+ranges_004322a6+ring_00405a12+links_00405a12+state_004059ef+div_00403bb6")

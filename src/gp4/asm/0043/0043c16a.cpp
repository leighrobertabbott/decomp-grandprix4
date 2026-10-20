#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "cmp left, right"
static inline uint32_t compare32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0043c16a
// Interpolates within the dword table at edx: the stage eax (clamped to 2..4, edi) selects the
// samples around table[edi]; 0x43c0cd gives the fraction of ebp = table[edi] between the previous
// sample (table[edi-2] when eax == 0, else table[edi-1]) and table[edi+1]. The result is the
// fraction plus edi*256, clamped to 0x100..0x500. Leaves edi = edi*256, ebp = table[edi], edx from
// 0x43c0cd and the flags of the final compare with 0x500.
void __cdecl FUN_0043c16a(gp4::Regs* r) {
    const uint32_t stageIn = r->eax;
    int32_t stage = static_cast<int32_t>(stageIn);
    if (stage < 2) stage = 2;
    if (stage > 4) stage = 4;

    const uint32_t* table = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(r->edx)) + stage;
    const uint32_t previous = stageIn == 0u ? table[-2] : table[-1];
    const uint32_t current = table[0];
    const uint32_t next = table[1];

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = previous;
    call.ebp = current;
    call.edx = next;
    call.edi = static_cast<uint32_t>(stage);
    gp4::call_regs(0x0043c0cd, call);

    int32_t result = static_cast<int32_t>(call.eax + (static_cast<uint32_t>(stage) << 8));
    if (result < 0x100) result = 0x100;
    const uint32_t flags = compare32Flags(static_cast<uint32_t>(result), 0x500u);
    if (result > 0x500) result = 0x500;

    r->eax = static_cast<uint32_t>(result);
    r->edx = call.edx;
    r->edi = static_cast<uint32_t>(stage) << 8;
    r->ebp = current;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043c16a, FUN_0043c16a, "regs(eax:i32[-1..6], edx:ptr[0x20]:i32[-8..8]) -> (eax, edx, edi, ebp, cf, pf, af, zf, sf, of)")

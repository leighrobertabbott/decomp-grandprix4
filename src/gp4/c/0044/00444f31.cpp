#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit ADD left, right.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Opcode-stream handler for opcode 0x0b: releases the car record whose pointer is stored at 0x0066d828 (via
// 0x00442dc8, esi = that pointer) and advances the stream read cursor (pointer at 0x0062d1a8) past the opcode byte.
// ebx is left holding the old cursor; the flags are those of the final "add dword [cursor], 1".
// FUNCTION: GP4 0x00444f31
void __cdecl FUN_00444f31(gp4::Regs* r) {
    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x0062d1a8);
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x0066d828);

    gp4::Regs call = *r;
    call.ebx = cursor;
    call.esi = record;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00442dc8, call);
    r->eax = call.eax;
    r->ebx = call.ebx;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->esi = call.esi;
    r->edi = call.edi;
    r->ebp = call.ebp;

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062d1a8);
    GP4_GLOBAL(uint32_t, 0x0062d1a8) = before + 1u;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(before, 1u);
}
GP4_IMPL(0x00444f31, FUN_00444f31, "regs() -> (ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040fa01+stream_0062d1a0")

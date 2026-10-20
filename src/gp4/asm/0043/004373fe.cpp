#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a 32-bit logic result (test): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Both body pointers (esi, ebx): the one in esi gets the force terms of the first lane set and the
// one in ebx those of the second.
static inline void clearOutputs(uint8_t* body) {
    GP4_FIELD(uint32_t, body, 0x1a8) = 0u;
    GP4_FIELD(uint8_t, body, 0x1c4) = 0u;
}

// FUNCTION: GP4 0x004373fe
// pushal / popal wrapper for two car bodies (esi, ebx). When neither has a bit of 0x14280 in dword
// +0x40 and 0x00437298 accepts both (carry clear for esi and, swapped, for ebx), it computes the
// responses 0x00437229, then fills the lane bytes +0x1a8.. of esi from the record 0x629e0c (scales
// [0x629e28], [0x629e50]) and of ebx from 0x629e34 (scales swapped) with 0x004372ca. Otherwise it
// zeroes dword +0x1a8 and byte +0x1c4 of both. All registers are preserved; the flags are those of the
// last flag-writing instruction (the last test, the last 0x00437298 or 0x004372ca call).
void __cdecl FUN_004373fe(gp4::Regs* r) {
    uint8_t* first = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* second = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint32_t flags;
    bool reject;

    uint32_t masked = GP4_FIELD(uint32_t, first, 0x40) & 0x14280u;
    flags = logicFlags32(masked);
    reject = masked != 0u;
    if (!reject) {
        masked = GP4_FIELD(uint32_t, second, 0x40) & 0x14280u;
        flags = logicFlags32(masked);
        reject = masked != 0u;
    }
    if (!reject) {
        gp4::Regs one = *r;
        one.st_in = 0;
        one.st_out = 0;
        gp4::call_regs(0x00437298, one);
        flags = one.eflags & 0x8d5u;
        reject = (one.eflags & 1u) != 0u;
        if (!reject) {
            gp4::Regs two = *r;
            two.st_in = 0;
            two.st_out = 0;
            two.esi = r->ebx;
            two.ebx = r->esi;
            gp4::call_regs(0x00437298, two);
            flags = two.eflags & 0x8d5u;
            reject = (two.eflags & 1u) == 0u ? false : true;
        }
    }

    if (reject) {
        clearOutputs(first);
        clearOutputs(second);
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    gp4::Regs responses = *r;
    responses.st_in = 0;
    responses.st_out = 0;
    gp4::call_regs(0x00437229, responses);

    gp4::Regs lanesA = *r;
    lanesA.st_in = 0;
    lanesA.st_out = 0;
    lanesA.edi = 0x00629e0c;
    lanesA.eax = GP4_GLOBAL(uint32_t, 0x00629e28);
    lanesA.edx = GP4_GLOBAL(uint32_t, 0x00629e50);
    gp4::call_regs(0x004372ca, lanesA);

    gp4::Regs lanesB = *r;
    lanesB.st_in = 0;
    lanesB.st_out = 0;
    lanesB.esi = r->ebx;
    lanesB.ebx = r->esi;
    lanesB.edi = 0x00629e34;
    lanesB.eax = GP4_GLOBAL(uint32_t, 0x00629e50);
    lanesB.edx = GP4_GLOBAL(uint32_t, 0x00629e28);
    gp4::call_regs(0x004372ca, lanesB);

    r->eflags = (r->eflags & ~0x8d5u) | (lanesB.eflags & 0x8d5u);
}
GP4_IMPL(0x004373fe, FUN_004373fe, "regs(esi:ptr[0x310]:bytes, ebx:ptr[0x310]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

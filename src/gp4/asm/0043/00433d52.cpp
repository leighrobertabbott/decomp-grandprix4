#include <gp4/gp4.h>

struct Unknown_00433d52Target {
    uint8_t Unknown00[0x118];
    uint32_t Unknown118;
    uint32_t Unknown11C;
    uint32_t Unknown120;
    uint32_t Unknown124;
};

struct Unknown_00433d52 {
    uint8_t Unknown00[0x1c];
    const Unknown_00433d52Target* Unknown1C;
    uint8_t Unknown20[0x70];
    uint16_t Unknown90;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Loads ecx = pointer at +0x1c, then edx:eax = the 64-bit value at target+0x118 when the word at
// +0x90 is negative, else the one at target+0x120. The flags are those of the
// "test word ptr [esi+0x90], 0xffff" (CF = OF = AF = 0).
// FUNCTION: GP4 0x00433d52
void __cdecl FUN_00433d52(gp4::Regs* r) {
    const Unknown_00433d52* rec = reinterpret_cast<const Unknown_00433d52*>(static_cast<uintptr_t>(r->esi));
    const Unknown_00433d52Target* target = rec->Unknown1C;
    const uint32_t word = rec->Unknown90;

    if ((word & 0x8000u) != 0u) {
        r->eax = target->Unknown118;
        r->edx = target->Unknown11C;
    } else {
        r->eax = target->Unknown120;
        r->edx = target->Unknown124;
    }
    r->ecx = reinterpret_cast<uint32_t>(target);
    r->eflags = (r->eflags & ~0x8d5u)
        | parityFlag(word) | (word == 0u ? 0x40u : 0u) | ((word >> 8) & 0x80u);
}
GP4_IMPL(0x00433d52, FUN_00433d52, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0")

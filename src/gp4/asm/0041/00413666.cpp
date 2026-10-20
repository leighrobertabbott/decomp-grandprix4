#include <gp4/gp4.h>

struct Unknown_00413666 {
    uint8_t Unknown00[0xe];
    int16_t UnknownE;
    uint8_t Unknown10[0x160];
    int16_t Unknown170;
    uint8_t Unknown172[0xbe];
    uint8_t Flags230;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// When bit 4 of the byte at +0x230 is set: eax = (eax + (int16)(word(+0x170) - word(+0xe))) >> 1
// (arithmetic), edx = that sign-extended difference, and DAT_00638d20 is cleared. Flags are those
// of the final "sar eax, 1" (OF = 0, AF = 0), or of the "test byte ptr [esi+0x230], 0x10".
// FUNCTION: GP4 0x00413666
void __cdecl FUN_00413666(gp4::Regs* r) {
    const Unknown_00413666* rec = reinterpret_cast<const Unknown_00413666*>(static_cast<uintptr_t>(r->esi));
    const uint32_t masked = rec->Flags230 & 0x10u;
    uint32_t flags = parityFlag(masked) | (masked == 0u ? 0x40u : 0u);

    if (masked != 0u) {
        const int16_t difference = static_cast<int16_t>(
            static_cast<uint16_t>(rec->Unknown170) - static_cast<uint16_t>(rec->UnknownE));
        const uint32_t edx = static_cast<uint32_t>(static_cast<int32_t>(difference));
        const uint32_t sum = r->eax + edx;
        const uint32_t result = static_cast<uint32_t>(static_cast<int32_t>(sum) >> 1);
        GP4_GLOBAL(uint32_t, 0x00638d20) = 0u;
        r->eax = result;
        r->edx = edx;
        flags = (sum & 1u) | parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80000000u ? 0x80u : 0u);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00413666, FUN_00413666, "regs(eax:u32, esi:ptr[0x238]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

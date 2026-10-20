#include <gp4/gp4.h>

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (result < left ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_0043488b {
    uint8_t Unknown00[0x40];
    uint32_t Flags40;
    uint8_t Unknown44[0x7b - 0x44];
    uint8_t Flags7B;
    uint8_t Unknown7C[0xe5 - 0x7c];
    uint8_t FlagsE5;
    uint8_t UnknownE6[0xf0 - 0xe6];
    uint32_t UnknownF0;
    uint8_t UnknownF4[0x161 - 0xf4];
    uint8_t Flags161;
    uint8_t Unknown162[0x203 - 0x162];
    uint8_t Flags203;
    uint8_t Unknown204[0x231 - 0x204];
    uint8_t Flags231;
};

// FUNCTION: GP4 0x0043488b
void __cdecl FUN_0043488b(gp4::Regs* r) {
    Unknown_0043488b* body = reinterpret_cast<Unknown_0043488b*>(static_cast<uintptr_t>(r->esi));
    uint32_t addend = 15000u;
    if (GP4_GLOBAL(uint8_t, 0x007c2e75) != 0) {
        addend = 3000u;
    } else if ((body->FlagsE5 & 4u) != 0) {
        if ((body->Flags161 & 2u) != 0) {
            addend = 3000u;
        } else {
            gp4::Regs callee = *r;
            gp4::call_regs(0x00434875, callee);
            if ((callee.eflags & 1u) != 0 || (body->Flags40 & 0x15280u) != 0 || (body->Flags7B & 2u) == 0) {
                addend = 3000u;
            } else if ((body->Flags203 & 2u) != 0 || (body->Flags231 & 0x40u) != 0) {
                addend = 30000u;
            }
        }
    }
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x0066d814);
    body->UnknownF0 = base;
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(base, addend);
    body->UnknownF0 += addend;
}
GP4_IMPL(0x0043488b, FUN_0043488b, "regs(esi:ptr[0x240]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

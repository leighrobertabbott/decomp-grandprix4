#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Flags of cmp/test/xor against zero-extended logic: CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Dword fill gives small signed values so both zero tests and the sign test are reached.
// FUNCTION: GP4 0x0041c300
void __cdecl FUN_0041c300(gp4::Regs* r) {
    uint8_t* body = (uint8_t*)r->ebx;
    uint32_t eax = r->eax;

    const uint32_t first = GP4_FIELD(uint32_t, body, 0x1c);
    uint32_t flagValue = first;                       // cmp edx, 0
    if (first != 0u) {
        const uint32_t second = GP4_FIELD(uint32_t, body, 0x40);
        flagValue = second;                           // test [ebx+0x40], -1
        if (second != 0u) {
            flagValue = first ^ second;               // xor edx, [ebx+0x40]
            if ((flagValue & 0x80000000u) != 0u) {
                eax = 0;
            }
        }
    }

    r->eax = eax;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags32(flagValue);
}
GP4_IMPL(0x0041c300, FUN_0041c300, "regs(ebx:ptr[0x48]:layout=rec_0041c300, eax:u32) -> (eax, cf, pf, af, zf, sf, of)")

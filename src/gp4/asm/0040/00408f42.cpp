#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x00408f42
void __cdecl FUN_00408f42(gp4::Regs* r) {
    uint8_t* record = (uint8_t*)r->esi;
    const uint16_t* table = GP4_ARRAY(uint16_t, 0x0066fe3c);

    uint32_t eax = r->esi - 0x0066fef0u;
    uint32_t edx = r->edx;
    uint32_t flags;
    uint32_t index = 0;

    for (;;) {
        const uint16_t entry = table[index];
        ++index;
        edx = (edx & 0xffff0000u) | entry;
        if ((entry & 0x8000u) != 0u) {
            // test dx,0xffff -> mov ax,0 : upper half of eax is kept
            eax &= 0xffff0000u;
            flags = parityFlag(entry) | 0x80u;
            break;
        }
        if ((uint16_t)eax == entry) {
            // mov eax,ebx / sub eax,0x66fe3e with ebx = table + 2*index
            const uint32_t ebx = 0x0066fe3cu + 2u * index;
            const uint32_t result = ebx - 0x0066fe3eu;
            flags = (ebx < 0x0066fe3eu ? 1u : 0u)
                | parityFlag(result)
                | (((ebx ^ 0x0066fe3eu ^ result) & 0x10u) != 0u ? 0x10u : 0u)
                | (result == 0u ? 0x40u : 0u)
                | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
                | ((((ebx ^ 0x0066fe3eu) & (ebx ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
            eax = result;
            break;
        }
    }

    record[0x7d] = (uint8_t)eax;
    record[0x415] = (uint8_t)eax;

    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408f42, FUN_00408f42, "regs(esi:u32[6749936..6755644], edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of)")

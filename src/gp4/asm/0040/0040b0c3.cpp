#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040b0c3
void __cdecl FUN_0040b0c3(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x00630224);
    uint8_t* const end = GP4_ARRAY(uint8_t, 0x0063e18c);
    // rep movsb with ecx = end - dst = 0xdf68 (direction flag clear)
    while (dst != end) {
        *dst++ = *src++;
    }
    r->eax = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));    // esi after the copy
    // The flags are those of the constant `sub ecx, edi`: all six arithmetic flags clear
    r->eflags = r->eflags & ~0x8d5u;
}
GP4_IMPL(0x0040b0c3, FUN_0040b0c3, "regs(esi:ptr[0xdf70]:bytes) -> (eax, cf, pf, af, zf, sf, of)")

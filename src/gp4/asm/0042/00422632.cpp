#include <gp4/gp4.h>

// FUNCTION: GP4 0x00422632
void __cdecl FUN_00422632(gp4::Regs* r) {
    // rep stosb: clear DAT_0060932c[0 .. DAT_0062a05c) (direction flag clear)
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x0062a05c);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x0060932c);
    for (uint32_t i = 0; i < count; ++i) {
        dst[i] = 0u;
    }
    r->eax = r->eax & 0xffffff00u;    // mov al, 0
    r->ecx = 0u;                      // rep counts ecx down to zero
    r->edi = 0x0060932cu + count;     // edi advances one byte per store
}
GP4_IMPL(0x00422632, FUN_00422632, "regs(eax:u32) -> (eax, ecx, edi); globals=layout=state_00422632")

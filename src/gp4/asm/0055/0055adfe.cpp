#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0055adfe
// Stores the low nibble of al into nibble number (dx) of the packed nibble table at 0x870618:
// even indices use the high nibble of byte dx/2, odd indices the low nibble. ax is masked to
// 0xf (ah cleared), and for an even index al is shifted into the high nibble. Leaves edx = dx>>1
// and the flags of the final `cmp al, cl` (new nibble vs old nibble in the same position, so ZF
// means the nibble did not change). ebx and ecx are saved and restored.
void __cdecl FUN_0055adfe(gp4::Regs* r) {
    uint8_t* const table = GP4_ARRAY(uint8_t, 0x00870618);
    const uint32_t index = r->edx & 0xffffu;
    uint32_t value = r->eax & 0xfu;
    uint8_t* const cell = table + (index >> 1);

    uint32_t old;
    if ((index & 1u) != 0u) {
        old = *cell & 0xfu;
        *cell = (uint8_t)(*cell & 0xf0u);
    } else {
        old = *cell & 0xf0u;
        value = (value << 4) & 0xffu;
        *cell = (uint8_t)(*cell & 0x0fu);
    }
    *cell = (uint8_t)(*cell | value);

    r->eax = (r->eax & 0xffff0000u) | value;
    r->edx = index >> 1;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(value, old));
}
GP4_IMPL(0x0055adfe, FUN_0055adfe, "regs(eax:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

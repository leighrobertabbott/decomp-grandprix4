#include <gp4/gp4.h>

// Builds 7 bytes at 0x00601258 from the 6 source bytes at eax: entries 0..5 are each byte & 0x7f
// clamped to 0x50 and entry 6 is the sign-bit mask (+0x11 when nonzero), all computed by 0x00410b3e
// (ecx = index, edx = source). The 7 bytes are then copied to edx's original value (rep movsb).
// eax = the last result, ecx = 0, edx = the source pointer; esi/edi are saved and restored. The
// loop ends on `cmp ecx, 7` with ecx = 7, so ZF = PF = 1 and the rest clear (rep movsb keeps them).
// FUNCTION: GP4 0x00410c1b
void __cdecl FUN_00410c1b(gp4::Regs* r) {
    const uint32_t source = r->eax;
    uint8_t* destination = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edx));
    uint8_t* buffer = GP4_ARRAY(uint8_t, 0x00601258);

    uint32_t result = 0;
    for (uint32_t index = 0; index != 7; ++index) {
        gp4::Regs callee = *r;
        callee.ecx = index;
        callee.edx = source;
        gp4::call_regs(0x00410b3e, callee);
        result = callee.eax;
        buffer[index] = static_cast<uint8_t>(result);
    }

    for (int i = 0; i < 7; ++i) {
        destination[i] = buffer[i];
    }

    r->eax = result;
    r->ecx = 0;
    r->edx = source;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00410c1b, FUN_00410c1b, "regs(eax:ptr[0x10]:bytes, edx:ptr[0x10]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of)")

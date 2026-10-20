#include <gp4/gp4.h>

// Copies the 7 bytes at eax to 0x00601258, then runs 0x00410b8c for each of them with
// eax = byte, ecx = index and edx unchanged (the callee keeps eax/ecx/edx). On exit ecx = 7,
// eax = the last byte, esi/edi are restored and the flags are those of the final `cmp ecx, 7`
// (equal: ZF = PF = 1).
// FUNCTION: GP4 0x00410c46
void __cdecl FUN_00410c46(gp4::Regs* r) {
    const uint8_t* source = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->eax));
    uint8_t* const copy = GP4_ARRAY(uint8_t, 0x00601258);
    for (uint32_t i = 0; i < 7u; ++i) {
        copy[i] = source[i];
    }

    gp4::Regs inner = *r;
    for (uint32_t i = 0; i < 7u; ++i) {
        inner.eax = copy[i];
        inner.ecx = i;
        gp4::call_regs(0x00410b8c, inner);
    }

    r->eax = inner.eax;
    r->ecx = 7u;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00410c46, FUN_00410c46, "regs(eax:ptr[16]:bytes, edx:ptr[0x40]:bytes) -> (eax, ecx, cf, pf, af, zf, sf, of)")

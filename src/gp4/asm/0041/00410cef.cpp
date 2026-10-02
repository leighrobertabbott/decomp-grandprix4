#include <gp4/gp4.h>

// FUNCTION: GP4 0x00410cef
void __cdecl FUN_00410cef(gp4::Regs* r) {
    const uint8_t* body = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const bool flagSet = (body[0xcf] & 4u) != 0u;

    // test byte [esi+0xcf], 4: CF = OF = AF = 0, SF = 0; the result is 0 or 4,
    // so ZF = clear and PF = set only for the zero result.
    const uint32_t flags = flagSet ? 0u : 0x44u;

    r->eax = flagSet ? 0xffffffffu : 0u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00410cef, FUN_00410cef, "regs(esi:ptr[0xd7]:bytes) -> (eax, cf, pf, af, zf, sf, of)")

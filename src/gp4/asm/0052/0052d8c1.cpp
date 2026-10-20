#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the key-binding input state: the six-word working copy of the key bindings at 0x007ab55a is cleared
// (`rep stosw`, leaving edi just past it and ecx = 0), the key queue is drained (0x0052d752, entered with
// ax = 0), the 256-bit wanted-key bitmap at 0x008245a0 is cleared (0x0052d6d8), the item flags of the static
// list 0x0084bc2a are set from the mask 1 (0x0053e4db), the flag byte [0x0082fd49] is cleared and
// [0x00824649] set to 1. eax = 1 and esi = 0x0084bc2a stay; the flags are those the item walk leaves.
// FUNCTION: GP4 0x0052d8c1
void __cdecl FUN_0052d8c1(gp4::Regs* r) {
    uint16_t* binding = GP4_ARRAY(uint16_t, 0x007ab55a);
    for (uint32_t i = 0u; i < 6u; ++i) {
        binding[i] = 0u;
    }
    const uint32_t bindingEnd = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(binding)) + 12u;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = bindingEnd;
    c.ecx = 0u;
    c.eax = r->eax & 0xffff0000u;
    gp4::call_regs(0x0052d752, c);
    gp4::call_regs(0x0052d6d8, c);

    c.eax = 1u;
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x0084bc2a)));
    gp4::call_regs(0x0053e4db, c);

    GP4_GLOBAL(uint8_t, 0x0082fd49) = 0u;
    GP4_GLOBAL(uint8_t, 0x00824649) = 1u;

    r->eax = 1u;
    r->ecx = 0u;
    r->esi = c.esi;
    r->edi = bindingEnd;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052d8c1, FUN_0052d8c1, "regs(eax:u32) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keyq_008245c0+list_0052dba4")

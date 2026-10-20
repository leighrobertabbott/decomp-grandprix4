#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the handler of every item of the current menu [0x82fd34] (list linked through +0): the flag
// byte +0x1f is masked with 0x3f and, unless bit 7 of +0x1e is set, the handler pointer at +0x1a is
// called with esi = item, ebx = the dword +0xe and edx = 1 (esi is pushed/popped around the call, so the
// walk continues from the item). The walk ends when a link is null: esi = 0 and the flags are those of
// `or esi, esi`; every other register is whatever the handlers left.
// FUNCTION: GP4 0x0053cb41
void __cdecl FUN_0053cb41(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint32_t node = GP4_GLOBAL(uint32_t, 0x0082fd34);
    for (;;) {
        node = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(node));
        if (node == 0u) {
            break;
        }
        uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(node));
        rec[0x1f] = static_cast<uint8_t>(rec[0x1f] & 0x3fu);
        const uint32_t held = rec[0x1e] & 0x80u;
        if (held == 0u) {
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(held));
            c.esi = node;
            c.ebx = *reinterpret_cast<const uint32_t*>(rec + 0xe);
            c.edx = 1u;
            gp4::call_regs(*reinterpret_cast<const uint32_t*>(rec + 0x1a), c);
        }
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = 0u;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x0053cb41, FUN_0053cb41, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+sel_0053d8c5")

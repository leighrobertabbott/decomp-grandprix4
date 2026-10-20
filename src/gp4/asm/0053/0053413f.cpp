#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores esi into entry eax of the dword table at 0x00828214. Leaves eax = the entry address and the
// flags of the final `add eax, 0x828214`.
// FUNCTION: GP4 0x0053413f
void __cdecl FUN_0053413f(gp4::Regs* r) {
    const uint32_t offset = r->eax << 2;
    const uint32_t address = offset + 0x00828214u;
    *reinterpret_cast<uint32_t*>(address) = r->esi;
    r->eax = address;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, 0x00828214u));
}
GP4_IMPL(0x0053413f, FUN_0053413f, "regs(eax:u32[0..15], esi:u32) -> (eax, cf, pf, af, zf, sf, of)")

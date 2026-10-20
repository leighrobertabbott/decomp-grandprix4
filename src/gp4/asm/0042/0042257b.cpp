#include <gp4/gp4.h>

// FUNCTION: GP4 0x0042257b
void __cdecl FUN_0042257b(gp4::Regs* r) {
    // For each of [0x62a05c] entries of the dword table [0x62a048] whose bits intersect the mask
    // [0x62a054]: entries with any of 0x14280 set go to 0x4223ed (eax = index, edx = entry);
    // the others go to 0x422397 (eax = index) when their signed byte at 0x60932c[index] is >= 4.
    // The loop ends on `cmp ebx, [0x62a05c]` equal, so the arithmetic flags are always ZF|PF.
    const uint32_t* table = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062a048)));
    const uint32_t mask = GP4_GLOBAL(uint32_t, 0x0062a054);
    const int8_t* levels = GP4_ARRAY(const int8_t, 0x0060932c);
    gp4::Regs c;
    c.eax = r->eax;
    c.ebx = 0;
    c.ecx = r->ecx;
    c.edx = r->edx;
    c.esi = reinterpret_cast<uintptr_t>(table);
    c.edi = mask;
    c.ebp = r->ebp;
    c.eflags = r->eflags;
    c.st_in = 0;
    c.st_out = 0;
    uint32_t index = 0;
    do {
        const uint32_t entry = table[index];
        if ((entry & mask) != 0u) {
            c.ebx = index;
            if ((entry & 0x14280u) == 0u) {
                if (levels[index] >= 4) {
                    c.eax = index;
                    gp4::call_regs(0x00422397, c);
                }
            } else {
                c.edx = entry;
                c.eax = index;
                gp4::call_regs(0x004223ed, c);
            }
        }
        ++index;
    } while (index != GP4_GLOBAL(uint32_t, 0x0062a05c));
    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->ebx = index;
    r->esi = c.esi;
    r->edi = c.edi;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0042257b, FUN_0042257b, "regs(eax:u32, edx:u32) -> (eax, ebx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=state_0042257b")

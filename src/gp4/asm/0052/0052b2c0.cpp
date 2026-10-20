#include <gp4/gp4.h>
#include <gp4/flags.h>

// Keyword table scan: edx points at 8-byte entries (+0 a nonzero value, +4 a pointer to a
// NUL-terminated keyword); the text is at esi. Each keyword is tried with the matcher 0x0052b2db
// (edi = keyword). On the first match eax = the matching entry, CF = 0 (esi advanced past the
// keyword by the matcher). Otherwise edx steps by 8 until an entry whose +0 is zero, then CF = 1
// with the flags of that last compare. edi is pushed and popped (unchanged on return).
// FUNCTION: GP4 0x0052b2c0
void __cdecl FUN_0052b2c0(gp4::Regs* r) {
    uint32_t entry = r->edx;
    gp4::Regs c = *r;
    uint32_t flags;
    for (;;) {
        c.edx = entry;
        c.edi = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(entry + 4u));
        gp4::call_regs(0x0052b2db, c);
        if ((c.eflags & 1u) == 0u) {
            r->eax = entry;
            flags = c.eflags & ~1u;
            break;
        }
        entry += 8u;
        const uint32_t value = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(entry));
        if (value == 0u) {
            flags = gp4::flags::sub32(value, 0u) | 1u;
            break;
        }
    }
    r->edx = entry;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052b2c0, FUN_0052b2c0, "regs(eax:u32, edx:ptr[0x30]:layout=kwtab_0052b2c0, esi:ptr[0x40]:u8{0x61,0x62,0x63,0,0x20,0x7b}) -> (eax, edx, esi, cf, pf, af, zf, sf, of)")

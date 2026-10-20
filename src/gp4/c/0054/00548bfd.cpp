#include <gp4/gp4.h>
#include <gp4/flags.h>

// Compacts the 22 per-driver flag bytes at 0x007c2a48 into the list at 0x00831a69: only the bytes that
// have a bit in the mask bh are kept, in order, and the rest of the 22 slots are zero-filled; the number
// kept goes to the dword 0x00831a7f. The mask is 0x3f (the number bits), or 0xc0 (the flag bits) when the
// option byte [0x007ad8b2] is 0xff and more than one driver is flagged (0x0052ceef: singles + pairs > 1).
// Exit registers: eax = the old eax with ax replaced by (last byte & mask) << 8 (al = 0), ebx keeps all but
// bh = the mask, ecx = 0, edx = the count kept, esi / edi one past the source / destination lists. The
// flags are those of `sub ecx, edx` (0x16 - count).
// FUNCTION: GP4 0x00548bfd
void __cdecl FUN_00548bfd(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = 0x007c2a48u;
    c.edi = 0x00831a69u;
    c.ecx = 0x16u;

    uint32_t mask = 0x3fu;
    uint32_t eax = r->eax;
    if (GP4_GLOBAL(uint8_t, 0x007ad8b2) == 0xffu) {
        gp4::call_regs(0x0052ceef, c);
        eax = c.eax + c.edx;
        if (eax > 1u) {
            mask = 0xc0u;
        }
    }

    const uint8_t* src = GP4_ARRAY(const uint8_t, 0x007c2a48);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x00831a69);
    uint32_t count = 0u;
    uint32_t kept = 0u;
    for (uint32_t n = 0u; n < 0x16u; ++n) {
        const uint32_t value = src[n];
        kept = value & mask;
        if (kept != 0u) {
            *dst++ = static_cast<uint8_t>(value);
            ++count;
        }
    }
    GP4_GLOBAL(uint32_t, 0x00831a7f) = count;
    for (uint32_t left = 0x16u - count; left != 0u; --left) {
        *dst++ = 0u;
    }

    r->eax = (eax & 0xffff0000u) | (kept << 8);
    r->ebx = (r->ebx & 0xffff00ffu) | (mask << 8);
    r->ecx = 0u;
    r->edx = count;
    r->esi = 0x007c2a48u + 0x16u;
    r->edi = 0x00831a69u + 0x16u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x16u, count));
}
GP4_IMPL(0x00548bfd, FUN_00548bfd, "regs(eax:u32, ebx:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=neg22_00520023")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies up to 0x101 bytes from the string at esi to edi, stopping before storing a double quote
// (0x22); the quote itself is consumed (esi is past it). Leaves al = the last byte read, esi/edi
// advanced, ecx restored, and the flags of the last compare (`cmp al, 0x22` when it ended on the
// quote, `cmp al, 0x20` when it ran out of count).
// FUNCTION: GP4 0x00514734
void __cdecl FUN_00514734(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint8_t al;
    uint32_t flags;
    uint32_t count = 0x101u;
    do {
        al = *src++;
        flags = gp4::flags::sub8(al, 0x22u);
        if (al == 0x22u) {
            break;
        }
        flags = gp4::flags::sub8(al, 0x20u);
        *dst++ = al;
        --count;
    } while (count != 0u);

    r->eax = (r->eax & 0xffffff00u) | al;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00514734, FUN_00514734, "regs(eax:u32, esi:ptr[0x108]:u8{0x41,0x1f,0x20,0x7f,0x22}, edi:ptr[0x108]:bytes) -> (eax, esi, edi, cf, pf, af, zf, sf, of)")

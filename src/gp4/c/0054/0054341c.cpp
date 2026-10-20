#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the option bytes of the set selected by the index [0x00831357] (four bytes at 0x0083135b +
// 4 * index) over the four current option bytes at 0x00831377, skipping every 0xff byte (an unchanged
// option). The loop leaves al = the last byte read, ecx = 0, esi/edi advanced by four; the flags are
// those of the last `inc edi` (CF kept from the last `cmp al, 0xff`).
// FUNCTION: GP4 0x0054341c
void __cdecl FUN_0054341c(gp4::Regs* r) {
    uint32_t esi = (GP4_GLOBAL(uint32_t, 0x00831357) << 2) + 0x0083135bu;
    uint32_t edi = 0x00831377u;
    uint32_t al = 0u;
    uint32_t flags = r->eflags;
    for (uint32_t n = 4u; n != 0u; --n) {
        al = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(esi));
        flags = gp4::flags::sub8(al, 0xffu);
        if (al != 0xffu) {
            *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(edi)) = static_cast<uint8_t>(al);
        }
        flags = gp4::flags::inc32(esi, flags);
        ++esi;
        flags = gp4::flags::inc32(edi, flags);
        ++edi;
    }
    r->eax = (r->eax & 0xffffff00u) | al;
    r->ecx = 0u;
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054341c, FUN_0054341c, "regs(eax:u32) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=preset_0054341c")

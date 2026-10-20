#include <gp4/gp4.h>
#include <gp4/flags.h>

// `lodsb`: al = *esi++. A zero byte copies the (unaligned) dword at 0x00833cc5 to 0x00833ccd and
// leaves it in eax; any other byte leaves eax with only its low byte replaced. The flags are those
// of `or al, al` (the jumps, mov and nop do not change them).
// FUNCTION: GP4 0x0054d268
void __cdecl FUN_0054d268(gp4::Regs* r) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t al = *p;
    r->esi = r->esi + 1u;
    r->eax = (r->eax & 0xffffff00u) | al;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(al));
    if (al == 0) {
        const uint32_t v = GP4_GLOBAL(uint32_t, 0x00833cc5);
        GP4_GLOBAL(uint32_t, 0x00833ccd) = v;
        r->eax = v;
    }
}
GP4_IMPL(0x0054d268, FUN_0054d268, "regs(eax:u32, esi:ptr[0x40]:u8{0,0,1,0x41}) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

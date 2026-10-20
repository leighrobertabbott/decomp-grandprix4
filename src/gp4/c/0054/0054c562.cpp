#include <gp4/gp4.h>
#include <gp4/flags.h>

// Skips to the end of the current text line, then over the line break: esi advances to the first
// '\r' or NUL, then over any run of '\r' / '\n' characters. The routine exits through the
// `cmp byte [esi], 0xa` of the second loop, so the flags are always those of that compare.
// FUNCTION: GP4 0x0054c562
void __cdecl FUN_0054c562(gp4::Regs* r) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    while (*p != 0x0du && *p != 0u) {
        p++;
    }
    while (*p == 0x0du || *p == 0x0au) {
        p++;
    }
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(*p, 0x0au));
}
GP4_IMPL(0x0054c562, FUN_0054c562, "regs(esi:ptr[0x40]:u8{0x0d,0x0a,0x41,0x42,0x20,0x0d,0x0a,0x00}) -> (esi, cf, pf, af, zf, sf, of)")

#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) const uint32_t* AddressOf(uint32_t* table) { return table; }

// ORs the eight dwords of the 32-byte "wanted" bitmap at 0x008245a0. eax, ebx and ecx are saved and restored;
// the only visible effect is the flags of the final `or eax, eax` (ZF set when the bitmap is empty).
// FUNCTION: GP4 0x0052d6ed
void __cdecl FUN_0052d6ed(gp4::Regs* r) {
    const uint32_t* bitmap = AddressOf(GP4_ARRAY(uint32_t, 0x008245a0));
    uint32_t bits = 0u;
    for (uint32_t i = 0u; i < 8u; ++i) {
        bits |= bitmap[i];
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(bits));
}
GP4_IMPL(0x0052d6ed, FUN_0052d6ed, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bitmap_00510e2d")

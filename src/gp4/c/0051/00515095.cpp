#include <gp4/gp4.h>
#include <gp4/flags.h>

// Picks one of two 16-bit selector words (by the sign of the byte at 0x007c2ea9) and compares it
// with the cached value at 0x00893e0c. A change updates the cache and the low byte copy at
// 0x0086608c (eax = low byte, flags of that `and`); an unchanged value clears bit 2 of the
// esi record byte +0x88 and sets 0x00954169 = 0xff (eax = the selector, flags of the byte `and`).
// FUNCTION: GP4 0x00515095
void __cdecl FUN_00515095(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(r->esi);
    uint32_t value = GP4_GLOBAL(uint16_t, 0x0066d89c);
    if ((int8_t)GP4_GLOBAL(uint8_t, 0x007c2ea9) >= 0) {
        value = GP4_GLOBAL(uint16_t, 0x0066d86a);
    }
    uint32_t flags;
    if (value != GP4_GLOBAL(uint32_t, 0x00893e0c)) {
        GP4_GLOBAL(uint32_t, 0x00893e0c) = value;
        value &= 0xffu;
        GP4_GLOBAL(uint32_t, 0x0086608c) = value;
        flags = gp4::flags::logic32(value);
    } else {
        const uint8_t updated = (uint8_t)(GP4_FIELD(uint8_t, record, 0x88) & 0xfbu);
        GP4_FIELD(uint8_t, record, 0x88) = updated;
        GP4_GLOBAL(uint8_t, 0x00954169) = 0xff;
        flags = gp4::flags::logic8(updated);
    }
    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00515095, FUN_00515095, "regs(esi:ptr[0x90]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

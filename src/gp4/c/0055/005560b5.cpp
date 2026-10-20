#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears the fifty words at 0x0066fd70. ebx counts the byte offset in steps of 2 and ends at 0x64; the
// flags are those of the final `cmp ebx, 0x64` (equal).
// FUNCTION: GP4 0x005560b5
void __cdecl FUN_005560b5(gp4::Regs* r) {
    uint8_t* const base = GP4_ARRAY(uint8_t, 0x0066fd70);
    uint32_t offset = 0u;
    do {
        *reinterpret_cast<uint16_t*>(base + offset) = 0u;
        offset += 2u;
    } while (offset != 0x64u);
    r->ebx = offset;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(offset, 0x64u));
}
GP4_IMPL(0x005560b5, FUN_005560b5, "regs() -> (ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

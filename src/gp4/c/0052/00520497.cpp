#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears the 0x9c bytes 0x0066d814..0x0066d8af one byte at a time. esi ends at the end address and
// the flags are those of the final `cmp esi, 0x66d8b0` (equal).
// FUNCTION: GP4 0x00520497
void __cdecl FUN_00520497(gp4::Regs* r) {
    uint8_t* cursor = GP4_ARRAY(uint8_t, 0x0066d814);
    uint8_t* const end = GP4_ARRAY(uint8_t, 0x0066d8b0);
    do {
        *cursor = 0;
        ++cursor;
    } while (cursor != end);
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cursor));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x0066d8b0u, 0x0066d8b0u));
}
GP4_IMPL(0x00520497, FUN_00520497, "regs() -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

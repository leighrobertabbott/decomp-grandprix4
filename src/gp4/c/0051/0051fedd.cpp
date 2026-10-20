#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears the 0xa8 bytes 0x00852d2c..0x00852dd3 one byte at a time. ebx ends at the end address and
// the flags are those of the final `cmp ebx, 0x852dd4` (equal).
// FUNCTION: GP4 0x0051fedd
void __cdecl FUN_0051fedd(gp4::Regs* r) {
    uint8_t* cursor = GP4_ARRAY(uint8_t, 0x00852d2c);
    uint8_t* const end = GP4_ARRAY(uint8_t, 0x00852dd4);
    do {
        *cursor = 0;
        ++cursor;
    } while (cursor != end);
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cursor));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x00852dd4u, 0x00852dd4u));
}
GP4_IMPL(0x0051fedd, FUN_0051fedd, "regs() -> (ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

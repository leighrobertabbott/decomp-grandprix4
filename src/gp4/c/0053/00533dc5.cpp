#include <gp4/gp4.h>
#include <gp4/flags.h>

// Removes the byte at index [0x008280ed] from the buffer at 0x0082811f by shifting the following
// bytes down one (the count is [0x008280f5] - [0x008280ed]; 0x0054e232 is called when it is zero),
// then decrements the length [0x008280f5], re-read after the copy. All registers are restored
// (pushal/popal); the flags are those of the final `dec [0x008280f5]` with CF from
// `add esi, 0x82811f` (inc/dec keep CF; rep movsb writes no flag).

// FUNCTION: GP4 0x00533dc5
void __cdecl FUN_00533dc5(gp4::Regs* r) {
    const uint32_t pos = GP4_GLOBAL(uint32_t, 0x008280ed);
    uint32_t count = GP4_GLOBAL(uint32_t, 0x008280f5) - pos;
    if (count == 0u) {
        gp4::cdecl_<0x0054e232, void>();
    }
    const uint32_t dst = pos + 0x0082811fu;
    uint8_t* d = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(dst));
    const uint8_t* s = d + 1;
    for (; count != 0u; --count) {
        *d++ = *s++;
    }
    const uint32_t add_flags = gp4::flags::add32(pos, 0x0082811fu);
    const uint32_t inc_flags = gp4::flags::inc32(dst, add_flags);
    const uint32_t length = GP4_GLOBAL(uint32_t, 0x008280f5);
    GP4_GLOBAL(uint32_t, 0x008280f5) = length - 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(length, inc_flags));
}
GP4_IMPL(0x00533dc5, FUN_00533dc5, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_00533dc5+span_00533dc5")

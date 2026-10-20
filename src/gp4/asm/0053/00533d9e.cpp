#include <gp4/gp4.h>
#include <gp4/flags.h>

// Inserts the byte al at index [0x008280ed] of the text buffer 0x0082811f (the mirror of the
// deletion 0x00533dc5): the bytes from the cursor up to and including the last one at index
// [0x008280f5] move up one place (`std; rep movsb`, copied from the top down), the byte goes to
// the cursor and the length [0x008280f5] grows by one. pushal/popal keeps every register; the
// flags are those of the final `inc dword [0x8280f5]` with CF from `add esi, 0x82811f`.
// FUNCTION: GP4 0x00533d9e
void __cdecl FUN_00533d9e(gp4::Regs* r) {
    const uint32_t value = r->eax & 0xffu;
    const uint32_t length = GP4_GLOBAL(uint32_t, 0x008280f5);
    uint32_t count = length - GP4_GLOBAL(uint32_t, 0x008280ed) + 1u;
    const uint32_t top = length + 0x0082811fu;
    const uint32_t add_flags = gp4::flags::add32(length, 0x0082811fu);
    uint8_t* src = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(top));
    uint8_t* dst = src + 1;
    for (; count != 0u; --count) {
        *dst-- = *src--;
    }
    *dst = static_cast<uint8_t>(value);
    GP4_GLOBAL(uint32_t, 0x008280f5) = length + 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(length, add_flags));
}
GP4_IMPL(0x00533d9e, FUN_00533d9e, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=layout=state_00533dc5+span_00533dc5")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0052d51c
// Fills four three-dword tables from base = [0x823690] * 6 + 0x1e9 (eax, ecx and edi are pushed and
// popped, so every GPR is unchanged):
//   0x81e3e8 = base, base+1, base+2        0x81e3d8 = base+2, base+1, base
//   0x81e408 = base+3, base+4, base+5      0x81e3f8 = base+5, base+4, base+3
// The flags are those of the last `dec ecx` (CF kept from the last `add edi, 4`).
void __cdecl FUN_0052d51c(gp4::Regs* r) {
    uint32_t value = GP4_GLOBAL(uint32_t, 0x00823690) * 6u + 0x1e9u;

    uint32_t* cursor = GP4_ARRAY(uint32_t, 0x0081e3e8);
    for (uint32_t n = 3u; n != 0u; --n) {
        *cursor = value;
        ++cursor;
        ++value;
    }

    cursor = GP4_ARRAY(uint32_t, 0x0081e3d8);
    for (uint32_t n = 3u; n != 0u; --n) {
        --value;
        *cursor = value;
        ++cursor;
    }

    value += 3u;
    cursor = GP4_ARRAY(uint32_t, 0x0081e408);
    for (uint32_t n = 3u; n != 0u; --n) {
        *cursor = value;
        ++cursor;
        ++value;
    }

    cursor = GP4_ARRAY(uint32_t, 0x0081e3f8);
    uint32_t flags = 0u;
    for (uint32_t n = 3u; n != 0u; --n) {
        --value;
        *cursor = value;
        flags = gp4::flags::add32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cursor)), 4u);
        ++cursor;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, flags));
}
GP4_IMPL(0x0052d51c, FUN_0052d51c, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

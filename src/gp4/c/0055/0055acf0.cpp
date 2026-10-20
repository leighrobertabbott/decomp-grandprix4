#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears two parallel tables of 16 words, 0x00874b72.. and the one 0x20 bytes after it, and the word
// [0x008705ea]. The register loop leaves ebx at the end of the first table (0x00874b92), ecx = 0 and
// the flags of its last `add ebx, 2`.
// FUNCTION: GP4 0x0055acf0
void __cdecl FUN_0055acf0(gp4::Regs* r) {
    uint16_t* cell = GP4_ARRAY(uint16_t, 0x00874b72);
    for (uint32_t i = 0u; i < 16u; ++i) {
        cell[i] = 0u;
        cell[i + 16u] = 0u;
    }
    GP4_GLOBAL(uint16_t, 0x008705ea) = 0u;

    const uint32_t end = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cell + 16));
    r->ebx = end;
    r->ecx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(end - 2u, 2u));
}
GP4_IMPL(0x0055acf0, FUN_0055acf0, "regs() -> (ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

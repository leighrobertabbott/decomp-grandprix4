#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears the 16-word table 0x00874b72 except, when bit 2 of the word (0x008705ea & 0x008705ec) is
// set, the entries 2 (bit 0x400 of the word 0x008705ec) and 3 (bit 0x800) which are kept; then
// replaces bits 2..5 of the dword 0x00870630 by bits 10..13 of 0x008705ec, stores the result in
// 0x008705b8 and clears 0x008705d4. Registers end as ebx = 0x00874b92 (past the table),
// edx = 0xffffffc3, eax = the new bits (word 0x008705ec >> 8) & 0x3c, ecx = the new 0x00870630;
// the flags are those of the final `or ecx, eax`.
// FUNCTION: GP4 0x0055ad14
void __cdecl FUN_0055ad14(gp4::Regs* r) {
    const uint16_t mask = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x008705ea) & GP4_GLOBAL(uint16_t, 0x008705ec));
    uint16_t* cell = GP4_ARRAY(uint16_t, 0x00874b72);
    for (uint32_t i = 0u; i != 0x10u; ++i) {
        const bool keep = (mask & 4u) != 0u &&
                          ((i == 2u && (GP4_GLOBAL(uint8_t, 0x008705ed) & 4u) != 0u) ||
                           (i == 3u && (GP4_GLOBAL(uint8_t, 0x008705ed) & 8u) != 0u));
        if (!keep) {
            cell[i] = 0u;
        }
    }

    const uint32_t bits = (static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x008705ec)) >> 8) & 0x3cu;
    const uint32_t value = (GP4_GLOBAL(uint32_t, 0x00870630) & 0xffffffc3u) | bits;
    GP4_GLOBAL(uint32_t, 0x00870630) = value;
    GP4_GLOBAL(uint32_t, 0x008705b8) = value;
    GP4_GLOBAL(uint32_t, 0x008705d4) = 0u;

    r->eax = bits;
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cell + 16));
    r->ecx = value;
    r->edx = 0xffffffc3u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
}
GP4_IMPL(0x0055ad14, FUN_0055ad14, "regs() -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

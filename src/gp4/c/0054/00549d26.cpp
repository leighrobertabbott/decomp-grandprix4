#include <gp4/gp4.h>
#include <gp4/flags.h>

// Fills the 800x600x16-bit surface whose address is the dword 0x00822258 (0x3a980 dwords) with the
// colour in eax repeated in both halves. When the byte 0x00ae6740 is zero the colour is first widened
// from 5-5-5 to 5-6-5 (`(c & 0x1f) | ((c & 0x7fe0) << 1)`). pushal/popal keep every register; the
// flags are those of the last `or eax, ebx` (rep stosd writes none).
// FUNCTION: GP4 0x00549d26
void __cdecl FUN_00549d26(gp4::Regs* r) {
    uint32_t colour = r->eax;
    if (GP4_GLOBAL(uint8_t, 0x00ae6740) == 0u) {
        colour = (colour & 0x1fu) | ((colour & 0x7fe0u) << 1);
    }
    const uint32_t fill = colour | (colour << 16);
    uint32_t* dst = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00822258)));
    for (uint32_t i = 0; i != 0x3a980u; ++i) {
        dst[i] = fill;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(fill));
}
GP4_IMPL(0x00549d26, FUN_00549d26, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); cases=40; globals=fuzz:i32+layout=surfcopy_00549d55; wall=3600")

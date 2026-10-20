#include <gp4/gp4.h>
#include <gp4/flags.h>

// Mode byte 0x0086857c == 1: the word at esi is read and its low byte stored at 0x0086856e (ax is
// loaded, so the upper half of eax is kept); every mode then skips the word (esi += 2). The flags
// are those of the final `add esi, 2` (the `cmp byte [0x86857c], 1` is overwritten).
// FUNCTION: GP4 0x00557464
void __cdecl FUN_00557464(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 1u) {
        const uint16_t ax = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(esi));
        GP4_GLOBAL(uint8_t, 0x0086856e) = static_cast<uint8_t>(ax);
        r->eax = (r->eax & 0xffff0000u) | ax;
    }
    r->esi = esi + 2u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 2u));
}
GP4_IMPL(0x00557464, FUN_00557464, "regs(eax:u32, esi:ptr[0x10]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")

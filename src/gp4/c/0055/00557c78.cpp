#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reads the six words at esi when the byte at 0x0086857c is 1 and stores them as three
// "word pairs swapped into a dword" values (first word in the high half) at 0x00852d0c,
// 0x00852d10 and 0x00852d14; eax is left holding the last of them. esi advances by 0xc in
// both cases and the flags are those of that `add esi, 0xc`.
// FUNCTION: GP4 0x00557c78
void __cdecl FUN_00557c78(gp4::Regs* r) {
    const uint32_t esi_in = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 1u) {
        const uint16_t* words = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(esi_in));
        const uint32_t first = ((uint32_t)words[0] << 16) | words[1];
        GP4_GLOBAL(uint32_t, 0x00852d0c) = first;
        const uint32_t second = ((uint32_t)words[2] << 16) | words[3];
        GP4_GLOBAL(uint32_t, 0x00852d10) = second;
        const uint32_t third = ((uint32_t)words[4] << 16) | words[5];
        GP4_GLOBAL(uint32_t, 0x00852d14) = third;
        r->eax = third;
    }
    r->esi = esi_in + 0xcu;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi_in, 0xcu));
}
GP4_IMPL(0x00557c78, FUN_00557c78, "regs(eax:u32, esi:ptr[0x14]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

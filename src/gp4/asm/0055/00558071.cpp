#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears bits 8 and 9 of the word at 0x008705bc, then reads a word from esi (advancing esi by 2):
// bit 15 of it sets bit 9 and bit 14 sets bit 8 of that word again.
// Leaves ax = the word read (upper half of eax kept), esi += 2 and the flags of the last
// `test ax, 0x4000` (bit 14 clear) or of the final `or word [0x8705bc], 0x100` (bit 14 set).

// FUNCTION: GP4 0x00558071
void __cdecl FUN_00558071(gp4::Regs* r) {
    uint16_t state = GP4_GLOBAL(uint16_t, 0x008705bc);
    state = static_cast<uint16_t>(state & 0xfdffu);
    state = static_cast<uint16_t>(state & 0xfeffu);

    const uint16_t word = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags = 0;
    if ((word & 0x8000u) != 0) {
        state = static_cast<uint16_t>(state | 0x0200u);
    }
    flags = gp4::flags::logic16(word & 0x4000u);
    if ((word & 0x4000u) != 0) {
        state = static_cast<uint16_t>(state | 0x0100u);
        flags = gp4::flags::logic16(state);
    }
    GP4_GLOBAL(uint16_t, 0x008705bc) = state;

    r->eax = (r->eax & 0xffff0000u) | word;
    r->esi += 2;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00558071, FUN_00558071, "regs(eax:u32, esi:ptr[0x40]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

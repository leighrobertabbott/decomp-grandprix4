#include <gp4/gp4.h>
#include <gp4/flags.h>

// `bt dword [0x7ad848], index` with a signed bit index: the dword at 0x7ad848 + 4 * (index >> 5),
// bit (index & 31).
static __declspec(noinline) uint32_t BitmapBit(uint32_t index) {
    const int32_t signedIndex = static_cast<int32_t>(index);
    const uint32_t* words = GP4_ARRAY(uint32_t, 0x007ad848);
    return (words[signedIndex >> 5] >> (index & 31u)) & 1u;
}

// `push eax ; call 0x00530774 ; sub eax, 0 ; bt [0x7ad848], eax ; pop eax ; ret`: tests the bit of
// the bitmap at 0x7ad848 selected by the current index [0x7ad87c] (read through 0x530774). CF is the
// bit, the other flags are those of `sub eax, 0`. eax is saved and restored.
// FUNCTION: GP4 0x00531847
void __cdecl FUN_00531847(gp4::Regs* r) {
    const uint32_t index = gp4::cdecl_<0x00530774, uint32_t>();
    const uint32_t flags = (gp4::flags::sub32(index, 0u) & ~gp4::flags::CF) | BitmapBit(index);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00531847, FUN_00531847, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cb_00531648+bits_007ad844")

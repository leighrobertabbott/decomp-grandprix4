#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reads six words at esi when the mode byte 0x0086857c is 1 and stores them as a rectangle
// description: words 0 and 1 with bit 0 cleared (to 0x00852c3c and 0x00852c40, zero-extended),
// words 2:3 packed as (w2 << 16) | w3 (to 0x00852d18) and words 4:5 packed as (w4 << 16) | w5
// (to 0x00852d1c). eax ends as the last packed dword in that case and is untouched otherwise. esi
// advances by 0xc in both cases and the flags are those of that `add esi, 0xc`.
// FUNCTION: GP4 0x00557a80
void __cdecl FUN_00557a80(gp4::Regs* r) {
    const uint32_t esi_in = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 1u) {
        const uint16_t* words = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(esi_in));
        GP4_GLOBAL(uint32_t, 0x00852c3c) = static_cast<uint32_t>(words[0] & 0xfffeu);
        GP4_GLOBAL(uint32_t, 0x00852c40) = static_cast<uint32_t>(words[1] & 0xfffeu);
        GP4_GLOBAL(uint32_t, 0x00852d18) = (static_cast<uint32_t>(words[2]) << 16) | words[3];
        const uint32_t last = (static_cast<uint32_t>(words[4]) << 16) | words[5];
        GP4_GLOBAL(uint32_t, 0x00852d1c) = last;
        r->eax = last;
    }
    r->esi = esi_in + 0xcu;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi_in, 0xcu));
}
GP4_IMPL(0x00557a80, FUN_00557a80, "regs(eax:u32, esi:ptr[0x10]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

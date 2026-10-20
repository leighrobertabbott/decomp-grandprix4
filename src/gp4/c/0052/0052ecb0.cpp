#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes two bytes at edi (stosb twice, edi advances by 2): the lower (signed) of the words
// [0x007c2e9c] and [0x007c2e96], then the low byte of [0x007c2e96]. ax ends as [0x007c2e96] (the high
// half of eax is kept) and the flags are those of `cmp ax, [0x007c2e96]` with ax = [0x007c2e9c].
// FUNCTION: GP4 0x0052ecb0
void __cdecl FUN_0052ecb0(gp4::Regs* r) {
    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint16_t first = GP4_GLOBAL(uint16_t, 0x007c2e9c);
    const uint16_t second = GP4_GLOBAL(uint16_t, 0x007c2e96);
    uint16_t low = first;
    if (static_cast<int16_t>(first) > static_cast<int16_t>(second)) {
        low = GP4_GLOBAL(uint16_t, 0x007c2e96);
    }
    out[0] = static_cast<uint8_t>(low);
    const uint16_t last = GP4_GLOBAL(uint16_t, 0x007c2e96);
    out[1] = static_cast<uint8_t>(last);
    r->edi += 2u;
    r->eax = (r->eax & 0xffff0000u) | last;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub16(first, second));
}
GP4_IMPL(0x0052ecb0, FUN_0052ecb0, "regs(eax:u32, edi:ptr[0x40]:bytes) -> (eax, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

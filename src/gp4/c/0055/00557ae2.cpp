#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reads four words at esi when the mode byte 0x0086857c is 1 and stores them as a rectangle:
// x = word 0 (to 0x0088f07a and 0x00852c84), width = word 1 (to 0x0088f07e), x + width (16-bit
// sum, to 0x00852c86), then words 2 and 3 to 0x00852c7c and 0x00852c7e. ax ends as word 3 and dx as
// the 16-bit sum (upper halves untouched). esi advances by 8 in both cases and the flags are those
// of that `add esi, 8`.
// FUNCTION: GP4 0x00557ae2
void __cdecl FUN_00557ae2(gp4::Regs* r) {
    const uint32_t esi_in = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 1u) {
        const uint16_t* words = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(esi_in));
        const uint16_t x = words[0];
        const uint16_t width = words[1];
        GP4_GLOBAL(uint16_t, 0x0088f07a) = x;
        GP4_GLOBAL(uint16_t, 0x00852c84) = x;
        GP4_GLOBAL(uint16_t, 0x0088f07e) = width;
        const uint16_t sum = static_cast<uint16_t>(width + x);
        GP4_GLOBAL(uint16_t, 0x00852c86) = sum;
        GP4_GLOBAL(uint16_t, 0x00852c7c) = words[2];
        const uint16_t last = words[3];
        GP4_GLOBAL(uint16_t, 0x00852c7e) = last;
        r->eax = (r->eax & 0xffff0000u) | last;
        r->edx = (r->edx & 0xffff0000u) | sum;
    }
    r->esi = esi_in + 8u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi_in, 8u));
}
GP4_IMPL(0x00557ae2, FUN_00557ae2, "regs(eax:u32, edx:u32, esi:ptr[0x10]:bytes) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Twin of 0x00557ae2 for the second rectangle slot: when the mode byte 0x0086857c is 1 the four
// words at esi are stored as x = word 0 (to 0x0088f04e and 0x00852c88), width = word 1 (to
// 0x0088f052), x + width (16-bit sum, to 0x00852c8a), then words 2 and 3 to 0x00852c80 and
// 0x00852c82. ax ends as word 3 and dx as the 16-bit sum (upper halves untouched). esi advances by
// 8 in both cases and the flags are those of that `add esi, 8`.
// FUNCTION: GP4 0x00557b2b
void __cdecl FUN_00557b2b(gp4::Regs* r) {
    const uint32_t esi_in = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 1u) {
        const uint16_t* words = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(esi_in));
        const uint16_t x = words[0];
        const uint16_t width = words[1];
        GP4_GLOBAL(uint16_t, 0x0088f04e) = x;
        GP4_GLOBAL(uint16_t, 0x00852c88) = x;
        GP4_GLOBAL(uint16_t, 0x0088f052) = width;
        const uint16_t sum = static_cast<uint16_t>(width + x);
        GP4_GLOBAL(uint16_t, 0x00852c8a) = sum;
        GP4_GLOBAL(uint16_t, 0x00852c80) = words[2];
        const uint16_t last = words[3];
        GP4_GLOBAL(uint16_t, 0x00852c82) = last;
        r->eax = (r->eax & 0xffff0000u) | last;
        r->edx = (r->edx & 0xffff0000u) | sum;
    }
    r->esi = esi_in + 8u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi_in, 8u));
}
GP4_IMPL(0x00557b2b, FUN_00557b2b, "regs(eax:u32, edx:u32, esi:ptr[0x10]:bytes) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x005580a8
// Reads a slope from the stream at esi when the mode byte [0x0086857c] is 1: a word count dx and a
// word value. With a zero count the value becomes the new base [0x00870608] and the count and the
// slope [0x00870606]/[0x0087060e] are cleared. Otherwise the count is stored to [0x00870606] and the
// slope [0x0087060e] = (i16)(value - base) / count (idiv: the quotient fits, |n| <= 32768 and the
// count is 1..65535). esi advances by 4 in every case.
// Registers: mode != 1: only esi (flags of `add esi, 4`). Zero count: ax = the value, dx = 0
// (upper halves kept), flags of the second `add esi, 2`. Otherwise eax = quotient, edx = remainder,
// ebp = count, flags of `sub ax, [0x870608]` (idiv leaves them).
void __cdecl FUN_005580a8(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1u) {
        r->esi = esi + 4u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 4u));
        return;
    }

    const uint16_t* words = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(esi));
    const uint32_t count = words[0];
    const uint16_t value = words[1];
    r->esi = esi + 4u;

    if (count == 0u) {
        GP4_GLOBAL(uint16_t, 0x00870608) = value;
        GP4_GLOBAL(uint16_t, 0x00870606) = 0u;
        GP4_GLOBAL(uint16_t, 0x0087060e) = 0u;
        r->eax = (r->eax & 0xffff0000u) | value;
        r->edx = r->edx & 0xffff0000u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi + 2u, 2u));
        return;
    }

    GP4_GLOBAL(uint16_t, 0x00870606) = static_cast<uint16_t>(count);
    const uint16_t base = GP4_GLOBAL(uint16_t, 0x00870608);
    const uint32_t flags = gp4::flags::sub16(value, base);
    const int32_t numerator = static_cast<int16_t>(static_cast<uint16_t>(value - base));
    const int32_t divisor = static_cast<int32_t>(count);
    const int32_t quotient = numerator / divisor;
    const int32_t remainder = numerator % divisor;
    GP4_GLOBAL(uint16_t, 0x0087060e) = static_cast<uint16_t>(quotient);
    r->eax = static_cast<uint32_t>(quotient);
    r->edx = static_cast<uint32_t>(remainder);
    r->ebp = count;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005580a8, FUN_005580a8, "regs(eax:u32, edx:u32, esi:ptr[0x10]:u16{0,0,1,2,3,7,0x100,0x1234,0x7fff,0x8000,0xffff}) -> (eax, edx, ebp, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

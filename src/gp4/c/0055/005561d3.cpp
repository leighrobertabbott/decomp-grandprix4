#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets the two low bits of the byte at +0x48 of the 0x160-byte ring records in consecutive spans, each span ending where
// the next begins: [0x891500, 0x866ed0) cleared, [0x866ed0, 0x868518) set to 1, [0x868518, 0x86851c) set to 3,
// [0x86851c, 0x866ed4) set to 2, [0x866ed4, 0x866ef8) cleared. Every span is a `cmp edi,ebp; jb` loop, so a bound
// below the span start leaves it empty and the next span starts at that bound. pushal/popal restore every register;
// the flags are those of the final `cmp edi, ebp`.
static inline uint32_t Span(uint32_t start, uint32_t end, uint32_t bits) {
    uint32_t record = start;
    while (record < end) {
        uint8_t& flags = *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record + 0x48u));
        flags = static_cast<uint8_t>((flags & 0xfcu) | bits);
        record += 0x160u;
    }
    return record;
}

// FUNCTION: GP4 0x005561d3
void __cdecl FUN_005561d3(gp4::Regs* r) {
    const uint32_t b1 = GP4_GLOBAL(uint32_t, 0x00866ed0);
    const uint32_t b2 = GP4_GLOBAL(uint32_t, 0x00868518);
    const uint32_t b3 = GP4_GLOBAL(uint32_t, 0x0086851c);
    const uint32_t b4 = GP4_GLOBAL(uint32_t, 0x00866ed4);
    const uint32_t b5 = GP4_GLOBAL(uint32_t, 0x00866ef8);

    Span(GP4_GLOBAL(uint32_t, 0x00891500), b1, 0u);
    Span(b1, b2, 1u);
    Span(b2, b3, 3u);
    Span(b3, b4, 2u);
    const uint32_t last = Span(b4, b5, 0u);

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(last, b5));
}
GP4_IMPL(0x005561d3, FUN_005561d3, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=pit_0055a9f8+state_004154d5+bounds_00406a57+lane48_005561d3")

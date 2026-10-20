#include <gp4/gp4.h>
#include <gp4/flags.h>

// Digit-character tail inlined by the original after each `mov al, 0x20`: '-' becomes 0x15 and
// '0'..'9' are shifted down by 0x1a. Applied here to the constant space it leaves 0x20 unchanged.
static inline uint8_t MapDigitChar(uint8_t c) {
    if (c == 0x2du) {
        c = 0x15;
    }
    if (c >= 0x30u && c <= 0x39u) {
        c = static_cast<uint8_t>(c - 0x1au);
    }
    return c;
}

// pushal/popal: builds a display line through the R3 byte setters and text helpers. The registers
// each call leaves carry into the next (a local copy of the register block); the caller gets its
// registers back. The flags are those of the last flag writer: `test [0x852d48]`, then the
// `neg/add` clock compare, then the `and [0x852d64], ~1` when the timer expired.
// FUNCTION: GP4 0x0051739a
void __cdecl FUN_0051739a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = 0;
    gp4::call_regs(0x0051d187, c);
    c.eax = (c.eax & 0xffffff00u) | 2u;
    gp4::call_regs(0x0051d494, c);
    c.eax = (c.eax & 0xffffff00u) | 0x57u;
    gp4::call_regs(0x0051d48e, c);

    GP4_GLOBAL(uint8_t, 0x0088fd94) = 8;
    GP4_GLOBAL(uint8_t, 0x0088fd95) = 2;
    GP4_GLOBAL(uint8_t, 0x0088fd96) = 0;
    GP4_GLOBAL(uint8_t, 0x0088fd97) = 0;

    const uint32_t* table = GP4_ARRAY(uint32_t, 0x00891bd0);
    GP4_GLOBAL(uint32_t, 0x00a4b340) = 1;
    c.esi = table[0x28];
    if (GP4_GLOBAL(uint8_t, 0x0066d7a4) == 0x80u) {
        GP4_GLOBAL(uint32_t, 0x00a4b340) = 2;
        c.esi = table[0x2a];
    }

    c.eax = (c.eax & 0xffffff00u) | 0xa7u;
    gp4::call_regs(0x0051d48e, c);
    gp4::call_regs(0x00516927, c);
    gp4::call_regs(0x0051734d, c);

    const uint8_t* car = GP4_GLOBAL(uint8_t*, 0x00679d30);
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
    c.eax = (c.eax & 0xffffff00u) | car[0x7c];
    gp4::call_regs(0x0051f2af, c);
    GP4_GLOBAL(uint32_t, 0x00a4b344) = c.eax;
    c.esi = c.eax;
    gp4::call_regs(0x00516d94, c);

    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_GLOBAL(uint8_t*, 0x00679d30)));
    if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u) {
        uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi));
        *out++ = MapDigitChar(0x20u);
        *out++ = MapDigitChar(0x20u);
        GP4_GLOBAL(uint8_t, 0x0088fda0) = 0x4f;
        *out++ = 2;
        const uint8_t attr = GP4_GLOBAL(uint8_t, 0x0088fda0);
        *out++ = attr;
        c.eax = (c.eax & 0xffffff00u) | attr;
        c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out));
        gp4::call_regs(0x00408309, c);
        GP4_GLOBAL(uint32_t, 0x00a4b348) = c.eax;
        gp4::call_regs(0x0051730d, c);
    }

    gp4::call_regs(0x00516940, c);
    gp4::call_regs(0x005142b4, c);

    uint32_t& timer = GP4_GLOBAL(uint32_t, 0x00852d48);
    const uint32_t due = timer;
    uint32_t arith = gp4::flags::logic32(due);
    if (due != 0u) {
        const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
        const uint32_t negated = 0u - due;
        arith = gp4::flags::add32(negated, clock);
        if (static_cast<int32_t>(negated + clock) >= 0) {
            timer = 0;
            uint32_t& bits = GP4_GLOBAL(uint32_t, 0x00852d64);
            bits &= 0xfffffffeu;
            arith = gp4::flags::logic32(bits);
            GP4_GLOBAL(uint16_t, 0x00852d68) = 0x6000;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051739a, FUN_0051739a, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00679d30+bits_005174cb")

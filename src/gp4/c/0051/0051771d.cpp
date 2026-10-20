#include <gp4/gp4.h>
#include <gp4/flags.h>

// Character tail used for each digit: 0x2d ('-') becomes 0x15 and '0'..'9' are shifted down by 0x1a.
// `arith` receives the flags of the last compare/sub executed on that path.
static inline uint8_t MapDigitChar(uint8_t c, uint32_t* arith) {
    uint32_t f = gp4::flags::sub8(c, 0x2du);
    if (c == 0x2du) {
        c = 0x15;
    }
    f = gp4::flags::sub8(c, 0x30u);
    if (c >= 0x30u) {
        f = gp4::flags::sub8(c, 0x39u);
        if (c <= 0x39u) {
            f = gp4::flags::sub8(c, 0x1au);
            c = (uint8_t)(c - 0x1au);
        }
    }
    *arith = f;
    return c;
}

// pushal/popal text line: claims bitmap bit 9 (0x51d187), sets the colour bytes (0x51d494 2, 0x51d48e 0x20),
// the cursor bytes 0x88fd94..97 = 8, 1, 0, 2, writes the 5-byte header at 0x89152c (0x516927), then the
// driver name of the car at byte offset [0x66d8a6] in the car array 0x66fef0 (number byte +0x7c ->
// 0x51f2af, kept in 0xa4b360; copied by 0x516d94), the colour escape 2, 0x3f; with byte [0x7c2ea9] == 0x80
// also two spaces, the escape 2, 0x4f and the car's (+0x415 >> 1) + 1 (0x408309, kept in 0xa4b364) as two
// digits (0x51730d). Registers are restored; the flags are those of the last flag writer.
// FUNCTION: GP4 0x0051771d
void __cdecl FUN_0051771d(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = 9;
    gp4::call_regs(0x0051d187, c);
    c.eax = (c.eax & 0xffffff00u) | 2u;
    gp4::call_regs(0x0051d494, c);
    c.eax = (c.eax & 0xffffff00u) | 0x20u;
    gp4::call_regs(0x0051d48e, c);
    GP4_GLOBAL(uint8_t, 0x0088fd94) = 8;
    GP4_GLOBAL(uint8_t, 0x0088fd95) = 1;
    GP4_GLOBAL(uint8_t, 0x0088fd96) = 0;
    GP4_GLOBAL(uint8_t, 0x0088fd97) = 2;
    gp4::call_regs(0x00516927, c);

    const uint8_t* car = GP4_ARRAY(const uint8_t, 0x0066fef0) + GP4_GLOBAL(uint16_t, 0x0066d8a6);
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
    c.eax = (c.eax & 0xffffff00u) | car[0x7c];
    gp4::call_regs(0x0051f2af, c);
    GP4_GLOBAL(uint32_t, 0x00a4b360) = c.eax;
    c.esi = c.eax;
    gp4::call_regs(0x00516d94, c);
    c.eax = (c.eax & 0xffffff00u) | 0x3fu;
    gp4::call_regs(0x0051d48e, c);

    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi));
    *out++ = 2;
    *out++ = GP4_GLOBAL(uint8_t, 0x0088fda0);
    c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x0088fda0);
    c.esi = GP4_ARRAY(uint32_t, 0x00891bd0)[0x2f];
    c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out));
    gp4::cdecl_<0x0051734d, void>();

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t arith = gp4::flags::sub8(mode, 0x80u);
    if (mode == 0x80u) {
        out[0] = MapDigitChar(0x20, &arith);
        out[1] = MapDigitChar(0x20, &arith);
        GP4_GLOBAL(uint8_t, 0x0088fda0) = 0x4f;
        out[2] = 2;
        out[3] = GP4_GLOBAL(uint8_t, 0x0088fda0);
        c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out + 4));
        c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x0088fda0);
        c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(
            GP4_ARRAY(const uint8_t, 0x0066fef0) + GP4_GLOBAL(uint16_t, 0x0066d8a6)));
        c.eflags = gp4::flags::merge(c.eflags, arith);
        gp4::call_regs(0x00408309, c);
        GP4_GLOBAL(uint32_t, 0x00a4b364) = c.eax;
        gp4::call_regs(0x0051730d, c);
    } else {
        c.eflags = gp4::flags::merge(c.eflags, arith);
    }
    gp4::cdecl_<0x00516940, void>();
    gp4::call_regs(0x005142b4, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051771d, FUN_0051771d, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carno_0066ff6c")

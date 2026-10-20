#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text formatter for an unsigned/signed decimal number, run inside a pushal/popal pair. Inputs: eax = the value,
// bl = the pad character, bh = the mode bits (0x80 = the value is signed, 0x40 = right-justify to a width, 0x20 =
// a sign is to be printed), cl = the field width. Every character goes through 0x0054d3a1 (character in al).
// A negative signed value gets '-' (else '+') and sets 0x20. With 0x40 the field is padded to cl characters; when
// the pad character is 0x16 (the glyph of '0') the sign is written before the zeros, otherwise after the blanks.
// A number that does not fit the width (or has no room for its sign) prints one '?' per digit. Digits are written
// as digit + 0x16. popal restores every register, so only the flags escape: the last call to 0x0054d3a1 always
// ends the routine, so they are that callee's.
static void __declspec(noinline) put_char(gp4::Regs& c, uint32_t ch) {
    c.eax = ch;
    gp4::call_regs(0x0054d3a1, c);
}

// FUNCTION: GP4 0x0054d969
void __cdecl FUN_0054d969(gp4::Regs* r) {
    uint32_t value = r->eax;
    uint32_t mode = (r->ebx >> 8) & 0xffu;
    const uint32_t pad = r->ebx & 0xffu;
    const uint32_t width = r->ecx & 0xffu;
    uint32_t sign = 0x2bu;

    if ((mode & 0x80u) != 0u && static_cast<int32_t>(value) < 0) {
        sign = 0x2du;
        value = 0u - value;
        mode |= 0x20u;
    }

    uint32_t digits[12];
    int count = 0;
    do {
        digits[count++] = value % 10u;
        value /= 10u;
    } while (value != 0u);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    bool overflow = false;
    if ((mode & 0x40u) != 0u) {
        int32_t fill = static_cast<int32_t>(width) - count;
        if (fill < 0) {
            overflow = true;
        } else {
            if ((mode & 0x20u) != 0u) {
                fill -= 1;
                if (fill < 0) {
                    overflow = true;
                } else if (pad == 0x16u) {
                    put_char(c, sign);
                    mode &= 0xdfu;
                }
            }
            if (!overflow) {
                while (fill != 0) {
                    put_char(c, pad);
                    fill -= 1;
                }
            }
        }
    }

    if (overflow) {
        for (int i = 0; i < count; ++i) {
            put_char(c, 0x3fu);
        }
    } else {
        if ((mode & 0x20u) != 0u) {
            put_char(c, sign);
        }
        for (int i = count - 1; i >= 0; --i) {
            put_char(c, digits[i] + 0x16u);
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054d969, FUN_0054d969, "regs(eax:u32{0,5,9,10,99,100,12345,999999,1000000000,0x7fffffff,0x80000000,0xffffffff,0xfffffff6,0xffffff9c,0xfffffc18,0xfffed4c7}, ebx:u32{0x16,0x20,0x4016,0x4020,0x8016,0x8020,0xc016,0xc020,0xa016,0xa020,0xe016,0xe020,0x2016,0x2020,0x6016,0x6020}, ecx:u32[0..14]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=log_004fd730")

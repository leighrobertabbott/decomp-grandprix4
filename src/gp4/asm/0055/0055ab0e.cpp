#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

static inline uint32_t StreamWord(uint32_t s) {
    return GP4_FIELD(uint16_t, s, 0);
}
static inline uint32_t StreamSWord(uint32_t s) {
    return static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, s, 0)));
}

// Parses the word stream at esi into the 32-slot tables 0x6f2f6c..0x6f38ec (slot ebx = entry number),
// until a 0xffff word at an entry start. Each entry: three ring-record numbers (0x0055ac8a turns ax into
// the 0x160-byte record at 0x8943a0) interleaved with signed words; the first record gets its own +0x2c
// bits or-ed into its flag dwords (0x00559bc0), the third must lie at least 6 records before the pit
// end [0x866ef8] (int3). Word 0x86855c bit 0x800 adds five words (and bit 0x80 one more, 0 = copy of
// slot 0x6f316c), bit 0x40 three more. More than 33 entries or none at all is an assertion (int3).
// The entry count goes to 0x6f3ad8 (and ebx); esi ends after the terminator, eax holds 0xffff in ax,
// edi/edx are those of the last 0x0055ac8a; the flags are those of `cmp ebx, 0`.
// FUNCTION: GP4 0x0055ab0e
void __cdecl FUN_0055ab0e(gp4::Regs* r) {
    gp4::Regs c = *r;
    uint32_t s = r->esi;
    uint32_t eax = r->eax;
    uint32_t n = 0;
    for (;;) {
        eax = (eax & 0xffff0000u) | StreamWord(s);
        s += 2u;
        if ((eax & 0xffffu) == 0xffffu) {
            break;
        }
        c.eax = eax;
        c.ebx = n;
        c.esi = s;
        gp4::call_regs(0x0055ac8a, c);
        GP4_ARRAY(uint32_t, 0x006f2f6c)[n] = c.edi;
        c.eax = GP4_FIELD(uint32_t, c.edi, 0x2c);
        gp4::call_regs(0x00559bc0, c);

        eax = StreamSWord(s);
        s += 2u;
        GP4_ARRAY(uint32_t, 0x006f30ec)[n] = eax;
        eax = StreamSWord(s);
        s += 2u;
        GP4_ARRAY(uint32_t, 0x006f316c)[n] = eax;

        eax = (eax & 0xffff0000u) | StreamWord(s);
        s += 2u;
        c.eax = eax;
        c.esi = s;
        gp4::call_regs(0x0055ac8a, c);
        GP4_ARRAY(uint32_t, 0x006f306c)[n] = c.edi;
        GP4_ARRAY(uint32_t, 0x006f2fec)[n] = c.edi;

        eax = StreamSWord(s);
        s += 2u;
        GP4_ARRAY(uint32_t, 0x006f31ec)[n] = eax;

        eax = (eax & 0xffff0000u) | StreamWord(s);
        s += 2u;
        c.eax = eax;
        c.esi = s;
        gp4::call_regs(0x0055ac8a, c);
        if (!(c.edi < GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x840u)) {
            __debugbreak();
        }
        GP4_ARRAY(uint32_t, 0x006f326c)[n] = c.edi;

        eax = StreamSWord(s);
        s += 2u;
        GP4_ARRAY(uint32_t, 0x006f32ec)[n] = eax;
        eax = StreamSWord(s);
        s += 2u;
        GP4_ARRAY(uint32_t, 0x006f336c)[n] = eax;
        eax = StreamSWord(s);
        s += 2u;
        GP4_ARRAY(uint32_t, 0x006f33ec)[n] = eax;
        eax = StreamSWord(s);
        s += 2u;
        GP4_ARRAY(uint32_t, 0x006f346c)[n] = eax;

        if ((GP4_GLOBAL(uint16_t, 0x0086855c) & 0x800u) != 0u) {
            eax = StreamSWord(s);
            s += 2u;
            GP4_ARRAY(uint32_t, 0x006f34ec)[n] = eax;
            eax = StreamSWord(s);
            s += 2u;
            GP4_ARRAY(uint32_t, 0x006f356c)[n] = eax;
            eax = StreamSWord(s);
            s += 2u;
            GP4_ARRAY(uint32_t, 0x006f35ec)[n] = eax;
            eax = StreamSWord(s);
            s += 2u;
            GP4_ARRAY(uint32_t, 0x006f366c)[n] = eax;
            eax = StreamSWord(s);
            s += 2u;
            GP4_ARRAY(uint32_t, 0x006f36ec)[n] = eax;
            if ((GP4_GLOBAL(uint16_t, 0x0086855c) & 0x80u) != 0u) {
                eax = StreamSWord(s);
                s += 2u;
                if (eax == 0u) {
                    eax = GP4_ARRAY(uint32_t, 0x006f316c)[n];
                }
                GP4_ARRAY(uint32_t, 0x006f376c)[n] = eax;
            }
        }
        if ((GP4_GLOBAL(uint16_t, 0x0086855c) & 0x40u) != 0u) {
            eax = StreamSWord(s);
            s += 2u;
            GP4_ARRAY(uint32_t, 0x006f37ec)[n] = eax;
            eax = StreamSWord(s);
            s += 2u;
            GP4_ARRAY(uint32_t, 0x006f386c)[n] = eax;
            eax = StreamWord(s);
            s += 2u;
            GP4_ARRAY(uint32_t, 0x006f38ec)[n] = eax;
        }
        ++n;
        if (static_cast<int32_t>(n) > 0x20) {
            __debugbreak();
        }
    }
    if (n == 0u) {
        __debugbreak();
    }
    GP4_GLOBAL(uint32_t, 0x006f3ad8) = n;

    r->eax = eax;
    r->ebx = n;
    r->edx = c.edx;
    r->edi = c.edi;
    r->esi = s;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(n, 0u));
}
GP4_IMPL(0x0055ab0e, FUN_0055ab0e, "regs(esi:ptr[0x600]:layout=stream_0055ab0e) -> (eax, ebx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=stream_0055ab0e")

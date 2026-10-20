#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0053707b
// Verifier twin of 0x0053700e / 0x00537048: walks the same LZ-style stream (esi = stream, ecx =
// its length, edi = the original data) and checks that expanding it reproduces the bytes at edi.
// A flag byte (LSB first) covers the next 8 items: bit 0 = one literal byte compared with the
// byte at edi, bit 1 = a 2-byte token (low 13 bits = distance - 1, top 3 bits = length - 3) whose
// run is compared (repe cmpsb) between edi - distance and edi.  The end test is made before every
// item (a flag byte is always fetched first, so the read pointer may overshoot by one).
// Returns CF = 0 when the stream ended with every item matching, CF = 1 on the first mismatch.
// (A mismatch inside a match token leaves the pushed esi on the stack, so the original's `ret`
// then goes astray: that path is never meant to be taken; it is modelled up to the ret.)
// On exit eax = (esi+ecx) with al = last flag byte read, ebp = esi+ecx, ecx = remaining run count
// (0 once a match ran) else the input length, dl/dh = remaining flag bits / items left in the
// group, edi = end of compared data.  The direction flag is clear, as the game runs everywhere.
void __cdecl FUN_0053707b(gp4::Regs* r) {
    const uint32_t length = r->ecx;
    uint32_t pos = r->esi;
    uint32_t out = r->edi;
    const uint32_t end = length + pos;
    uint32_t ecx = length;
    uint32_t flagBits;
    uint32_t itemsLeft;
    uint32_t lastFlagByte;
    uint32_t arith;
    uint32_t esiOut;

    for (;;) {
        flagBits = *(const uint8_t*)(uintptr_t)pos;
        pos += 1;
        lastFlagByte = flagBits;
        itemsLeft = 8;
        for (;;) {
            if (pos >= end) {
                arith = gp4::flags::sub32(pos, end);
                esiOut = pos;
                goto done;
            }
            const bool isMatch = (flagBits & 1u) != 0;
            flagBits >>= 1;
            if (!isMatch) {
                const uint32_t a = *(const uint8_t*)(uintptr_t)pos;
                const uint32_t b = *(const uint8_t*)(uintptr_t)out;
                pos += 1;
                out += 1;
                if (a != b) {
                    arith = gp4::flags::sub8(a, b) | gp4::CF;
                    esiOut = pos;
                    goto done;
                }
            } else {
                const uint32_t token = *(const uint16_t*)(uintptr_t)pos;
                uint32_t from = out + (0u - ((token & 0x1fffu) + 1u));
                ecx = (token >> 13) + 3u;
                uint32_t a;
                uint32_t b;
                do {
                    a = *(const uint8_t*)(uintptr_t)from;
                    b = *(const uint8_t*)(uintptr_t)out;
                    from += 1;
                    out += 1;
                    ecx -= 1;
                } while (a == b && ecx != 0);
                if (a != b) {
                    arith = gp4::flags::sub8(a, b) | gp4::CF;
                    esiOut = from;
                    goto done;
                }
                pos += 2;
            }
            itemsLeft -= 1;
            if (itemsLeft == 0) {
                break;
            }
        }
    }

done:
    r->eax = (end & ~0xffu) | lastFlagByte;
    r->ebp = end;
    r->ecx = ecx;
    r->edx = (r->edx & 0xffff0000u) | (itemsLeft << 8) | flagBits;
    r->esi = esiOut;
    r->edi = out;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053707b, FUN_0053707b, "regs(ecx:u32[0..0x40], edx:u32, esi:ptr[0x48]:u8{0,0,0,0,0,0,0,1}, edi:rec[0x8943a0,1,8192..9216]) -> (eax, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of)")

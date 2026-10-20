#include <gp4/gp4.h>
#include <gp4/flags.h>

// LZ-style stream expander (the writing twin of 0x00537048, which only sizes the output).
// esi = compressed stream, ecx = its length in bytes, edi = output buffer.  A flag byte (LSB
// first) covers the next 8 items: bit 0 = one literal byte, bit 1 = a 2-byte token whose low 13
// bits are (distance - 1) and top 3 bits (length - 3); the copy runs forward byte by byte, so
// overlapping matches replicate.  The end test is made before every item (a flag byte is always
// fetched first, so the read pointer may overshoot the end by one).
// On exit eax = (esi+ecx) with al = last flag byte read, ebp = esi+ecx, ecx = 0 once any match
// ran (rep movsb) else the input length, dl/dh = remaining flag bits / items left in the group,
// esi = final read pointer, edi = end of output; flags of the final `cmp esi, ebp`.
// The direction flag is clear, as the game runs everywhere.
// FUNCTION: GP4 0x0053700e
void __cdecl FUN_0053700e(gp4::Regs* r) {
    const uint32_t length = r->ecx;
    uint32_t pos = r->esi;
    uint32_t out = r->edi;
    const uint32_t end = length + pos;
    uint32_t ecx = length;
    uint32_t flagBits;
    uint32_t itemsLeft;
    uint32_t lastFlagByte;

    for (;;) {
        flagBits = *(const uint8_t*)(uintptr_t)pos;
        pos += 1;
        lastFlagByte = flagBits;
        itemsLeft = 8;
        for (;;) {
            if (pos >= end) {
                goto done;
            }
            const bool isMatch = (flagBits & 1u) != 0;
            flagBits >>= 1;
            if (!isMatch) {
                *(uint8_t*)(uintptr_t)out = *(const uint8_t*)(uintptr_t)pos;
                out += 1;
                pos += 1;
            } else {
                const uint32_t token = *(const uint16_t*)(uintptr_t)pos;
                uint32_t count = (token >> 13) + 3u;
                uint32_t from = out + (0u - ((token & 0x1fffu) + 1u));
                ecx = 0;
                while (count != 0) {
                    *(uint8_t*)(uintptr_t)out = *(const uint8_t*)(uintptr_t)from;
                    out += 1;
                    from += 1;
                    count -= 1;
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
    r->esi = pos;
    r->edi = out;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(pos, end));
}
GP4_IMPL(0x0053700e, FUN_0053700e, "regs(ecx:u32[0..0x40], edx:u32, esi:ptr[0x48]:bytes, edi:ptr[0x2400]:bytes) -> (eax, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of)")

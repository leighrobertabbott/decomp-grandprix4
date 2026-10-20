#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00537048
// Walks an LZ-style stream (a flag byte, LSB first, covers the next 8 items: bit 0 = one literal
// byte, bit 1 = a 2-byte match token whose top 3 bits give length-3) and returns in ecx the size
// the stream expands to. Input: esi = stream, ecx = stream length in bytes. The walk stops when
// the read pointer reaches the end, tested before each item (a flag byte is fetched first, so the
// pointer may overshoot the end by up to one byte).
// Leaves eax = (esi+ecx) with al = last flag byte read, ebp = esi+ecx, edi = ecx = expanded size,
// dl/dh = remaining flag bits / items left in the group, esi = final read pointer and the flags of
// the final `cmp esi, ebp`.
void __cdecl FUN_00537048(gp4::Regs* r) {
    const uint32_t length = r->ecx;
    uint32_t pos = r->esi;
    const uint32_t end = length + pos;
    uint32_t expanded = 0;
    uint32_t flagBits = 0;
    uint32_t itemsLeft = 0;
    uint32_t lastFlagByte = 0;

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
                pos += 1;
                expanded += 1;
            } else {
                const uint32_t token = *(const uint16_t*)(uintptr_t)pos;
                expanded += (token >> 13) + 3u;
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
    r->edi = expanded;
    r->ecx = expanded;
    r->edx = (r->edx & 0xffff0000u) | (itemsLeft << 8) | flagBits;
    r->esi = pos;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(pos, end));
}
GP4_IMPL(0x00537048, FUN_00537048, "regs(ecx:u32[0..0x40], edx:u32, esi:ptr[0x48]:bytes) -> (eax, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of)")

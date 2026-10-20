#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream decoder opcode "raise flag 8": the word at esi is a record count. When the mode byte 0x86857c
// is not 2, esi just advances by two (flags of that add). In mode 2 the byte +0x64 of that many
// consecutive 0x160-byte ring records starting at ebx gets bit 8 set; the cursor wraps by the ring
// span [0x866ebc] whenever it reaches the ring end [0x866eb8]. esi advances by two, ebx ends on the
// next record, ecx is spent by `loop` (zero) and the flags are those of the last `cmp ebx, end` or the
// `sub ebx, span` that follows it.
// FUNCTION: GP4 0x00558a88
void __cdecl FUN_00558a88(gp4::Regs* r) {
    const uint32_t before = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2u) {
        r->esi = before + 2u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 2u));
        return;
    }
    uint32_t count = GP4_FIELD(uint16_t, before, 0);
    uint32_t record = r->ebx;
    uint32_t arith = 0u;
    do {
        GP4_FIELD(uint8_t, record, 0x64) |= 8u;
        record += 0x160u;
        const uint32_t end = GP4_GLOBAL(uint32_t, 0x00866eb8);
        arith = gp4::flags::sub32(record, end);
        if (record >= end) {
            const uint32_t span = GP4_GLOBAL(uint32_t, 0x00866ebc);
            arith = gp4::flags::sub32(record, span);
            record -= span;
        }
        count -= 1u;
    } while (count != 0u);
    r->esi = before + 2u;
    r->ebx = record;
    r->ecx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00558a88, FUN_00558a88, "regs(ebx:rec[0x8943a0,0x160,0..29], esi:ptr[8]:u16[1..40]) -> (ebx, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+mode_0086857c")

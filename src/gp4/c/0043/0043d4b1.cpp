#include <gp4/gp4.h>

// Flags of the 16-bit `sub left, right`.
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Zeroes the two running counters DAT_006269ac / DAT_006269b0, then walks the 0x160-byte track
// segment ring backwards from the lap start [0x00891504] ([0x00868558] + 0x100 + 1 steps, or one
// when that count is negative), running 0x0043d38f on every segment; the walk wraps by
// [0x00866ebc] when it drops below [0x00866ec0]. eax, ebx, ecx, edx and ebp are whatever the last
// 0x0043d38f left, edi is the segment after the last step, the low word of esi is the counter
// (0xffff at the end of a full walk) and the flags are those of the final `sub si, 1`.
// FUNCTION: GP4 0x0043d4b1
void __cdecl FUN_0043d4b1(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x006269ac) = 0;
    GP4_GLOBAL(uint32_t, 0x006269b0) = 0;

    uint32_t record = GP4_GLOBAL(uint32_t, 0x00891504);
    uint16_t counter = GP4_GLOBAL(uint16_t, 0x00868558);
    counter = static_cast<uint16_t>(counter + 0x100u);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint16_t previous;
    do {
        c.edi = record;
        c.esi = (r->esi & 0xffff0000u) | counter;
        gp4::call_regs(0x0043d38f, c);

        record -= 0x160u;
        if (!(record >= GP4_GLOBAL(uint32_t, 0x00866ec0))) {
            record += GP4_GLOBAL(uint32_t, 0x00866ebc);
        }
        previous = counter;
        counter = static_cast<uint16_t>(counter - 1u);
    } while (static_cast<int16_t>(counter) >= 0);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->edi = record;
    r->esi = (r->esi & 0xffff0000u) | counter;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags16(previous, 1u);
}
GP4_IMPL(0x0043d4b1, FUN_0043d4b1, "regs(ebx:u32, ecx:u32, esi:u32) -> (eax, ebx, ecx, edx, ebp, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_0043f696+ring_0043d30c+words_008943a0+tab_0043d5d8")

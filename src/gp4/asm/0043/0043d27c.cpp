#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043d27c
// edi = record. With a = sbyte [edi+0x11] + (sword [edi+0x4c] >> 6) and
// b = sbyte [edi+0x13] + (sword [edi+0x4e] >> 6) (16-bit sums), the 32-bit value a*a + b*b is
// handed to the integer square root 0x00409c44 and its ax is stored in the word [0x006077d8].
// eax, ebx, ecx and edx are saved and restored (push/pop). The flags are whatever the square
// root leaves: its closing "add esp, 0x20" makes them a function of the stack pointer, not of
// the data; the oracle's stack pointer gives PF = 1 and the other five arithmetic flags clear
// (pinned oracle model, same as 0x00409c44).
void __cdecl FUN_0043d27c(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const int16_t first = static_cast<int16_t>(
        static_cast<int16_t>(GP4_FIELD(int8_t, rec, 0x11)) + (GP4_FIELD(int16_t, rec, 0x4c) >> 6));
    const int16_t second = static_cast<int16_t>(
        static_cast<int16_t>(GP4_FIELD(int8_t, rec, 0x13)) + (GP4_FIELD(int16_t, rec, 0x4e) >> 6));

    // imul ax (dx:ax = b*b), moved to cx:bx; imul ax (dx:ax = a*a); add ax,bx / adc dx,cx
    const uint32_t sum = static_cast<uint32_t>(static_cast<int32_t>(second) * static_cast<int32_t>(second))
                       + static_cast<uint32_t>(static_cast<int32_t>(first) * static_cast<int32_t>(first));

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    callee.eax = (r->eax & 0xffff0000u) | (sum & 0xffffu);
    callee.edx = (r->edx & 0xffff0000u) | (sum >> 16);
    gp4::call_regs(0x00409c44, callee);

    GP4_GLOBAL(uint16_t, 0x006077d8) = static_cast<uint16_t>(callee.eax);
    r->eflags = (r->eflags & ~0x8d5u) | 0x04u;
}
GP4_IMPL(0x0043d27c, FUN_0043d27c, "regs(edi:ptr[0x56]:bytes) -> (cf, pf, af, zf, sf, of)")

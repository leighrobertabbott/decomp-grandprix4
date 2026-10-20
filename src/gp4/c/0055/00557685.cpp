#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// FUNCTION: GP4 0x00557685
// Pushes a ring record (ebx) on the 0x80-slot status stack when the mode byte 0x86857c is 3 (else
// only esi += 2): the stack counter 0x871c80 is incremented (>= 0x80 is an int3 assertion) and
// copied to 0x871c84; slot [counter] of the byte tables 0x875b46 / 0x875c4a gets the counter and the
// low byte of the mirror counter 0x871c88, slot [counter] of the dword table 0x875940 gets ebx, the
// record's byte +0x2b gets the counter and its word +0x78 gets bit 0x400; the word at [esi] goes to
// slot [counter] of the word table 0x871a70 and esi advances by 2. The status bits are then spread
// by 0x55776f. eax ends as (dword 0x871c88 & 0xffff0000) | the word, edx as the counter; ecx is
// pushed/popped; the flags are the callee's (or those of `add esi, 2` when the mode is not 3).
void __cdecl FUN_00557685(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 3u) {
        const uint32_t before = r->esi;
        r->esi = before + 2u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 2u));
        return;
    }

    GP4_GLOBAL(uint32_t, 0x00871c80) += 1u;
    if (GP4_GLOBAL(uint32_t, 0x00871c80) >= 0x80u) {
        __debugbreak();
        return;
    }
    const uint32_t index = GP4_GLOBAL(uint32_t, 0x00871c80);
    GP4_GLOBAL(uint32_t, 0x00871c84) = index;
    GP4_ARRAY(uint8_t, 0x00875b46)[index] = static_cast<uint8_t>(index);
    const uint32_t mirror = GP4_GLOBAL(uint32_t, 0x00871c88);
    GP4_ARRAY(uint8_t, 0x00875c4a)[index] = static_cast<uint8_t>(mirror);
    GP4_ARRAY(uint32_t, 0x00875940)[index] = r->ebx;

    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    GP4_FIELD(uint8_t, record, 0x2b) = static_cast<uint8_t>(index);
    GP4_FIELD(uint16_t, record, 0x78) |= 0x400u;

    const uint32_t word = GP4_GLOBAL(uint16_t, r->esi);
    GP4_ARRAY(uint16_t, 0x00871a70)[index] = static_cast<uint16_t>(word);

    gp4::Regs c = *r;
    c.eax = (mirror & 0xffff0000u) | word;
    c.edx = index;
    c.esi = r->esi + 2u;
    gp4::call_regs(0x0055776f, c);

    r->eax = c.eax;
    r->edx = index;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00557685, FUN_00557685, "regs(ebx:rec[0x8943a0,0x160,0..29], esi:ptr[0x10]:bytes) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+bits_0055616f+mode_0086857c+stack_0055753f")

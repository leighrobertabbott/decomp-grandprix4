#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Pushes the ring record ebx on the 0x80-entry history stack (only in mode 3 of the byte 0x0086857c;
// in any other mode it just skips the two words at esi). The stack index 0x00871c80 is incremented and
// copied to 0x00871c84/0x00871c88 and to the byte tables 0x00875b46/0x00875c4a; the record goes into the
// pointer table 0x00875940, its index into byte +0x2b and bits 0x400|0x800 are set in word +0x78. The two
// words at esi (consumed, esi += 4) go to the word tables 0x00871a70 / 0x00871b72; a nonzero first word
// runs 0x0055776f and a nonzero second word 0x005577ac for the record. eax keeps its upper half and ends
// with the second word, edx = the new index, ecx unchanged; the flags are those of the last `or ax, ax`
// or of the last callee.
// FUNCTION: GP4 0x005575fe
void __cdecl FUN_005575fe(gp4::Regs* r) {
    const uint32_t record = r->ebx;
    uint32_t esi = r->esi;

    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 3u) {
        r->esi = esi + 4u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 4u));
        return;
    }

    const uint32_t index = GP4_GLOBAL(uint32_t, 0x00871c80) + 1u;
    GP4_GLOBAL(uint32_t, 0x00871c80) = index;
    if (index >= 0x80u) {
        __debugbreak();
    }
    const uint32_t edx = GP4_GLOBAL(uint32_t, 0x00871c80);
    GP4_GLOBAL(uint32_t, 0x00871c84) = edx;
    GP4_ARRAY(uint8_t, 0x00875b46)[edx] = static_cast<uint8_t>(edx);
    GP4_GLOBAL(uint32_t, 0x00871c88) = edx;
    GP4_ARRAY(uint8_t, 0x00875c4a)[edx] = static_cast<uint8_t>(edx);
    GP4_ARRAY(uint32_t, 0x00875940)[edx] = record;
    GP4_FIELD(uint8_t, record, 0x2b) = static_cast<uint8_t>(edx);
    GP4_FIELD(uint16_t, record, 0x78) |= 0x400u;
    GP4_FIELD(uint16_t, record, 0x78) |= 0x800u;

    uint32_t eax = r->eax;
    uint32_t flags;

    const uint16_t first = GP4_FIELD(uint16_t, esi, 0);
    esi += 2u;
    GP4_ARRAY(uint16_t, 0x00871a70)[edx] = first;
    eax = (eax & 0xffff0000u) | first;
    flags = gp4::flags::logic16(first);
    if (first != 0u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eax = eax;
        c.edx = edx;
        c.esi = esi;
        c.eflags = gp4::flags::merge(c.eflags, flags);
        gp4::call_regs(0x0055776f, c);
        flags = c.eflags;
    }

    const uint16_t second = GP4_FIELD(uint16_t, esi, 0);
    esi += 2u;
    GP4_ARRAY(uint16_t, 0x00871b72)[edx] = second;
    eax = (eax & 0xffff0000u) | second;
    flags = gp4::flags::logic16(second);
    if (second != 0u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eax = eax;
        c.edx = edx;
        c.esi = esi;
        c.eflags = gp4::flags::merge(c.eflags, flags);
        gp4::call_regs(0x005577ac, c);
        flags = c.eflags;
    }

    r->eax = eax;
    r->edx = edx;
    r->esi = esi;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005575fe, FUN_005575fe, "regs(eax:u32, ebx:rec[0x8943a0,0x160,0..29], edx:u32, esi:ptr[8]:u16{=0,1,2,0x8000}) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c+stack_0055753f+ring_008943a0+bits_0055616f+flags2c_0055774b")

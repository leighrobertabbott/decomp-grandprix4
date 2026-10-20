#include <gp4/gp4.h>
#include <gp4/flags.h>

// Entry k of the dword table at 0x00891bd0 (indexed in a helper so the compiler keeps the table base as the
// only global, as in the original).
static __declspec(noinline) uint32_t TableEntry(uint32_t k) {
    return GP4_ARRAY(uint32_t, 0x00891bd0)[k];
}

// Readout set-up for the car/record at esi: the byte 0x00893cfb is the complement of the bit selected by the
// record byte +0x7f (low byte of the dword mask), the header cell 2 of the table 0x00891bd0 goes to the (empty)
// routine 0x00515b63, the 6-byte records 5 and 6 are fetched by 0x00514def, word +0x60 >> 6 and the value of
// 0x0040832b are written as two-digit readouts by 0x00510e71 (at [edi]), and when bit 5 of byte +0x8d is set
// cell 0xd of the table goes to 0x00515b63 too. Finally bit 3 of byte +0x8a is cleared and byte 0x00954169
// becomes 0xff. The registers are whatever the last callee leaves; the flags are those of the final `and`.
// FUNCTION: GP4 0x005159e0
void __cdecl FUN_005159e0(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;

    const uint32_t selector = record[0x7f];
    c.eax = selector;
    c.edx = ~(1u << (selector & 31u));
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(0u));
    GP4_GLOBAL(uint8_t, 0x00893cfb) = static_cast<uint8_t>(c.edx);

    c.eax = TableEntry(2u);
    gp4::call_regs(0x00515b63, c);

    c.eax = 5u;
    gp4::call_regs(0x00514def, c);

    const uint32_t word = *reinterpret_cast<const uint16_t*>(record + 0x60);
    const uint32_t shifted = word >> 6;
    c.eax = (c.eax & 0xffff0000u) | shifted;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic16(shifted) | ((word >> 5) & 1u));
    gp4::call_regs(0x00510e71, c);

    c.eax = 6u;
    gp4::call_regs(0x00514def, c);

    gp4::call_regs(0x0040832b, c);
    gp4::call_regs(0x00510e71, c);

    const uint32_t bit = record[0x8d] & 0x20u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(bit));
    if (bit != 0u) {
        c.eax = TableEntry(0xdu);
        gp4::call_regs(0x00515b63, c);
    }

    record[0x8a] = static_cast<uint8_t>(record[0x8a] & 0xf7u);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(record[0x8a]));
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xffu;
    *r = c;
}
GP4_IMPL(0x005159e0, FUN_005159e0, "regs(eax:u32, edx:u32, esi:ptr[0x1a0]:bytes, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+hudpos_00514def+glyph_005159e0")

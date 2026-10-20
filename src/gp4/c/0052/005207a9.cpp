#include <gp4/gp4.h>
#include <gp4/flags.h>

// Appends an 8-byte event to the ring buffer [[0x007a8e28], [0x007a8e2c]) at offset [0x0066d830],
// describing the car record [0x0066d820] (skipped when that pointer is null: eax = 0, flags of
// `or eax, eax`): byte 0 = ([0x00679d1b] & 0x60) | (record[+0x6f] & 0x9f), byte 1 = [0x00679d1c],
// word 2 = record[+0x6a] & 0x7fff, bytes 4/5/6 = record[+0x6e]/[+0x6c]/[+0x6d]. The offset advances
// by 8 and wraps to 0 once the cursor reaches the buffer end. Registers: esi = the record, dl =
// byte 0's record part, ax = (word & 0x7f00) | record[+0x6d], ebx = the new offset, flags of the
// final `sub ebx, [0x007a8e28]`.
// FUNCTION: GP4 0x005207a9
void __cdecl FUN_005207a9(gp4::Regs* r) {
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x0066d820);
    if (record == 0u) {
        r->eax = 0u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
        return;
    }

    uint32_t cursor = GP4_GLOBAL(uint32_t, 0x0066d830) + GP4_GLOBAL(uint32_t, 0x007a8e28);
    const uint8_t record_bits = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x6f) & 0x9fu);
    GP4_FIELD(uint8_t, cursor, 0) = static_cast<uint8_t>((GP4_GLOBAL(uint8_t, 0x00679d1b) & 0x60u) | record_bits);
    GP4_FIELD(uint8_t, cursor, 1) = GP4_GLOBAL(uint8_t, 0x00679d1c);
    const uint16_t word = static_cast<uint16_t>(GP4_FIELD(uint16_t, record, 0x6a) & 0x7fffu);
    GP4_FIELD(uint16_t, cursor, 2) = word;
    GP4_FIELD(uint8_t, cursor, 4) = GP4_FIELD(uint8_t, record, 0x6e);
    GP4_FIELD(uint8_t, cursor, 5) = GP4_FIELD(uint8_t, record, 0x6c);
    const uint8_t last = GP4_FIELD(uint8_t, record, 0x6d);
    GP4_FIELD(uint8_t, cursor, 6) = last;

    cursor += 8u;
    if (!(cursor < GP4_GLOBAL(uint32_t, 0x007a8e2c))) {
        cursor = GP4_GLOBAL(uint32_t, 0x007a8e28);
    }
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x007a8e28);
    const uint32_t flags = gp4::flags::sub32(cursor, base);
    const uint32_t offset = cursor - base;
    GP4_GLOBAL(uint32_t, 0x0066d830) = offset;

    r->eax = (record & 0xffff0000u) | (word & 0xff00u) | last;
    r->edx = (r->edx & 0xffffff00u) | record_bits;
    r->esi = record;
    r->ebx = offset;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005207a9, FUN_005207a9, "regs(edx:u32) -> (eax, edx, esi, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00437888+fill_005204d0+event_005207a9")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reads the next 8-byte event back from the ring buffer [[0x007a8e28], [0x007a8e2c]) at offset
// [0x0066d830] into the car record [0x0066d820] (nothing happens when that pointer is null: eax = 0,
// flags of `or eax, eax`): record[+0x6f] = byte0 & 0x9f, [0x00679d1b] = byte0 & 0x60,
// [0x00679d1c] = byte1, record[+0x6a] = the word at +2 shifted left then arithmetic right by 1
// (15-bit sign extension), record[+0x6e]/[+0x6c]/[+0x6d] = bytes 4/5/6. The offset advances by 8
// and wraps to 0 once the cursor reaches the end. Reader twin of 0x005207a9. Registers: dl =
// byte0 & 0x60, ax = (shifted word & 0xff00) | byte 6 (the high half of eax keeps the record
// pointer), flags of the final `sub edi, [0x007a8e28]`; esi and edi are restored.
// FUNCTION: GP4 0x00520735
void __cdecl FUN_00520735(gp4::Regs* r) {
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x0066d820);
    if (record == 0u) {
        r->eax = 0u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
        return;
    }

    uint32_t cursor = GP4_GLOBAL(uint32_t, 0x0066d830) + GP4_GLOBAL(uint32_t, 0x007a8e28);
    const uint8_t byte0 = GP4_FIELD(uint8_t, cursor, 0);
    GP4_FIELD(uint8_t, record, 0x6f) = static_cast<uint8_t>(byte0 & 0x9fu);
    const uint8_t high_bits = static_cast<uint8_t>(byte0 & 0x60u);
    GP4_GLOBAL(uint8_t, 0x00679d1b) = high_bits;
    GP4_GLOBAL(uint8_t, 0x00679d1c) = GP4_FIELD(uint8_t, cursor, 1);
    const int16_t doubled = static_cast<int16_t>(static_cast<uint16_t>(GP4_FIELD(uint16_t, cursor, 2) << 1));
    const uint16_t word = static_cast<uint16_t>(doubled >> 1);
    GP4_FIELD(uint16_t, record, 0x6a) = word;
    GP4_FIELD(uint8_t, record, 0x6e) = GP4_FIELD(uint8_t, cursor, 4);
    GP4_FIELD(uint8_t, record, 0x6c) = GP4_FIELD(uint8_t, cursor, 5);
    const uint8_t last = GP4_FIELD(uint8_t, cursor, 6);
    GP4_FIELD(uint8_t, record, 0x6d) = last;

    cursor += 8u;
    if (!(cursor < GP4_GLOBAL(uint32_t, 0x007a8e2c))) {
        cursor = GP4_GLOBAL(uint32_t, 0x007a8e28);
    }
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x007a8e28);
    const uint32_t flags = gp4::flags::sub32(cursor, base);
    GP4_GLOBAL(uint32_t, 0x0066d830) = cursor - base;

    r->eax = (record & 0xffff0000u) | (static_cast<uint32_t>(word) & 0xff00u) | last;
    r->edx = (r->edx & 0xffffff00u) | high_bits;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00520735, FUN_00520735, "regs(edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00437888+fill_005204d0+event_005207a9+evbuf_00520735")

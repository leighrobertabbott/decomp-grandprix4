#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0054a515
// Unpacks the 6-byte record at 0x007a97b2 into the menu input state:
//   [0x007aaffc] = byte0 & 0x1f              [0x0082fd6a] = byte0 >> 5
//   [0x0082fd9c] = byte1 >> 4                [0x0082fd94] = ((byte1 << 8) | byte2) & 0x3ff
//   [0x0082fd98] = (((byte1 >> 2) << 8) | byte3) & 0x3ff     [0x007ab00e] = word at +4
// then advances the 3-bit counter with 0x0054ba6b (eax = word at +4 with its low byte replaced by the
// new counter value, flags of that routine's `and al, 7`) and ends with `clc`. ebx and edx end as the
// two coordinates, esi as the record address.
void __cdecl FUN_0054a515(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(0x007a97b2));
    const uint32_t byte0 = record[0];
    const uint32_t byte1 = record[1];
    const uint32_t byte2 = record[2];
    const uint32_t byte3 = record[3];
    const uint32_t word4 = *reinterpret_cast<const uint16_t*>(record + 4);

    GP4_GLOBAL(uint8_t, 0x007aaffc) = static_cast<uint8_t>(byte0 & 0x1fu);
    GP4_GLOBAL(uint8_t, 0x0082fd6a) = static_cast<uint8_t>(byte0 >> 5);
    GP4_GLOBAL(uint32_t, 0x0082fd9c) = (byte1 >> 4) & 0xfu;
    const uint32_t x = ((byte1 << 8) | byte2) & 0x3ffu;
    const uint32_t y = ((((byte1 >> 2) & 0xffu) << 8) | byte3) & 0x3ffu;
    GP4_GLOBAL(uint32_t, 0x0082fd94) = x;
    GP4_GLOBAL(uint32_t, 0x0082fd98) = y;
    GP4_GLOBAL(uint16_t, 0x007ab00e) = static_cast<uint16_t>(word4);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = word4;
    gp4::call_regs(0x0054ba6b, c);

    r->eax = c.eax;
    r->ebx = x;
    r->edx = y;
    r->esi = 0x007a97b2u;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags & ~gp4::flags::CF);   // clc
}
GP4_IMPL(0x0054a515, FUN_0054a515, "regs() -> (eax, ebx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draws a two-field time-like readout of the record at edi: byte +0xf (a 12-hour clock value: 1..11 -> text 0x43 with the
// value, 12 -> text 0x44 with 12, 13.. -> text 0x44 with the value - 12) as a zero-padded two-character number
// (0x0054d969 with bh = 0x40, bl = 0x16, cl = 2), the separator byte at 0x0082974a (".,.." string, its first byte) through
// 0x0054d3a1, then byte +0xe as another zero-padded two-character number, and finally the string table entry whose id is
// left in esi (0x43 / 0x44) through 0x0054d1a2. Every callee preserves the registers it was not given a result in, so the
// registers and flags that escape are those the chain leaves: eax = byte +0xe, ebx = (ebx & ~0xffff) | 0x4016,
// ecx = (ecx & ~0xff) | 2, esi from 0x0054d1a2, the flags of the last callee.
// FUNCTION: GP4 0x0054373b
void __cdecl FUN_0054373b(gp4::Regs* r) {
    const uint8_t* rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));

    uint32_t value = rec[0xf];
    uint32_t id = 0x43u;
    const uint32_t compare = gp4::flags::sub32(value, 0xcu);
    if (value >= 0xcu) {
        id = 0x44u;
        if (value != 0xcu) {
            value -= 0xcu;
        }
    }

    gp4::Regs c = *r;
    c.eflags = gp4::flags::merge(r->eflags, compare);
    c.eax = value;
    c.esi = id;
    c.ebx = (c.ebx & 0xffff0000u) | 0x4016u;
    c.ecx = (c.ecx & 0xffffff00u) | 2u;
    gp4::call_regs(0x0054d969, c);

    c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x0082974a);
    gp4::call_regs(0x0054d3a1, c);

    c.eax = rec[0xe];
    c.ebx = (c.ebx & 0xffff0000u) | 0x4016u;
    c.ecx = (c.ecx & 0xffffff00u) | 2u;
    gp4::call_regs(0x0054d969, c);

    gp4::call_regs(0x0054d1a2, c);
    *r = c;
}
GP4_IMPL(0x0054373b, FUN_0054373b, "regs(ebx:u32, ecx:u32, edi:ptr[0x40]:u8{=0,1,5,0xb,0xc,0xd,0x15,0x17,0x20,0x6f,0x70,0xff}) -> (eax, ebx, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_all_00835846+log_004fd730+textout_0054d211")

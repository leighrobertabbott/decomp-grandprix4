#include <gp4/gp4.h>

// Consumes one 10-byte record of the opcode stream read at [0x0062d1a8] (byte +1 = car number, +4 = control
// byte, words +5 / +7, byte +9) and applies it to the car with that number: its pointer (0x00445267) becomes
// [0x0066d828]; with no such car CF is set (stc, flags of `or esi, esi`). Otherwise the settings block at
// 0x007ab56c is copied into the car (0x00410dd2), the control byte is unpacked into its flag bits (0x004451b9),
// the words go to car +0xf6 / +0x10e, the byte to [0x0066d8b6], and CF is cleared. The read cursor always
// advances by 10. ebx comes back as the old cursor; eax / ecx / esi are those of the callees (al = byte +9).
// FUNCTION: GP4 0x00445130
void __cdecl FUN_00445130(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x0062d1a8);
    GP4_GLOBAL(uint32_t, 0x0062d1a8) = cursor + 10u;
    const uint8_t* stream = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(cursor));

    c.ebx = cursor;
    c.eax = (c.eax & 0xffffff00u) | stream[1];
    gp4::call_regs(0x00445267, c);
    GP4_GLOBAL(uint32_t, 0x0066d828) = c.esi;

    if (c.esi == 0u) {
        r->eax = c.eax;
        r->ebx = cursor;
        r->ecx = c.ecx;
        r->esi = 0u;
        r->eflags = (r->eflags & ~0x8d5u) | 0x45u;    // or esi, esi (ZF|PF), then stc
        return;
    }

    c.ebx = 0x007ab56cu;
    gp4::call_regs(0x00410dd2, c);

    c.ebx = cursor;
    c.eax = (c.eax & 0xffffff00u) | stream[4];
    gp4::call_regs(0x004451b9, c);

    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
    *reinterpret_cast<uint16_t*>(car + 0xf6) = *reinterpret_cast<const uint16_t*>(stream + 5);
    *reinterpret_cast<uint16_t*>(car + 0x10e) = *reinterpret_cast<const uint16_t*>(stream + 7);
    const uint8_t last = stream[9];
    GP4_GLOBAL(uint8_t, 0x0066d8b6) = last;

    r->eax = (c.eax & 0xffffff00u) | last;
    r->ebx = cursor;
    r->ecx = c.ecx;
    r->esi = c.esi;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d4u);    // clc keeps the flags of `and al, 0xf`
}
GP4_IMPL(0x00445130, FUN_00445130, "regs(eax:u32) -> (eax, ebx, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+carno_0066ff6c+stream_0062d1a8+flag_007a97a0+ctl_007ab56c")

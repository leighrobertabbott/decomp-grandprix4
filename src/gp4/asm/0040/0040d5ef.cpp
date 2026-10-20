#include <gp4/gp4.h>

// edi = a track-segment record: its position (dwords +0xf0 / +0xf8) is looked up by 0x00406dbb (which
// leaves a record in edi and sets byte [0x627946] when the position is on the pit lane), then, in that
// case only, 0x00407811 converts it and the result [0x6278b4] is returned. edx returns the record that
// 0x00406dbb left in edi; every other register is restored, the flags are those of 0x00407811 (or of
// `cmp byte [0x627946], 0` on equal operands).
// FUNCTION: GP4 0x0040d5ef
void __cdecl FUN_0040d5ef(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(uint8_t, 0x00627945) = 0xff;
    GP4_GLOBAL(uint8_t, 0x00627944) = 0;
    GP4_GLOBAL(uint8_t, 0x00627947) = 0;
    c.eax = GP4_FIELD(uint32_t, record, 0xf0);
    c.edx = GP4_FIELD(uint32_t, record, 0xf8);
    gp4::call_regs(0x00406dbb, c);

    if (GP4_GLOBAL(uint8_t, 0x00627946) != 0u) {
        c.st_in = 0;
        c.st_out = 0;
        gp4::call_regs(0x00407811, c);
        r->eax = GP4_GLOBAL(uint32_t, 0x006278b4);
        r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
    } else {
        r->eax = 0u;
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;  // cmp of two zero bytes: PF | ZF
    }
    r->edx = c.edi;
}
GP4_IMPL(0x0040d5ef, FUN_0040d5ef, "regs(edi:rec[0x8943a0,0x160,0..11]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+state_0040759e+trackfar_008943a0")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the 0x160-byte segment records from the ring start [0x00866ec0] up to the ring end
// [0x00866eb8]: for each record the attribute pairs of its flag dwords are completed by 0x00558872
// (edi = the record, ebp = dword +0x2c of the PREVIOUS record, of the record itself for the first one:
// the loop re-enters at the call, after `mov ebp, [edi+0x2c]` of the record just processed), then the
// signed words +0x8e and +0x90 are raised to +0xb0 and +0xb2 when they are lower. A pushal/popal
// wrapper: every register is restored, the flags are those of the last `cmp edi, [0x00866eb8]`
// (equal on exit).
// FUNCTION: GP4 0x005589cb
void __cdecl FUN_005589cb(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint32_t record = GP4_GLOBAL(uint32_t, 0x00866ec0);
    c.ebp = GP4_FIELD(uint32_t, record, 0x2c);
    uint32_t arith;
    do {
        c.edi = record;
        gp4::call_regs(0x00558872, c);
        c.ebp = GP4_FIELD(uint32_t, record, 0x2c);

        const int16_t floorA = GP4_FIELD(int16_t, record, 0xb0);
        if (GP4_FIELD(int16_t, record, 0x8e) < floorA) {
            GP4_FIELD(int16_t, record, 0x8e) = floorA;
        }
        const int16_t floorB = GP4_FIELD(int16_t, record, 0xb2);
        if (GP4_FIELD(int16_t, record, 0x90) < floorB) {
            GP4_FIELD(int16_t, record, 0x90) = floorB;
        }

        record += 0x160u;
        arith = gp4::flags::sub32(record, GP4_GLOBAL(uint32_t, 0x00866eb8));
    } while (record != GP4_GLOBAL(uint32_t, 0x00866eb8));

    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005589cb, FUN_005589cb, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=segs_00558d9b+wave_005587a0+raise_005589cb")

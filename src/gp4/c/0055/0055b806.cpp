#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: runs the per-record routine 0x0055b763 over two forward walks of the 0x160-byte
// track ring: the records from the ring base 0x008943a0 up to the lap end [0x00866ecc], then those
// from the pit start [0x00891500] up to the pit end [0x00866ef8] (both do-while walks that stop on
// equality). popal restores every register, so the only output is the flags of the last
// `cmp edi, [0x00866ef8]` (equal).
// FUNCTION: GP4 0x0055b806
void __cdecl FUN_0055b806(gp4::Regs* r) {
    uint32_t record = 0x008943a0u;
    do {
        gp4::Regs c = {};
        c.edi = record;
        gp4::call_regs(0x0055b763, c);
        record += 0x160u;
    } while (record != GP4_GLOBAL(uint32_t, 0x00866ecc));
    record = GP4_GLOBAL(uint32_t, 0x00891500);
    uint32_t pit_end;
    do {
        gp4::Regs c = {};
        c.edi = record;
        gp4::call_regs(0x0055b763, c);
        record += 0x160u;
        pit_end = GP4_GLOBAL(uint32_t, 0x00866ef8);
    } while (record != pit_end);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(record, pit_end));
}
GP4_IMPL(0x0055b806, FUN_0055b806, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ring_00555b28+bits_0055b806")

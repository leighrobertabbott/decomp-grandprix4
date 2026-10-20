#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00510752
// pushal wrapper: looks up the car record for the index [0xa69878] (0x005106f6, clamped to the 22 cars),
// remembers its track-segment pointer (+0x1c) in [0xa6987c] and stores the low 11 bits of the dword +0x1a
// of that segment record back into [0xa69878]. popal restores every GPR; the flags are those of the final
// `and ecx, 0x7ff`.
void __cdecl FUN_00510752(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ecx = GP4_GLOBAL(uint32_t, 0x00a69878);
    gp4::call_regs(0x005106f6, c);

    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
    const uint32_t segment = GP4_FIELD(uint32_t, car, 0x1c);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = segment;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(segment));
    const uint32_t value = GP4_FIELD(uint32_t, record, 0x1a) & 0x7ffu;
    GP4_GLOBAL(uint32_t, 0x00a69878) = value;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
}
GP4_IMPL(0x00510752, FUN_00510752, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+cars_00433709+records_008943a0+seg1a_00510752")

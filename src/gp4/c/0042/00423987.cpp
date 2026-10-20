#include <gp4/gp4.h>
#include <gp4/x87.h>

// Array of 0x64-byte records at 0x00621aa0 (only the fields this routine touches).
struct Unknown_00621aa0 {
    uint8_t Unknown00[0x4c];
    int32_t Unknown4C;
    int32_t Unknown50;
    int32_t Unknown54;
    uint8_t Unknown58[0x64 - 0x58];
};
static_assert(sizeof(Unknown_00621aa0) == 0x64, "record stride");

// FUNCTION: GP4 0x00423987
// For each of the DAT_00609864 records: divisor = max(rec.Unknown54, 8) (also left in
// DAT_006f2bf4 and in ebp); rec.Unknown50 = fistp of 0x00401ccc(limit = DAT_00608028,
// divisor, dividend = rec.Unknown4C * DAT_006f2b2c). With a zero count nothing happens.
// ecx ends equal to the count, ebp holds the last divisor; flags are those of the final
// compare (equal), which is also what the zero-count test leaves.
void __cdecl FUN_00423987(gp4::Regs* r) {
    using namespace gp4::x87;
    if (GP4_GLOBAL(uint32_t, 0x00609864) == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    Unknown_00621aa0* const base = GP4_ARRAY(Unknown_00621aa0, 0x00621aa0);
    uint32_t i = 0;
    int32_t ebp;
    do {
        Unknown_00621aa0* const rec = base + i;
        ebp = rec->Unknown54;
        if (ebp < 8) {
            ebp = 8;
        }
        const Ext dividend = ext_i32(rec->Unknown4C) * ext(GP4_GLOBAL(double, 0x006f2b2c));
        GP4_GLOBAL(int32_t, 0x006f2bf4) = ebp;
        const Ext divisor = ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
        const Ext limit = ext_i32(GP4_GLOBAL(int32_t, 0x00608028));

        gp4::Regs callee = *r;
        callee.st_xin_mask = 0;
        callee.st_xout_mask = 0;
        callee.st_in = 3;
        callee.st_out = 1;
        set_in_st(callee, 0, limit);
        set_in_st(callee, 1, divisor);
        set_in_st(callee, 2, dividend);
        gp4::call_regs(0x00401ccc, callee);

        rec->Unknown50 = fistp32(out_st(callee, 0));
        ++i;
    } while (i != GP4_GLOBAL(uint32_t, 0x00609864));

    r->ebp = (uint32_t)ebp;
    r->ecx = i;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00423987, FUN_00423987, "regs(ebp:u32, ecx:u32) -> (ebp, ecx, cf, pf, af, zf, sf, of); globals=layout=state_00423987")

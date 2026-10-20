#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores into 0x00a6987c a byte-derived value for the selected car record [0x00679d30]:
//  - bit 4 of the record byte +0x8b set: the result of 0x004080c1 (the record's byte +0x226);
//  - otherwise row = clamp(((byte +0x7c) - 1) & 0x3f, 0, 0x15), column = byte 0x0066d936[row] and
//    the value is (row * 0x2c with its low byte replaced by the table byte 0x0066d962[row*0x2c+column]).
// pushal/popal wrapper: every GPR is restored; the flags are those of the last flag writer: the
// state 0x004080c1 leaves (call path) or the `imul eax, eax, 0x2c` (row * 0x2c never overflows:
// ZF/SF/PF from the product, CF = OF = AF = 0).
// FUNCTION: GP4 0x00516750
void __cdecl FUN_00516750(gp4::Regs* r) {
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x00679d30);
    uint32_t value;
    uint32_t arith;
    if ((GP4_FIELD(uint8_t, record, 0x8b) & 0x10u) != 0u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.st_xin_mask = 0;
        call.st_xout_mask = 0;
        call.esi = record;
        gp4::call_regs(0x004080c1, call);
        value = call.eax;
        arith = call.eflags;
    } else {
        uint32_t row = ((uint32_t)(uint8_t)(GP4_FIELD(uint8_t, record, 0x7c) - 1u)) & 0x3fu;
        if ((int32_t)row > 0x15) {
            row = 0x15u;
        }
        const uint32_t column = GP4_ARRAY(uint8_t, 0x0066d936)[row];
        const uint32_t product = row * 0x2cu;
        value = (product & 0xffffff00u) | GP4_ARRAY(uint8_t, 0x0066d962)[column + product];
        arith = gp4::flags::logic32(product);
    }
    GP4_GLOBAL(uint32_t, 0x00a6987c) = value;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00516750, FUN_00516750, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+carno_0066ff6c+order_00418f8f+counter_0040f0dd+tabs_004413b7")

#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit cmp left, right.
static inline uint32_t cmp32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_00411891_Table {
    int32_t Unknown00;      // first index with a segment
    int32_t Unknown04;      // last index (clamp)
    double Unknown08;       // input scale
    double Unknown10[1];    // samples
};

// FUNCTION: GP4 0x00411891
void __cdecl FUN_00411891(gp4::Regs* r) {
    const Unknown_00411891_Table* table = reinterpret_cast<const Unknown_00411891_Table*>(static_cast<uintptr_t>(r->edx));
    const gp4::x87::Ext input = gp4::x87::in_st(r, 0);

    // fldcw [0x6f29a4] (rounding control for the table lookup)
    gp4::x87::set_cw(GP4_GLOBAL(uint16_t, 0x006f29a4));

    // fmul qword [edx+8] ; fld st0 ; frndint ; fist dword [0x6f2bf8] ; fsubp st1
    const gp4::x87::Ext scaled = input * gp4::x87::ext(table->Unknown08);
    const gp4::x87::Ext whole = gp4::x87::frndint(scaled);
    const int32_t index = gp4::x87::fistp32(whole);
    GP4_GLOBAL(int32_t, 0x006f2bf8) = index;
    const gp4::x87::Ext fraction = scaled - whole;

    gp4::x87::Ext result;
    uint32_t flags = cmp32Flags(static_cast<uint32_t>(index), static_cast<uint32_t>(table->Unknown04));
    if (index >= table->Unknown04) {
        result = gp4::x87::ext(table->Unknown10[table->Unknown04]);
    } else {
        flags = cmp32Flags(static_cast<uint32_t>(index), static_cast<uint32_t>(table->Unknown00));
        if (index >= table->Unknown00) {
            const gp4::x87::Ext step = gp4::x87::ext(table->Unknown10[index + 1])
                                       - gp4::x87::ext(table->Unknown10[index]);
            result = fraction * step + gp4::x87::ext(table->Unknown10[index]);
        } else {
            result = gp4::x87::ext(table->Unknown10[0]);
        }
    }

    // fldcw [0x6f29a6] (restore)
    gp4::x87::set_cw(GP4_GLOBAL(uint16_t, 0x006f29a6));

    gp4::x87::set_out_st(r, 0, result);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00411891, FUN_00411891, "regs(edx:ptr[0x100]:layout=table_00411891, st0:f64) -> (st0, cf, pf, af, zf, sf, of)")

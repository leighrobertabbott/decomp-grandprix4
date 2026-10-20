#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left + right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040cdc3
// pushal / popal wrapper (every register comes back; the flags are those of the final
// `add eax, 0x2d60`). esi = the circuit stream pointer [0x62d280] for the whole chain: the stream
// parser 0x0040cd00, the nearest-record search 0x0040def9, 0x0040d4c4, 0x0041e7f3, 0x0040cb38,
// 0x00431e58, 0x00431de8 and 0x0040b081. Then words +0x16 and +0x18 of the pit-end segment record
// [0x866ef8] are cleared and [0x866f0c] = [0x866f08] + 0x2d60.
void __cdecl FUN_0040cdc3(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = GP4_GLOBAL(uint32_t, 0x0062d280);

    gp4::call_regs(0x0040cd00, c);
    gp4::call_regs(0x0040def9, c);
    gp4::call_regs(0x0040d4c4, c);
    gp4::call_regs(0x0041e7f3, c);
    gp4::call_regs(0x0040cb38, c);
    gp4::call_regs(0x00431e58, c);
    gp4::call_regs(0x00431de8, c);
    gp4::call_regs(0x0040b081, c);

    uint8_t* segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00866ef8)));
    GP4_FIELD(uint16_t, segment, 0x16) = 0;
    GP4_FIELD(uint16_t, segment, 0x18) = 0;
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x00866f08);
    GP4_GLOBAL(uint32_t, 0x00866f0c) = base + 0x2d60u;

    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(base, 0x2d60u);
}
GP4_IMPL(0x0040cdc3, FUN_0040cdc3, "regs(ebx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=records_008943a0+ring_0040dde6+ends_0040def9+state_0040cb38+state_00431e58+stream_0062d280+dvar_0041eacf+state_0040d19f+state_0040d389+ptr_00627938+scan_00406d4b+lap_0040cf8c+trackgeo_008943a0+tbl_00868598+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+state_0040759e+trackfar_008943a0+pit_00406dbb+pitgeo_0040d644+wrap_0040cdc3+pitgeo_0040cdc3+geo_0040cdc3")
// v3 retry

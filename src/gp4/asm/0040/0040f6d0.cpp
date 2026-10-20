#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a byte logic result (test / and / or): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// edx:eax = signed car value * signed factor, then `shrd eax, edx, 0x18`: returns the new eax and
// leaves the high dword (edx) in *high.
static inline uint32_t scaledValue(int32_t value, int32_t factor, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(factor);
    const uint32_t lo = static_cast<uint32_t>(product);
    const uint32_t hi = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    *high = hi;
    return (lo >> 24) | (hi << 8);
}

// FUNCTION: GP4 0x0040f6d0
// One-shot car reset, guarded by bit 0 of byte +0xe5 (already set: return, flags of that test).
// Otherwise it sets the bit, clears +0xac / +0x288 / word +0x23c, runs the placement step
// 0x4048da with the word at +0x18 advanced by (+0x28c * [0x603854]) >> 24 (and restores the word),
// then 0x40f629, 0x43624f, 0x409456 (only while [0x604418] is nonzero), 0x403041, 0x409467 and
// 0x4036ca, advances the word at +0xe by (+0x28c * [0x603400]) >> 24, runs 0x40f680 and sets the
// flag bits +0x162 |= 2, +0x203 |= 0x40, +0xce |= 0x10, +0x203 &= 0xfd, +0x2bb = 0. pushal/popal
// restore every register; the flags are those of the final `and`.
void __cdecl FUN_0040f6d0(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint32_t state = GP4_FIELD(uint8_t, car, 0xe5);
    if ((state & 1u) != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(state & 1u);
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_FIELD(uint8_t, car, 0xe5) = static_cast<uint8_t>(state | 1u);
    GP4_FIELD(uint32_t, car, 0xac) = 0u;
    GP4_FIELD(uint32_t, car, 0x288) = 0u;
    GP4_FIELD(uint16_t, car, 0x23c) = 0u;
    c.edi = GP4_FIELD(uint32_t, car, 0x1c);

    // ax = word +0x18 is pushed (upper half of eax unchanged), the word is advanced for 0x4048da.
    const uint16_t word18 = GP4_FIELD(uint16_t, car, 0x18);
    const uint32_t savedEax = (r->eax & 0xffff0000u) | word18;
    uint32_t high = 0u;
    const uint32_t advance = scaledValue(GP4_FIELD(int32_t, car, 0x28c), GP4_GLOBAL(int32_t, 0x00603854), &high);
    const uint16_t advanced = static_cast<uint16_t>(static_cast<uint16_t>(advance) + word18);
    GP4_FIELD(uint16_t, car, 0x18) = advanced;
    c.eax = (advance & 0xffff0000u) | advanced;
    c.edx = high;
    gp4::call_regs(0x004048da, c);

    c.eax = savedEax;
    GP4_FIELD(uint16_t, car, 0x18) = word18;
    gp4::call_regs(0x0040f629, c);
    gp4::call_regs(0x0043624f, c);
    if (GP4_GLOBAL(uint8_t, 0x00604418) != 0u) {
        gp4::call_regs(0x00409456, c);
    }
    gp4::call_regs(0x00403041, c);
    gp4::call_regs(0x00409467, c);
    gp4::call_regs(0x004036ca, c);

    const uint32_t turn = scaledValue(GP4_FIELD(int32_t, car, 0x28c), GP4_GLOBAL(int32_t, 0x00603400), &high);
    c.eax = turn;
    c.edx = high;
    GP4_FIELD(uint16_t, car, 0xe) = static_cast<uint16_t>(GP4_FIELD(uint16_t, car, 0xe) + static_cast<uint16_t>(turn));
    gp4::call_regs(0x0040f680, c);

    GP4_FIELD(uint8_t, car, 0x162) |= 2;
    GP4_FIELD(uint8_t, car, 0x203) |= 0x40;
    GP4_FIELD(uint8_t, car, 0x2bb) = 0;
    GP4_FIELD(uint8_t, car, 0xce) |= 0x10;
    GP4_FIELD(uint8_t, car, 0x203) &= 0xfd;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(GP4_FIELD(uint8_t, car, 0x203));
}
GP4_IMPL(0x0040f6d0, FUN_0040f6d0, "regs(esi:ptr[0x420]:layout=car_004129fa+rec_00412b88) -> (cf, pf, af, zf, sf, of); cases=400; globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_0040705f+frac_0040705f+state_00407af6+state_004381ab+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+state_0040759e+trackfar_008943a0")

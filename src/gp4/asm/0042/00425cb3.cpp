#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the one-operand `imul r/m16` (oracle model): CF = OF = the 32-bit product does not fit the
// low word, PF/ZF/SF come from the low word, AF = 0.
static inline uint32_t imulFlags16(int32_t product) {
    const uint32_t low = static_cast<uint32_t>(product) & 0xffffu;
    const bool overflow = product != static_cast<int32_t>(static_cast<int16_t>(low));
    return (overflow ? 0x801u : 0u)
        | parityFlag8(low)
        | (low == 0u ? 0x40u : 0u)
        | ((low & 0x8000u) != 0u ? 0x80u : 0u);
}

static inline uint32_t withLow16(uint32_t reg, uint32_t value) {
    return (reg & 0xffff0000u) | (value & 0xffffu);
}

static inline uint32_t signExtend16(uint32_t value) {
    return static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(value)));
}

// Blend of the stored lane endpoint with the car's weight: low word of ((ax - end) * weight << 2 >> 16) + end,
// the 32-bit product leaving bits 14..29 in dx.
static inline void blendLane(gp4::Regs& st, uint8_t* car, uint8_t* lane, uint32_t endOffset) {
    const int16_t end = GP4_FIELD(int16_t, lane, endOffset);
    const int16_t diff = static_cast<int16_t>(st.eax - static_cast<uint32_t>(end));
    const int32_t product = static_cast<int32_t>(diff) * static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x140));
    const uint32_t mid = (static_cast<uint32_t>(product) >> 14) & 0xffffu;
    st.edx = withLow16(st.edx, mid);
    st.eax = withLow16(st.eax, mid + static_cast<uint32_t>(static_cast<uint16_t>(end)));
}

// FUNCTION: GP4 0x00425cb3
// Steering-limit evaluation of the car at esi for lane entry eax & 0xff of the 16-byte table at 0x6269b8
// (word +0 base offset, +2 reference, +4 / +6 lane bounds, dword +8 / +0xc segment records copied to
// 0x602d84 / 0x602d88). The bound is clamped by 0x425be8 / 0x425bed (flags +0xfe 0x40, +0x163), eased by the
// weight +0x140 when [0x603e6c] is neither 0xff nor 0 and +0x135 bit 5 is set, optionally refined by
// 0x425c87 and 0x4056c9 (>> 8), by 0x40f833 for the player car and by 0x425b71, stored at +0x100 / +0x144 and
// returned as dx:ax = (v - ref) * ((v + ref) >> 1) (16-bit imul). The word +0xa2 and dword +0x240 of the car are
// restored; ebx keeps the table entry address.
void __cdecl FUN_00425cb3(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs st = *r;
    st.st_in = 0;
    st.st_out = 0;

    const uint32_t savedDword240 = GP4_FIELD(uint32_t, car, 0x240);
    const uint16_t savedWordA2 = GP4_FIELD(uint16_t, car, 0xa2);

    const uint32_t entry = 0x006269b8u + (r->eax & 0xffu) * 0x10u;
    uint8_t* const lane = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(entry));
    st.ebx = entry;
    GP4_GLOBAL(uint32_t, 0x00602d84) = GP4_FIELD(uint32_t, lane, 8);
    GP4_GLOBAL(uint32_t, 0x00602d88) = GP4_FIELD(uint32_t, lane, 0xc);
    st.eax = withLow16(GP4_FIELD(uint32_t, lane, 0xc), GP4_FIELD(uint16_t, lane, 0));

    if (GP4_FIELD(uint32_t, lane, 8) != 0u) {
        const uint32_t savedEcx = st.ecx;
        const uint32_t savedEdx = st.edx;
        st.eax = signExtend16(st.eax);
        if ((car[0xfe] & 0x40u) != 0u) {
            st.edx = signExtend16(GP4_FIELD(uint16_t, lane, (car[0xfe] & 0x10u) == 0u ? 6u : 4u));
            gp4::call_regs(0x00425be8, st);
        }
        st.edx = savedEdx;
        if ((car[0x163] & 0x80u) != 0u && (car[0x122] & 0x80u) == 0u && (car[0x163] & 0x40u) != 0u) {
            gp4::call_regs(0x00425bed, st);
        }
        st.ecx = savedEcx;
        if (GP4_GLOBAL(uint8_t, 0x00603e6c) != 0xffu && (car[0x135] & 0x20u) != 0u) {
            blendLane(st, car, lane, (car[0x135] & 0x10u) == 0u ? 6u : 4u);
        }
    }

    if (GP4_GLOBAL(uint8_t, 0x00603e6c) != 0u) {
        gp4::call_regs(0x00425c87, st);
    }
    st.eax = signExtend16(st.eax);
    GP4_GLOBAL(uint32_t, 0x00602d7c) = st.eax;
    gp4::call_regs(0x004056c9, st);
    st.eax = static_cast<uint32_t>(static_cast<int32_t>(st.eax) >> 8);
    if (st.esi == GP4_GLOBAL(uint32_t, 0x00679d2c) && (GP4_GLOBAL(uint16_t, 0x0066d8b4) & 0x200u) != 0u) {
        gp4::call_regs(0x0040f833, st);
    }
    if ((car[0xe5] & 4u) != 0u && (car[0x179] & 4u) == 0u) {
        gp4::call_regs(0x00425b71, st);
    }

    GP4_FIELD(uint16_t, car, 0x100) = static_cast<uint16_t>(st.eax);
    const uint16_t reference = GP4_FIELD(uint16_t, lane, 2);
    st.edx = withLow16(st.edx, reference);
    GP4_FIELD(uint16_t, car, 0x144) = reference;
    const uint16_t value = static_cast<uint16_t>(st.eax);
    const uint16_t diff = static_cast<uint16_t>(value - reference);
    const uint16_t mean = static_cast<uint16_t>(static_cast<uint16_t>(value + reference) >> 1);
    const int32_t product = static_cast<int32_t>(static_cast<int16_t>(diff)) * static_cast<int32_t>(static_cast<int16_t>(mean));
    st.eax = withLow16(st.eax, static_cast<uint32_t>(product));
    st.edx = withLow16(st.edx, static_cast<uint32_t>(product) >> 16);

    GP4_FIELD(uint16_t, car, 0xa2) = savedWordA2;
    GP4_FIELD(uint32_t, car, 0x240) = savedDword240;

    r->eax = st.eax;
    r->ebx = st.ebx;
    r->edx = st.edx;
    r->esi = st.esi;
    r->edi = st.edi;
    r->ebp = st.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | imulFlags16(product);
}
GP4_IMPL(0x00425cb3, FUN_00425cb3, "regs(eax:u32[0..95], edx:u32, esi:ptr[0x420]:layout=car_0042ba87+rec_00409416+car_00429fcd+gears_0041034e+rec_0043259c) -> (eax, ebx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0042595c+ring_008943a0+words_008943a0+words_00413090+engine_00413606+lanes_00429fcd+surf_0041f02b+state_0043c1ba+state_004320f2+counter_0040f0dd+scale_0066ec60")

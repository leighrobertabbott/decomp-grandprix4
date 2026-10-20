#include <gp4/gp4.h>
#include <gp4/x87.h>

// esi: a car record (only the fields copied here are named).
struct Unknown_00418942_Car {
    int32_t Unknown00;
    int32_t Unknown04;
    int32_t Unknown08;
    uint8_t Unknown0C[0xc];
    int16_t Unknown18;
    uint8_t Unknown1A[0x48];
    uint16_t Unknown62;
    uint8_t Unknown64[0x34];
    int32_t Unknown98;
    uint8_t Unknown9C[8];
    int32_t UnknownA4;
    int32_t UnknownA8;
    uint8_t UnknownAC[0x42];
    uint16_t UnknownEE;
    uint8_t UnknownF0[0x60];
    int32_t Unknown150;
    uint8_t Unknown154[0xe3];
    uint8_t Unknown237;
    uint8_t Unknown238[8];
    uint32_t Unknown240;
    uint8_t Unknown244[0x28];
    uint32_t Unknown26C;
    uint8_t Unknown270[0x7c];
    uint32_t Unknown2EC[4];
};

// The 0xa8-byte entry returned by 0x00418901 (0x625abc + 0xa8 * byte esi+0x202).
struct Unknown_00418942_Entry {
    uint32_t Unknown00;
    uint32_t Unknown04;
    uint32_t Unknown08;
    int32_t Unknown0C;
    int32_t Unknown10;
    int32_t Unknown14;
    int32_t Unknown18;
    int32_t Unknown1C;
    int32_t Unknown20;
    int32_t Unknown24;
    uint32_t Unknown28;
    uint32_t Unknown2C;
    uint32_t Unknown30;
    uint32_t Unknown34;
    uint32_t Unknown38[4];
    uint32_t Unknown48[4];
    uint32_t Unknown58[4];
    uint32_t Unknown68[4];
    uint32_t Unknown78[4];
    uint32_t Unknown88;
    int32_t Unknown8C;
    uint32_t Unknown90;
    uint32_t Unknown94;
};

// One of the four 0x300-byte records at 0x006380d8.
struct Unknown_00418942_Record {
    uint8_t Unknown00[0x14];
    int32_t Unknown14;
    uint8_t Unknown18[0x15];
    uint8_t Unknown2D;
    uint8_t Flags2E;
    uint8_t Unknown2F[0x23d];
    int32_t Unknown26C;
    uint8_t Unknown270[0x10];
    double Unknown280;
    double Unknown288;
    uint8_t Unknown290[0x70];
};

// The record base, kept out of line so the per-record fields stay base-relative.
static __declspec(noinline) const Unknown_00418942_Record* records() {
    return reinterpret_cast<const Unknown_00418942_Record*>(GP4_ARRAY(uint8_t, 0x006380d8));
}

// clamp(value, ..0x7000) - offset, floored at 0, scaled by `factor` (fild/fmul/fistp through
// the scratch dword 0x6f2bf4); returns the magnitude.
static inline uint32_t scaledExcess(int32_t value, int32_t offset, double factor) {
    if (value > 0x7000) {
        value = 0x7000;
    }
    int32_t excess = static_cast<int32_t>(static_cast<uint32_t>(value) - static_cast<uint32_t>(offset));
    if (excess < 0) {
        excess = 0;
    }
    GP4_GLOBAL(int32_t, 0x006f2bf4) = excess;
    const int32_t scaled = gp4::x87::fistp32(gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)) * gp4::x87::ext(factor));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = scaled;
    return scaled < 0 ? 0u - static_cast<uint32_t>(scaled) : static_cast<uint32_t>(scaled);
}

// FUNCTION: GP4 0x00418942
// Fills the 0xa8-byte entry of the car at esi (0x00418901) with a snapshot of its state:
// position dwords, word +0x18, the >> 8 values of +0xa4/+0x150/+0xa8 and of max(+0x98, 0), the
// mean of dword +0x14 of the four 0x6380d8 records, byte +0x237, the bytes of +0x240, per record
// bit 0x20 of +0x2e, byte +0x2d, |fistp((min(+0x26c, 0x7000) - [0x62692c])+ * +0x280)| and the
// same with [0x626930] / +0x288, the car dwords +0x2ec.., words +0xee/+0x62; then rebuilds the
// status bits (0x004187d3), copies the entry's first dword to +0x26c and calls 0x00418ef6.
// pushal/popal: every register is preserved; the flags are those left by 0x00418ef6.
void __cdecl FUN_00418942(gp4::Regs* r) {
    Unknown_00418942_Car* car = reinterpret_cast<Unknown_00418942_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs s = *r;
    gp4::call_regs(0x00418901, s);
    Unknown_00418942_Entry* entry = reinterpret_cast<Unknown_00418942_Entry*>(static_cast<uintptr_t>(s.eax));
    const Unknown_00418942_Record* rec = records();

    entry->Unknown10 = car->Unknown00;
    entry->Unknown14 = car->Unknown08;
    entry->Unknown18 = car->Unknown04;
    entry->Unknown0C = car->Unknown18;
    entry->Unknown1C = car->UnknownA4 >> 8;
    entry->Unknown20 = car->Unknown150 >> 8;
    entry->Unknown24 = car->UnknownA8 >> 8;
    int32_t level = car->Unknown98;
    if (level < 0) {
        level = 0;
    }
    entry->Unknown08 = static_cast<uint32_t>(level) >> 8;
    entry->Unknown8C = static_cast<int32_t>(static_cast<uint32_t>(rec[0].Unknown14) + static_cast<uint32_t>(rec[1].Unknown14)
        + static_cast<uint32_t>(rec[2].Unknown14) + static_cast<uint32_t>(rec[3].Unknown14)) >> 2;
    entry->Unknown88 = car->Unknown237;
    const uint32_t packed = car->Unknown240;
    entry->Unknown28 = packed & 0xffu;
    entry->Unknown2C = (packed >> 8) & 0xffu;
    entry->Unknown30 = (packed >> 16) & 0xffu;
    entry->Unknown34 = packed >> 24;

    for (int i = 0; i < 4; ++i) {
        const Unknown_00418942_Record* r4 = rec + i;
        entry->Unknown78[i] = (r4->Flags2E & 0x20u) != 0u ? 1u : 0u;
        entry->Unknown68[i] = r4->Unknown2D;
        entry->Unknown48[i] = scaledExcess(r4->Unknown26C, GP4_GLOBAL(int32_t, 0x0062692c), r4->Unknown280);
        entry->Unknown58[i] = scaledExcess(r4->Unknown26C, GP4_GLOBAL(int32_t, 0x00626930), r4->Unknown288);
        entry->Unknown38[i] = car->Unknown2EC[i];
    }

    entry->Unknown90 = car->UnknownEE;
    entry->Unknown94 = car->Unknown62;

    gp4::Regs t = *r;
    t.edi = s.eax;
    gp4::call_regs(0x004187d3, t);
    car->Unknown26C = entry->Unknown00;
    gp4::call_regs(0x00418ef6, t);

    r->eflags = (r->eflags & ~0x8d5u) | (t.eflags & 0x8d5u);
}
GP4_IMPL(0x00418942, FUN_00418942, "regs(esi:ptr[0x304]:layout=slot_00418942) -> (cf, pf, af, zf, sf, of); globals=layout=recs_006380d8+telem_00418942")

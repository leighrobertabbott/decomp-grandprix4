#include <gp4/gp4.h>

// The 0xa8-byte per-car slot at 0x00625abc (selected by 0x00418901).
struct Unknown_00625abc {
    uint32_t Unknown00;
    uint32_t Status04;          // rebuilt by 0x00418ef6
    uint32_t Unknown08;
    uint32_t Unknown0C;
    uint32_t Unknown10;
    uint32_t Unknown14;
    uint32_t Unknown18;
    uint32_t Unknown1C;
    uint32_t Unknown20;
    uint32_t Unknown24;
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
    uint32_t Unknown8C;
    uint32_t Unknown90;
    uint32_t Unknown94;
    uint32_t Unknown98[4];
};
static_assert(sizeof(Unknown_00625abc) == 0xa8, "layout");

// Q14 cosine of a 16-bit angle: the word at 0x007c5c5c + ((angle >> 2) & 0xfffe).
static inline int32_t cosineWord(uint16_t angle) {
    const uint32_t byteOffset = (static_cast<uint32_t>(angle) >> 2) & 0xfffeu;
    return GP4_FIELD(int16_t, GP4_ARRAY(uint8_t, 0x007c5c5c), byteOffset);
}

// (a * b) >> 14 as the low dword of `imul ; shrd eax, edx, 14`.
static inline uint32_t scaleQ14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x00418afe
// pushal wrapper: fills the car's 0xa8-byte slot (0x00418901: 0x00625abc + byte[esi+0x202] * 0xa8)
// from the car record in esi (position dwords +0 / +4 / +8 into +0x10 / +0x18 / +0x14, the clamped
// dword +0x98 >> 8 into +8, the heading word +0x18 and its Q14 sine / cosine scaled by it into
// +0xc / +0x1c / +0x24, the four bytes of dword +0x240 into +0x28..+0x34, the four dwords at
// 0x0060309c into +0x38.., zeros elsewhere, words +0xee / +0x62 into +0x90 / +0x94 and dword +0x26c
// into +0), then lets 0x00418ef6 rebuild the status dword. Every register is restored by popal; the
// flags are those 0x00418ef6 leaves.
void __cdecl FUN_00418afe(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs find = *r;
    find.st_in = 0;
    find.st_out = 0;
    gp4::call_regs(0x00418901, find);
    Unknown_00625abc* slot = reinterpret_cast<Unknown_00625abc*>(static_cast<uintptr_t>(find.eax));

    slot->Unknown10 = GP4_FIELD(uint32_t, car, 0x00);
    slot->Unknown14 = GP4_FIELD(uint32_t, car, 0x08);
    slot->Unknown18 = GP4_FIELD(uint32_t, car, 0x04);

    int32_t strength = GP4_FIELD(int32_t, car, 0x98);
    if (strength < 0) {
        strength = 0;
    }
    slot->Unknown08 = static_cast<uint32_t>(strength) >> 8;

    const int16_t heading = GP4_FIELD(int16_t, car, 0x18);
    slot->Unknown0C = static_cast<uint32_t>(static_cast<int32_t>(heading));

    uint16_t angle = static_cast<uint16_t>(0x4000u - static_cast<uint16_t>(heading));
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    slot->Unknown1C = scaleQ14(static_cast<int32_t>(slot->Unknown08), cosineWord(angle));
    slot->Unknown20 = 0;

    angle = static_cast<uint16_t>(heading);
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    slot->Unknown24 = scaleQ14(static_cast<int32_t>(slot->Unknown08), cosineWord(angle));

    slot->Unknown88 = GP4_FIELD(uint8_t, car, 0x237);
    const uint32_t packed = GP4_FIELD(uint32_t, car, 0x240);
    slot->Unknown28 = packed & 0xffu;
    slot->Unknown2C = (packed >> 8) & 0xffu;
    slot->Unknown30 = (packed >> 16) & 0xffu;
    slot->Unknown34 = packed >> 24;

    const uint32_t* defaults = GP4_ARRAY(uint32_t, 0x0060309c);
    for (uint32_t i = 0; i != 4; ++i) {
        slot->Unknown78[i] = 0;
        slot->Unknown68[i] = 0;
        slot->Unknown48[i] = 0;
        slot->Unknown58[i] = 0;
        slot->Unknown38[i] = defaults[i];
    }

    slot->Unknown90 = GP4_FIELD(uint16_t, car, 0xee);
    slot->Unknown94 = GP4_FIELD(uint16_t, car, 0x62);
    slot->Unknown8C = 0;
    slot->Unknown00 = GP4_FIELD(uint32_t, car, 0x26c);

    gp4::Regs status = *r;
    status.st_in = 0;
    status.st_out = 0;
    status.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slot));
    gp4::call_regs(0x00418ef6, status);
    r->eflags = (r->eflags & ~0x8d5u) | (status.eflags & 0x8d5u);
}
GP4_IMPL(0x00418afe, FUN_00418afe, "regs(esi:ptr[0x304]:layout=slot_00418942) -> (cf, pf, af, zf, sf, of); globals=layout=telem_00418942")

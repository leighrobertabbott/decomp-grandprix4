#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004ad6b0_Dst {
    uint8_t Unknown000[0x570];
    uint32_t Flags570;
};
struct Unknown_004ad6b0_Src {
    uint8_t Unknown00[0x7c];
    int32_t Unknown7C;
    int32_t Unknown80;
    int32_t Unknown84;
    int32_t Unknown88;
    int32_t Unknown8C;
    int32_t Unknown90;
    int32_t Unknown94;
    int32_t Unknown98;
    int32_t Unknown9C;
    int32_t UnknownA0;
    int32_t UnknownA4;
    int32_t UnknownA8;
    int32_t UnknownAC;
    int32_t UnknownB0;
    int32_t UnknownB4;
    int32_t UnknownB8;
    int32_t UnknownBC;
    int32_t UnknownC0;
    int32_t UnknownC4;
    int32_t UnknownC8;
    int32_t UnknownCC;
    int32_t UnknownD0;
    int32_t UnknownD4;
    int32_t UnknownD8;
    int32_t UnknownDC;
    int32_t UnknownE0;
    int32_t UnknownE4;
};
#pragma pack(pop)

// Packs the boolean dwords +0x7c..+0xe4 of src into one bit mask (bit 31 always set) and
// stores it at dst+0x570.
// FUNCTION: GP4 0x004ad6b0
void __cdecl FUN_004ad6b0(Unknown_004ad6b0_Dst* dst, const Unknown_004ad6b0_Src* src) {
    uint32_t mask = 0x80000000u;
    if (src->UnknownE4 != 0) mask = 0x88000000u;
    if (src->UnknownE0 != 0) mask |= 0x4000000u;
    if (src->UnknownA8 != 0) mask |= 0x1u;
    if (src->UnknownAC != 0) mask |= 0x2u;
    if (src->UnknownB0 != 0) mask |= 0x4u;
    if (src->UnknownB4 != 0) mask |= 0x8u;
    if (src->UnknownB8 != 0) mask |= 0x10u;
    if (src->UnknownBC != 0) mask |= 0x20u;
    if (src->UnknownC0 != 0) mask |= 0x40u;
    if (src->UnknownC4 != 0) mask |= 0x80u;
    if (src->UnknownC8 != 0) mask |= 0x100u;
    if (src->UnknownCC != 0) mask |= 0x200u;
    if (src->UnknownD0 != 0) mask |= 0x400u;
    if (src->UnknownD4 != 0) mask |= 0x800u;
    if (src->UnknownD8 != 0) mask |= 0x1000u;
    if (src->Unknown7C != 0) mask |= 0x2000u;
    if (src->Unknown8C != 0) mask |= 0x4000u;
    if (src->Unknown88 != 0) mask |= 0x8000u;
    if (src->Unknown90 != 0) mask |= 0x10000u;
    if (src->Unknown94 != 0) mask |= 0x20000u;
    if (src->Unknown98 != 0) mask |= 0x40000u;
    if (src->Unknown9C != 0) mask |= 0x80000u;
    if (src->Unknown84 != 0) mask |= 0x100000u;
    if (src->Unknown80 != 0) mask |= 0x200000u;
    if (src->UnknownA0 != 0) mask |= 0x400000u;
    if (src->UnknownA4 != 0) mask |= 0x800000u;
    if (src->UnknownDC != 0) mask |= 0x1000000u;
    dst->Flags570 = mask;
}
GP4_IMPL(0x004ad6b0, FUN_004ad6b0, "cdecl(ptr[0x578]:bytes, ptr[0xf0]:u32{=0,1}) -> void")

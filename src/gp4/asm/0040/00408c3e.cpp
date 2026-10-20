#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a logic result (and / or / test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | (result & 0x80u);
}
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of `left - right` on bytes (cmp byte).
static inline uint32_t subFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of `left - right` on dwords (sub eax, 1).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline void setFlags(gp4::Regs& s, uint32_t flags) {
    s.eflags = (s.eflags & ~0x8d5u) | (flags & 0x8d5u);
}

// Fields of the 0x41c-byte car records at 0x0066fef0 that this routine writes.
struct Unknown_00408c3e_Car {
    uint8_t Unknown00[0x20];
    uint32_t Unknown20;
    uint32_t Unknown24;
    uint32_t Unknown28;
    uint32_t Unknown2C;
    uint32_t Unknown30;
    uint32_t Unknown34;
    uint8_t Unknown38[0x62 - 0x38];
    uint16_t Unknown62;
    uint8_t Unknown64[0x6f - 0x64];
    uint8_t Unknown6F;
    uint8_t Unknown70;
    uint8_t Unknown71;
    uint8_t Unknown72[0x74 - 0x72];
    uint8_t Unknown74;
    uint8_t Unknown75[0x78 - 0x75];
    uint8_t Flags78;
    uint8_t Unknown79;
    uint8_t Flags7A;
    uint8_t Flags7B;
    uint8_t CarNumber7C;
    uint8_t Unknown7D[0x8d - 0x7d];
    uint8_t Flags8D;
    uint8_t Unknown8E[0xa0 - 0x8e];
    uint16_t UnknownA0;
    uint8_t UnknownA2[0xc0 - 0xa2];
    uint32_t UnknownC0;
    uint8_t UnknownC4[0xca - 0xc4];
    uint16_t UnknownCA;
    uint8_t FlagsCC;
    uint8_t UnknownCD[0xd8 - 0xcd];
    uint16_t UnknownD8;
    uint8_t UnknownDA[0xe4 - 0xda];
    uint8_t UnknownE4;
    uint8_t FlagsE5;
    uint8_t UnknownE6[0xfe - 0xe6];
    uint8_t FlagsFE;
    uint8_t UnknownFF[0x162 - 0xff];
    uint8_t Flags162;
    uint8_t Unknown163[0x1bc - 0x163];
    uint16_t Unknown1BC;
    uint8_t Unknown1BE[0x244 - 0x1be];
    uint32_t Unknown244;
    uint8_t Unknown248[0x24c - 0x248];
    uint16_t Unknown24C;
    uint16_t Unknown24E;
};

// esi = the car record being (re)initialised for a race start: clears its status bytes, runs the
// sub-initialisers (0x0041afde, 0x004094e3, 0x00408f42, 0x00410a65, 0x0043cec0, 0x0043d194, 0x00440b67 unless
// byte 0x007c2e75 is set, 0x004326ca, 0x0040882a, 0x00441166, 0x004415c9), sets the default state words and
// copies the three per-car start dwords (tables 0x00675970 / 0x006759c8 / 0x00675a20, index car number - 1)
// into +0x34 / +0x2c / +0x30, then 0x00408659 and the closing flag updates. A car whose table byte
// 0x007c2a73[number] is 2 gets bit 0 of +0x8d. Registers and flags are whatever the last callee and the
// final "or [esi+0x78], 0x20" leave.
// FUNCTION: GP4 0x00408c3e
void __cdecl FUN_00408c3e(gp4::Regs* r) {
    Unknown_00408c3e_Car* car = reinterpret_cast<Unknown_00408c3e_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    car->Unknown71 = 0;
    car->FlagsE5 = 0;
    car->FlagsCC = 0;
    car->UnknownE4 = 0;
    car->Flags7B = 0;
    car->Flags7A = 0;
    car->Unknown6F = 0;
    car->UnknownCA = 0;

    gp4::call_regs(0x0041afde, s);
    gp4::call_regs(0x004094e3, s);

    const int32_t slot = static_cast<int32_t>(car->CarNumber7C & 0x3fu) - 1;
    s.eax = static_cast<uint32_t>(slot);
    const uint8_t kind = GP4_ARRAY(const uint8_t, 0x007c2a74)[slot];
    setFlags(s, subFlags8(kind, 2u));
    if (kind == 2u) {
        car->Flags8D = static_cast<uint8_t>(car->Flags8D | 0x01u);
        setFlags(s, logicFlags8(car->Flags8D));
    }
    gp4::call_regs(0x00408f42, s);
    gp4::call_regs(0x00410a65, s);
    gp4::call_regs(0x0043cec0, s);
    gp4::call_regs(0x0043d194, s);

    const uint8_t skip = GP4_GLOBAL(uint8_t, 0x007c2e75);
    setFlags(s, subFlags8(skip, 0u));
    if (skip == 0u) {
        gp4::call_regs(0x00440b67, s);
    }
    gp4::call_regs(0x004326ca, s);
    gp4::call_regs(0x0040882a, s);
    gp4::call_regs(0x00441166, s);
    gp4::call_regs(0x004415c9, s);

    car->UnknownD8 = 0x7802u;
    car->Unknown62 = 0x2710u;
    car->Unknown244 = 0x005d0000u;
    car->UnknownA0 = 0x4000u;
    car->Unknown24C = 0x1000u;
    car->Unknown24E = 0x1000u;
    car->UnknownC0 = 0xc0000000u;
    car->Unknown28 = 0x10000000u;
    car->Unknown20 = 0x10000000u;
    car->Unknown24 = 0x10000000u;
    car->Unknown74 = 2u;

    const uint32_t number = car->CarNumber7C & 0x3fu;
    setFlags(s, logicFlags32(number));
    const uint32_t index = number - 1u;
    s.eax = index;
    setFlags(s, subFlags32(number, 1u));
    car->Unknown34 = GP4_ARRAY(const uint32_t, 0x00675970)[static_cast<int32_t>(index)];
    car->Unknown2C = GP4_ARRAY(const uint32_t, 0x006759c8)[static_cast<int32_t>(index)];
    car->Unknown30 = GP4_ARRAY(const uint32_t, 0x00675a20)[static_cast<int32_t>(index)];
    gp4::call_regs(0x00408659, s);

    car->Unknown1BC = 0xffffu;
    car->FlagsFE = static_cast<uint8_t>(car->FlagsFE & 0xdfu);
    car->FlagsE5 = static_cast<uint8_t>(car->FlagsE5 | 0x04u);
    car->Flags7B = static_cast<uint8_t>(car->Flags7B & 0xfdu);
    car->Flags162 = static_cast<uint8_t>(car->Flags162 | 0x01u);
    car->Flags78 = static_cast<uint8_t>(car->Flags78 | 0x20u);
    setFlags(s, logicFlags8(car->Flags78));

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x00408c3e, FUN_00408c3e, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, edi:rec[0x8943a0,0x160,0..11], esi:ptr[0x420]:layout=rec_0040882a+rec_004087aa+lvl_00431d1f) -> (eax, ebx, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); cases=400; globals=layout=cars_0066fef0+list_0066fe3c+state_00412af5+state_00403800+ranges_004322a6+ring_00405a12+links_00405a12+state_004059ef+state_004094e3+div_00403bb6+trk_007ad894+circ_007adb50+lvl_007adb86+codes_0043ba12+curves_0062ecb0+wheel_00410a1c+state_0066d8c0+words_0062d210+state_0042595c+carno_0066ff6c+state_0040f112+gain_00408790+state_004087aa+div_0060138c+state_0040860a+state_00405f77+levels_006f2cc4+tbl_0066d94c+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043c4d9+tabs_0043ced2+state_00411109+scale_0066ec60+keys_00408c3e")

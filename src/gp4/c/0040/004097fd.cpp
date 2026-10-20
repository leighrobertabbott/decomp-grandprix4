#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (or / and / test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | (result & 0x80u);
}

// Fields of the car record (esi) this routine writes.
struct Unknown_004097fd_Car {
    uint8_t Unknown00[0x70];
    uint8_t Unknown70;
    uint8_t Flags71;
    uint8_t Unknown72;
    uint8_t Unknown73[0x7b - 0x73];
    uint8_t Flags7B;
    uint8_t Number7C;
    uint8_t Unknown7D[0x8d - 0x7d];
    uint8_t Flags8D;
    uint8_t Unknown8E[0xe4 - 0x8e];
    uint8_t UnknownE4;
    uint8_t FlagsE5;
    uint8_t UnknownE6[0xea - 0xe6];
    uint16_t StateEA;
    uint8_t UnknownEC[0xf0 - 0xec];
    uint32_t UnknownF0;
    uint8_t UnknownF4[0xfe - 0xf4];
    uint8_t FlagsFE;
    uint8_t UnknownFF[0x162 - 0xff];
    uint8_t Flags162;
    uint8_t Unknown163[0x26b - 0x163];
    uint8_t Flags26B;
};

// FUNCTION: GP4 0x004097fd
// Puts the car at esi into the "pit stop / parked" state: the start position is derived from the
// segment record of the car's slot (0x0043207c) shifted by 0x4320b2's offset (the sign of
// byte 0x868582 picks the side; only when bit 3 of +0x26b is set), then 0x00408c3e resets the car,
// the state word +0xea becomes 0x20, 0x00409754 loads the channel words and a run of flag bytes is
// updated. The car number (+0x7c & 0x3f) - 1 indexes the byte table 0x0066d90a whose bit 7 is set.
// pushal / popal keep every register; the flags are those of the final `or`.
void __cdecl FUN_004097fd(gp4::Regs* r) {
    Unknown_004097fd_Car* car = reinterpret_cast<Unknown_004097fd_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    gp4::call_regs(0x0043207c, s);
    s.edi = s.eax;
    s.eax = GP4_GLOBAL(uint32_t, 0x00627870);
    s.edx = 0;

    bool shifted = false;
    if (GP4_GLOBAL(int8_t, 0x00868582) < 0) {
        s.eax = 0u - s.eax;
        s.ecx = 0x200;
        if ((car->Flags26B & 8u) != 0u) {
            gp4::call_regs(0x004320b2, s);
            s.eax -= GP4_GLOBAL(uint32_t, 0x0062d2ac);
            s.edx = 0x4000u + GP4_GLOBAL(uint32_t, 0x0062d2b0);
            shifted = true;
        }
    } else if ((car->Flags26B & 8u) != 0u) {
        gp4::call_regs(0x004320b2, s);
        s.eax += GP4_GLOBAL(uint32_t, 0x0062d2ac);
        s.edx = 0xffffc000u + GP4_GLOBAL(uint32_t, 0x0062d2b0);
        shifted = true;
    }
    if (shifted) {
        s.ecx = 0;
        const uint32_t saved = s.eax;
        s.eax = s.edi;
        gp4::call_regs(0x00432095, s);
        s.edi = s.eax;
        s.eax = saved;
    }

    gp4::call_regs(0x00408c3e, s);
    car->StateEA = 0x20;
    gp4::call_regs(0x00409754, s);

    car->UnknownE4 = 2;
    car->Flags8D = static_cast<uint8_t>(car->Flags8D | 0x80u);
    car->UnknownE4 = 2;
    car->Unknown72 = 0;
    car->Flags71 = static_cast<uint8_t>(car->Flags71 | 0x80u);
    car->FlagsFE = static_cast<uint8_t>(car->FlagsFE | 0x04u);
    car->Flags8D = static_cast<uint8_t>(car->Flags8D | 0x02u);
    car->Flags7B = static_cast<uint8_t>(car->Flags7B & 0xfdu);
    car->FlagsFE = static_cast<uint8_t>(car->FlagsFE & 0xdfu);
    car->Flags71 = static_cast<uint8_t>(car->Flags71 | 0x02u);
    car->Flags7B = static_cast<uint8_t>(car->Flags7B | 0x40u);
    car->Flags162 = static_cast<uint8_t>(car->Flags162 | 0x40u);
    if (static_cast<int8_t>(car->Number7C) >= 0) {
        car->FlagsE5 = static_cast<uint8_t>(car->FlagsE5 | 0x04u);
    }
    car->UnknownF0 = GP4_GLOBAL(uint32_t, 0x0066d814);
    car->UnknownF0 = car->UnknownF0 + 1u;
    car->Unknown70 = 1;

    const int32_t index = static_cast<int32_t>(car->Number7C & 0x3fu) - 1;
    uint8_t* slot = &GP4_ARRAY(uint8_t, 0x0066d90a)[index];
    *slot = static_cast<uint8_t>(*slot | 0x80u);

    // popal restores every register (r keeps the entry values); the flags stay those of the `or`.
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(*slot);
}
GP4_IMPL(0x004097fd, FUN_004097fd, "regs(ebx:u32, ecx:u32, ebp:u32, esi:ptr[0x420]:layout=rec_0040882a+rec_004087aa+lvl_00431d1f+car_00431ffb) -> (cf, pf, af, zf, sf, of); cases=400; globals=fuzz:i32+layout=cars_0066fef0+list_0066fe3c+state_00412af5+state_00403800+ranges_004322a6+ring_00405a12+links_00405a12+state_004059ef+state_004094e3+div_00403bb6+trk_007ad894+circ_007adb50+lvl_007adb86+codes_0043ba12+curves_0062ecb0+wheel_00410a1c+state_0066d8c0+words_0062d210+state_0042595c+carno_0066ff6c+state_0040f112+gain_00408790+state_004087aa+div_0060138c+state_0040860a+state_00405f77+levels_006f2cc4+tbl_0066d94c+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043c4d9+tabs_0043ced2+state_00411109+scale_0066ec60+keys_00408c3e+state_00431ffb")

#include <gp4/gp4.h>

struct Unknown_004098fd_Ring {
    uint8_t Unknown00[0x16];
    int16_t Unknown16;
    int16_t Unknown18;
};

// Car record fields touched here (offsets from the esi record base).
struct Unknown_004098fd_Car {
    uint8_t Unknown00[0x70];
    uint8_t Unknown70;
    uint8_t Unknown71;
    uint8_t Unknown72;
    uint8_t Unknown73[7];
    uint8_t Unknown7A;
    uint8_t Unknown7B;
    uint8_t Unknown7C;
    uint8_t Unknown7D[0x66];
    uint8_t UnknownE3;
    uint8_t UnknownE4;
    uint8_t UnknownE5;
    uint8_t UnknownE6[4];
    uint16_t UnknownEA;
    uint8_t UnknownEC[0x12];
    uint8_t UnknownFE;
    uint8_t UnknownFF[0x63];
    uint8_t Unknown162;
};

static inline uint32_t parityFlag8(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// pushal; edi = [0x866ecc] - word[0x62d256] * 0x160 (a 0x8943a0 ring record); calls 0x00408c3e with
// eax = sx word [edi+0x16], edx = sx word [edi+0x18], ecx = 0 and the entry esi/ebx/ebp; then sets
// flags in the car record at esi and marks the car's slot in the byte table at 0x0066d909; popal.
// The flags that escape are those of the last "or byte [ebx+0x66d90a], 0x80".
// FUNCTION: GP4 0x004098fd
void __cdecl FUN_004098fd(gp4::Regs* r) {
    const uint32_t index = GP4_GLOBAL(uint16_t, 0x0062d256);
    const uint32_t ringAddr = GP4_GLOBAL(uint32_t, 0x00866ecc) - index * 0x160u;
    const Unknown_004098fd_Ring* ring = reinterpret_cast<const Unknown_004098fd_Ring*>(ringAddr);

    gp4::Regs c = *r;
    c.edi = ringAddr;
    c.ecx = 0;
    c.edx = static_cast<uint32_t>(static_cast<int32_t>(ring->Unknown18));
    c.eax = static_cast<uint32_t>(static_cast<int32_t>(ring->Unknown16));
    gp4::call_regs(0x00408c3e, c);

    Unknown_004098fd_Car* car = reinterpret_cast<Unknown_004098fd_Car*>(c.esi);
    car->UnknownEA = 0;
    car->Unknown72 = 0;
    car->Unknown162 |= 0x40;
    car->Unknown7B |= 2;
    car->UnknownFE |= 0x20;
    if (static_cast<int8_t>(car->Unknown7C) >= 0) {
        car->UnknownE5 |= 4;
    }
    car->Unknown70 = 0;

    const uint32_t slot = (static_cast<uint32_t>(car->Unknown7C) & 0x3fu) - 1u;
    uint8_t* flagByte = GP4_ARRAY(uint8_t, 0x0066d90a) + slot;
    const uint8_t marked = static_cast<uint8_t>(*flagByte | 0x80u);
    *flagByte = marked;

    r->eflags = (r->eflags & ~0x8d5u) | parityFlag8(marked) | 0x80u;
}
GP4_IMPL(0x004098fd, FUN_004098fd, "regs(ebx:u32, ebp:u32, esi:ptr[0x420]:layout=rec_0040882a+rec_004087aa+lvl_00431d1f) -> (cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0+list_0066fe3c+state_00412af5+state_00403800+ranges_004322a6+ring_00405a12+links_00405a12+state_004059ef+state_004094e3+div_00403bb6+trk_007ad894+circ_007adb50+lvl_007adb86+codes_0043ba12+curves_0062ecb0+wheel_00410a1c+state_0066d8c0+words_0062d210+state_0042595c+carno_0066ff6c+state_0040f112+gain_00408790+state_004087aa+div_0060138c+state_0040860a+state_00405f77+levels_006f2cc4+tbl_0066d94c+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043c4d9+tabs_0043ced2+state_00411109+scale_0066ec60+keys_00408c3e+start_004098fd")

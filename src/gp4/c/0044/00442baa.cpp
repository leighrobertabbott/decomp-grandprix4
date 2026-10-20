#include <gp4/gp4.h>

struct Unknown_00442baa_Car {
    uint8_t Unknown00[0x7b];
    uint8_t Flags7B;
    uint8_t Unknown7C[0x82];
    uint8_t FlagsFE;
};

static inline uint32_t parityFlag8(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit TEST against 0xff (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint8_t value) {
    return parityFlag8(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// Per-car reset used when a car is (re)placed: esi = the car record [0x679d34]; the three words
// 0x66d8be/c0/c2 are saved across the work (push eax x3 / pop eax x3, so eax leaves with the
// upper half of the entry eax and the first word), 0x442c41 / 0x408742 / 0x4097c7 / 0x409b87 /
// 0x410d04 run on the car (0x410d04 leaves a new esi/ebx), +0x7b bit 1 and +0xfe bit 5 are set when
// 0x7c2e75 is nonzero, 0x852d3c takes 0x679d30, and 0x444c89 runs when 0x7aafd4 is nonzero.
// The flags that escape are those of "test byte [0x7aafd4], 0xff" or of 0x444c89.
// FUNCTION: GP4 0x00442baa
void __cdecl FUN_00442baa(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.esi = GP4_GLOBAL(uint32_t, 0x00679d34);

    c.eax = (c.eax & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x0066d8be);
    const uint32_t savedBe = c.eax;
    c.eax = (c.eax & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x0066d8c0);
    const uint32_t savedC0 = c.eax;
    c.eax = (c.eax & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x0066d8c2);
    const uint32_t savedC2 = c.eax;

    gp4::call_regs(0x00442c41, c);
    gp4::call_regs(0x00408742, c);
    c.eax = (c.eax & ~0xffu) | GP4_GLOBAL(uint8_t, 0x0066d897);
    GP4_GLOBAL(uint8_t, 0x0066d896) = static_cast<uint8_t>(c.eax);
    gp4::call_regs(0x004097c7, c);
    gp4::call_regs(0x00409b87, c);
    gp4::call_regs(0x00410d04, c);

    if (GP4_GLOBAL(uint8_t, 0x007c2e75) != 0) {
        Unknown_00442baa_Car* car = reinterpret_cast<Unknown_00442baa_Car*>(static_cast<uintptr_t>(c.esi));
        car->Flags7B |= 2;
        car->FlagsFE |= 0x20;
    }
    GP4_GLOBAL(uint32_t, 0x00852d3c) = GP4_GLOBAL(uint32_t, 0x00679d30);
    GP4_GLOBAL(uint8_t, 0x00852d57) = 0;
    GP4_GLOBAL(uint8_t, 0x00852d5c) = 0;

    c.eax = savedC2;
    GP4_GLOBAL(uint16_t, 0x0066d8c2) = static_cast<uint16_t>(c.eax);
    c.eax = savedC0;
    GP4_GLOBAL(uint16_t, 0x0066d8c0) = static_cast<uint16_t>(c.eax);
    c.eax = savedBe;
    GP4_GLOBAL(uint16_t, 0x0066d8be) = static_cast<uint16_t>(c.eax);
    GP4_GLOBAL(uint8_t, 0x0095404e) = 0xff;

    const uint8_t gate = GP4_GLOBAL(uint8_t, 0x007aafd4);
    c.eflags = (c.eflags & ~0x8d5u) | testFlags8(gate);
    if (gate != 0) {
        gp4::call_regs(0x00444c89, c);
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00442baa, FUN_00442baa, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, cf, pf, af, zf, sf, of); cases=300; globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_0041104f+flag_007a97a0+cars_0066fef0+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_0040705f+frac_0040705f+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+state_0040759e+trackfar_008943a0+segflag_004381ab+ctl_007ab548+carst_0040f6d0+carf_00410d04+list_0066fe3c+state_00412af5+state_004059ef+trk_007ad894+circ_007adb50+lvl_007adb86+codes_0043ba12+curves_0062ecb0+wheel_00410a1c+state_0066d8c0+words_0062d210+state_0042595c+carno_0066ff6c+state_0040f112+gain_00408790+state_004087aa+div_0060138c+state_0040860a+state_00405f77+levels_006f2cc4+tbl_0066d94c+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043c4d9+tabs_0043ced2+state_00411109+scale_0066ec60+keys_00408c3e+state_00431ffb+start_004098fd+counter_0040f0dd+counts_0066ec49+flags_00408f7b+carx_0042fe2f+stream_0062d1a0+rec_004450e5+car_00442baa+win_0041e069+place_00442baa")

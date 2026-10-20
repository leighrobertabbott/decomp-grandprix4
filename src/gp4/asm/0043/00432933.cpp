#include <gp4/gp4.h>

// Car record fields cleared or set by this reset (flag bytes unless noted).
struct Unknown_00432933_Car {
    uint8_t Unknown00[0x40];
    uint32_t Unknown40;
    uint32_t Unknown44;
    uint8_t Unknown48[0x80 - 0x48];
    uint8_t Flags80;
    uint8_t Flags81;
    uint8_t Flags82;
    uint8_t Flags83;
    uint8_t Unknown84[0x87 - 0x84];
    uint8_t Flags87;
    uint8_t Unknown88[0x8d - 0x88];
    uint8_t Flags8D;
    uint8_t Unknown8E[0xcd - 0x8e];
    uint8_t FlagsCD;
    uint8_t FlagsCE;
    uint8_t UnknownCF[0x161 - 0xcf];
    uint8_t Flags161;
    uint8_t Unknown162[0x168 - 0x162];
    uint16_t Unknown168;
    uint8_t Unknown16A[0x203 - 0x16a];
    uint8_t Flags203;
};

// pushal ... popal wrapper on the car record in esi: runs 0x00410894 when bit 4 of +0x87 is set, clears the
// transient state bits (+0x80, +0x81, +0x87, +0xcd, +0xce, +0x161, +0x203 and, when bit 4 of +0x8d is set,
// +0x8d / +0x161 / +0x82), zeroes the dwords +0x40 / +0x44, runs 0x0040fbf3, sets +0x82 bit 0 and +0x83
// bit 5, zeroes the word +0x168 and runs 0x004087aa. popal restores every register; the flags are those
// 0x004087aa leaves.
// FUNCTION: GP4 0x00432933
void __cdecl FUN_00432933(gp4::Regs* r) {
    Unknown_00432933_Car* const car = reinterpret_cast<Unknown_00432933_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    if ((car->Flags87 & 0x10u) != 0u) {
        gp4::call_regs(0x00410894, c);
    }

    car->Flags203 = static_cast<uint8_t>(car->Flags203 & 0xefu);
    car->Flags81 = static_cast<uint8_t>(car->Flags81 & 0xdfu);
    car->Flags80 = static_cast<uint8_t>(car->Flags80 & 0xbfu);
    car->Flags80 = static_cast<uint8_t>(car->Flags80 & 0x7fu);
    car->Unknown40 = 0u;
    car->Unknown44 = 0u;
    car->Flags80 = static_cast<uint8_t>(car->Flags80 & 0xdfu);
    car->Flags80 = static_cast<uint8_t>(car->Flags80 & 0xefu);
    car->Flags80 = static_cast<uint8_t>(car->Flags80 & 0xf7u);
    car->Flags80 = static_cast<uint8_t>(car->Flags80 & 0xfbu);
    car->FlagsCE = static_cast<uint8_t>(car->FlagsCE & 0xf7u);
    car->FlagsCE = static_cast<uint8_t>(car->FlagsCE & 0xfdu);
    car->Flags161 = static_cast<uint8_t>(car->Flags161 & 0xbfu);
    car->Flags87 = static_cast<uint8_t>(car->Flags87 & 0x7fu);
    car->Flags87 = static_cast<uint8_t>(car->Flags87 & 0xbfu);
    car->Flags87 = static_cast<uint8_t>(car->Flags87 & 0xfbu);
    car->Flags87 = static_cast<uint8_t>(car->Flags87 & 0xdfu);
    car->Flags87 = static_cast<uint8_t>(car->Flags87 & 0xefu);
    car->Flags87 = static_cast<uint8_t>(car->Flags87 & 0xf7u);
    car->FlagsCD = static_cast<uint8_t>(car->FlagsCD & 0xfbu);
    if ((car->Flags8D & 0x10u) != 0u) {
        car->Flags161 = static_cast<uint8_t>(car->Flags161 & 0xfbu);
        car->Flags8D = static_cast<uint8_t>(car->Flags8D & 0xefu);
        car->Flags82 = static_cast<uint8_t>(car->Flags82 & 0xbfu);
    }

    gp4::call_regs(0x0040fbf3, c);

    car->Flags82 = static_cast<uint8_t>(car->Flags82 | 1u);
    car->Flags83 = static_cast<uint8_t>(car->Flags83 | 0x20u);
    car->Unknown168 = 0;

    gp4::call_regs(0x004087aa, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00432933, FUN_00432933, "regs(esi:ptr[0x358]:layout=car_00431ffb+rec_00410894+rec_004087aa) -> (cf,pf,af,zf,sf,of); globals=fuzz:i32+layout=state_00410894+state_004087aa+trk_007ad894+circ_007adb50+codes_0043ba12+curves_0062ecb0+div_0060138c")

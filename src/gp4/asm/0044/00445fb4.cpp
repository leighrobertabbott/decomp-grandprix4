#include <gp4/gp4.h>

struct Unknown_Car {
    uint8_t Unknown00[0x8d];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x57];
    uint8_t FlagsE5;
    uint8_t Unknown_E6[0x32e];
    uint8_t Flags414;
};

// Retires the car record in esi: clears the top three bits of byte +0x414, forgets it as the car pointer
// 0x0062f8b0 and, when it is the car pointer 0x0066d820, releases it (0x00410d92, 0x0040f9e6, bit 5 of
// +0x8d cleared); sets bit 2 of +0xe5, then runs 0x0040f6d0 and 0x00434d11. pushal / popal restore every
// register; the flags are those 0x00434d11 leaves.
// FUNCTION: GP4 0x00445fb4
void __cdecl FUN_00445fb4(gp4::Regs* r) {
    Unknown_Car* car = reinterpret_cast<Unknown_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    car->Flags414 &= 0x1f;
    if (r->esi == GP4_GLOBAL(uint32_t, 0x0062f8b0)) {
        GP4_GLOBAL(uint32_t, 0x0062f8b0) = 0u;
    }
    car->FlagsE5 |= 0x04;
    if (r->esi == GP4_GLOBAL(uint32_t, 0x0066d820)) {
        gp4::call_regs(0x00410d92, c);
        gp4::call_regs(0x0040f9e6, c);
        GP4_GLOBAL(uint32_t, 0x0066d820) = 0u;
        car->Flags8D &= 0xdf;
    }
    gp4::call_regs(0x0040f6d0, c);
    gp4::call_regs(0x00434d11, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00445fb4, FUN_00445fb4, "regs(esi:ptr[0x420]:layout=car_004129fa+rec_00412b88) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+state_0040759e+trackfar_008943a0+state_00434602+sel_0062f8b0")

#include <gp4/gp4.h>

// Car record fields touched directly: gear +0x72 (copied to +0x121), flags +0x6f, +0xe5, +0xff.
struct Unknown_00416487_Car {
    uint8_t Unknown00[0x6f];
    uint8_t Flags6F;
    uint8_t Unknown70[0x72 - 0x70];
    uint8_t Gear72;
    uint8_t Unknown73[0xe5 - 0x73];
    uint8_t FlagsE5;
    uint8_t UnknownE6[0xff - 0xe6];
    uint8_t FlagsFF;
    uint8_t Unknown100[0x121 - 0x100];
    uint8_t Gear121;
};

// Sum of the two table dwords +0x40 and +0x340 of the array at 0x006380d8 (reached through a register in the original).
static __declspec(noinline) int32_t pairSum(uint8_t* table) {
    return static_cast<int32_t>(GP4_FIELD(uint32_t, table, 0x40) + GP4_FIELD(uint32_t, table, 0x340));
}

// Gear ratio (0x0041242f of the gear byte +0x72 with the byte table at +0x1c6) and the scaled value
// |(([+0x40] + [+0x340]) >> 1) + [0x00638ce0]| * ratio >> 22 (imul; shrd 22), kept in 0x00638d38 / 0x00638dc8.
// Leaves eax = the scaled value, edx = the high dword of the product and ebx = 0x006380d8.
static void gearScale(gp4::Regs& c, const Unknown_00416487_Car* car) {
    c.eax = (c.eax & 0xffffff00u) | car->Gear72;
    c.edx = c.esi + 0x1c6u;
    gp4::call_regs(0x0041242f, c);
    GP4_GLOBAL(uint32_t, 0x00638d38) = c.eax;

    int32_t value = (pairSum(reinterpret_cast<uint8_t*>(0x006380d8)) >> 1) +
                    GP4_GLOBAL(int32_t, 0x00638ce0);
    if (value < 0) {
        value = static_cast<int32_t>(0u - static_cast<uint32_t>(value));
    }
    const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00638d38));
    const uint32_t high = static_cast<uint32_t>(product >> 32);
    const uint32_t low = static_cast<uint32_t>(product);
    c.eax = (low >> 22) | (high << 10);
    c.edx = high;
    c.ebx = 0x006380d8u;
    GP4_GLOBAL(uint32_t, 0x00638dc8) = c.eax;
}

// Per-frame update of one car's drivetrain state, in order: the gear ratio / scaled slip value
// (0x0041242f), 0x004157aa, the gear is mirrored to +0x121; with bit 0x10 of +0x6f clear bits 0x04 of +0xff
// and 0x02 of +0xe5 are cleared, otherwise 0x004161fb runs and the scaled value is recomputed; then
// 0x0041bb95, 0x0041ae14 (unless [0x00622424] is nonzero), [0x006034ac] = 0 and 0x0041bee6, whose registers
// and flags are what is left.
// FUNCTION: GP4 0x00416487
void __cdecl FUN_00416487(gp4::Regs* r) {
    Unknown_00416487_Car* const car = reinterpret_cast<Unknown_00416487_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gearScale(c, car);
    gp4::call_regs(0x004157aa, c);

    c.eax = (c.eax & 0xffffff00u) | car->Gear72;
    car->Gear121 = car->Gear72;
    if ((car->Flags6F & 0x10u) == 0u) {
        car->FlagsFF = static_cast<uint8_t>(car->FlagsFF & 0xfbu);
        car->FlagsE5 = static_cast<uint8_t>(car->FlagsE5 & 0xfdu);
    } else {
        gp4::call_regs(0x004161fb, c);
        gearScale(c, car);
    }

    gp4::call_regs(0x0041bb95, c);
    if (GP4_GLOBAL(uint8_t, 0x00622424) == 0u) {
        gp4::call_regs(0x0041ae14, c);
    }
    GP4_GLOBAL(uint32_t, 0x006034ac) = 0u;
    gp4::call_regs(0x0041bee6, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00416487, FUN_00416487, "regs(eax:u32,ecx:u32,edi:rec[0x8943a0,0x160,0..26],esi:ptr[0x420]:layout=rec_00412b88+rec_0041b950+rec_0041bb36+rec_00426947+car_0041aea9) -> (eax,ebx,ecx,edx,ebp,cf,pf,af,zf,sf,of); globals=fuzz:i32+layout=state_0041bce9+state_0041b950+cars_0066fef0+rel_0042c0c4+state_0043c677+pit_0040d9e6+trk_007ad894+state_00427880+state_00426947+frac_00624790+state_0041242f+thr_007c28bc")

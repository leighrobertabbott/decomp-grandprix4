#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit `cmp value, 0`.
static inline uint32_t cmpZeroFlags8(uint32_t value) {
    value &= 0xffu;
    return parityFlag8(value)
        | (value == 0u ? 0x40u : 0u)
        | (value & 0x80u);
}

// Fields of the car record (esi) this routine reads / writes.
struct Unknown_00421ca5_Car {
    int32_t Unknown00;
    int32_t Unknown04;
    uint8_t Unknown08[0x40 - 0x08];
    uint32_t Flags40;
    uint8_t Unknown44[0xcd - 0x44];
    uint8_t FlagsCD;
    uint8_t UnknownCE[0x200 - 0xce];
    uint8_t Unknown200;
    uint8_t Unknown201[0x25e - 0x201];
    uint8_t ObjectSlot25E;
};

// Sum of the dword at +0xe8 of the four 0x300-byte wheel records at `base` (0x006380d8).
static __declspec(noinline) int32_t sumWheelRecords(uint8_t* base) {
    return GP4_FIELD(int32_t, base, 0xe8)
        + GP4_FIELD(int32_t, base, 0x3e8)
        + GP4_FIELD(int32_t, base, 0x6e8)
        + GP4_FIELD(int32_t, base, 0x9e8);
}

// FUNCTION: GP4 0x00421ca5
// Runs the "contact point" evaluation for the car at esi (pushal / popal: only memory and the
// flags change). The mean of the four wheel-record values at +0xe8 (>> 2, + 0x4000, >> 1) goes to
// 0x609754; 0x421c1b converts the fixed-point inputs; the selector 0x60827d follows +0xcd bits 6 / 4
// and the constant 0x6080e0 is picked by +0x40 & 0x14280; the car's position and the object record
// (0x63bd7c + byte +0x25e * 0x78) go through 0x4241c4, 0x424462 and, by [0x60827f], either
// 0x401fe0 + 0x4238ac or 0x424234; with [0x60827f] clear 0x422056 runs last and [0x60827c] is
// stored at +0x200.
void __cdecl FUN_00421ca5(gp4::Regs* r) {
    Unknown_00421ca5_Car* car = reinterpret_cast<Unknown_00421ca5_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(uint32_t, 0x00609840) = 0;
    if (GP4_GLOBAL(uint8_t, 0x0060827f) != 0u) {
        gp4::call_regs(0x0042379d, c);
    }

    c.ebx = 0x006380d8u;
    int32_t mean = sumWheelRecords(GP4_ARRAY(uint8_t, 0x006380d8));
    mean >>= 2;
    mean += 0x4000;
    mean >>= 1;
    c.eax = static_cast<uint32_t>(mean);
    GP4_GLOBAL(int32_t, 0x00609754) = mean;
    GP4_GLOBAL(uint8_t, 0x006097a8) = 0;
    GP4_GLOBAL(uint32_t, 0x00607fbc) = 0;
    GP4_GLOBAL(uint32_t, 0x00607fc0) = 0x3000;

    gp4::call_regs(0x00421c1b, c);

    GP4_GLOBAL(uint8_t, 0x0060827d) = 0;
    if ((car->FlagsCD & 0x40u) != 0u) {
        c.eax = GP4_GLOBAL(uint32_t, 0x006080e4);
        if ((car->Flags40 & 0x14280u) == 0u) {
            c.eax = GP4_GLOBAL(uint32_t, 0x006080e8);
        }
        GP4_GLOBAL(uint32_t, 0x006080e0) = c.eax;
        if ((car->FlagsCD & 0x10u) == 0u) {
            GP4_GLOBAL(uint8_t, 0x0060827d) = 0xff;
        } else {
            GP4_GLOBAL(uint8_t, 0x0060827d) = 1;
        }
    }
    c.eax = static_cast<uint32_t>(car->Unknown00);
    GP4_GLOBAL(uint32_t, 0x006082a0) = c.eax;
    c.eax = GP4_GLOBAL(uint32_t, 0x00637fe4);
    GP4_GLOBAL(uint32_t, 0x006082a4) = c.eax;
    c.eax = static_cast<uint32_t>(car->Unknown04);
    GP4_GLOBAL(uint32_t, 0x006082a8) = c.eax;
    GP4_GLOBAL(uint8_t, 0x0063963d) = 0;

    const uint32_t savedEsi = c.esi;
    c.ebx = c.esi;
    c.eax = car->ObjectSlot25E;
    c.esi = c.eax * 0x78u + 0x0063bd7cu;
    gp4::call_regs(0x004241c4, c);
    GP4_GLOBAL(uint8_t, 0x0063963c) = 0xff;
    gp4::call_regs(0x00424462, c);
    if (GP4_GLOBAL(uint8_t, 0x0060827f) != 0u) {
        gp4::call_regs(0x00401fe0, c);
        gp4::call_regs(0x004238ac, c);
    } else {
        gp4::call_regs(0x00424234, c);
    }
    c.esi = savedEsi;

    uint32_t flags = cmpZeroFlags8(GP4_GLOBAL(uint8_t, 0x0060827f));
    if (GP4_GLOBAL(uint8_t, 0x0060827f) == 0u) {
        gp4::call_regs(0x00422056, c);
        flags = c.eflags;
        c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x0060827c);
        car->Unknown200 = static_cast<uint8_t>(c.eax);
    }

    // popal restores every register; the flags stay those of the last flag writer.
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x00421ca5, FUN_00421ca5, "regs(esi:ptr[0x300]:layout=car_0041d795) -> (cf, pf, af, zf, sf, of); cases=300; globals=fuzz:i32+layout=objv_0063bd7c+recs_0063bd7c+state_00422056+state_004238ac+trackgeo_008943a0+tbl_00868598+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+gridlinks_008943a0+state_0040759e+trackfar_008943a0+seg_00424e1b+seg_0043b127+rate_00603854+state_0042458b+blk_00424553+tag_0042456e+view_00609860+state_0042430b+state_00422ac2+state_00424011+state_00422397+recs_00621aa0+state_004250b9+state_004250f1+state_00422bc9+state_00421fc3+state_0042320a+divide_00609834+state_004240ff+gain_004240ff+state_00421a06+objs_00424e7f")

#include <gp4/gp4.h>

// |value| the way `test eax,eax ; jns ; neg eax` computes it (0x80000000 stays negative when read signed).
static inline uint32_t absWord(uint32_t value) {
    return (value & 0x80000000u) != 0u ? 0u - value : value;
}

// FUNCTION: GP4 0x0041d645
// Per-frame update of the car record in esi: refresh the rotation of the three angle blocks, then (unless
// 0x00622424 is set) either latch the "far from the grid slot" flags (bit 2 of +0xcc clear: both position
// deltas 0x638034 / 0x638038 reach their limits) or clear the flag again and run 0x0042283b when the car is
// back inside the slot limits; finally the tail routines run on the sine/cosine tables.
void __cdecl FUN_0041d645(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x00401fb4, c);
    gp4::call_regs(0x00402906, c);
    gp4::call_regs(0x00401fe0, c);

    if (GP4_GLOBAL(uint8_t, 0x00622424) == 0u) {
        const uint32_t dx = absWord(GP4_GLOBAL(uint32_t, 0x00638034));
        const uint32_t dz = absWord(GP4_GLOBAL(uint32_t, 0x00638038));
        if ((GP4_FIELD(uint8_t, car, 0xcc) & 4u) == 0u) {
            if (dx >= GP4_GLOBAL(uint32_t, 0x00608068) || dz >= GP4_GLOBAL(uint32_t, 0x0060806c)) {
                GP4_FIELD(uint8_t, car, 0xcc) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0xcc) | 4u);
                GP4_FIELD(uint8_t, car, 0xcd) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0xcd) | 8u);
                GP4_FIELD(uint8_t, car, 0x200) = 0u;
                gp4::call_regs(0x00425027, c);
            }
        } else if ((GP4_FIELD(uint32_t, car, 0x2ec) | GP4_FIELD(uint32_t, car, 0x2f4) |
                    GP4_FIELD(uint32_t, car, 0x2f8)) != 0u) {
            if (dx < GP4_GLOBAL(uint32_t, 0x00608070) && dz < GP4_GLOBAL(uint32_t, 0x00608074)) {
                const int32_t a = static_cast<int32_t>(absWord(GP4_FIELD(uint32_t, car, 0x154)));
                const int32_t b = static_cast<int32_t>(absWord(GP4_FIELD(uint32_t, car, 0x15c)));
                const int32_t wide = a < b ? b : a;
                if (wide < GP4_GLOBAL(int32_t, 0x00608078)) {
                    const int32_t mid = static_cast<int32_t>(absWord(GP4_FIELD(uint32_t, car, 0x158)));
                    if (mid < GP4_GLOBAL(int32_t, 0x0060807c)) {
                        GP4_FIELD(uint8_t, car, 0xcc) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0xcc) & 0xfbu);
                        gp4::call_regs(0x0042283b, c);
                    }
                }
            }
        }
    }

    gp4::call_regs(0x0041d4e5, c);
    GP4_GLOBAL(uint32_t, 0x0063803c) = 0u - GP4_GLOBAL(uint32_t, 0x00623ebc);
    gp4::call_regs(0x0041d22c, c);
    gp4::call_regs(0x0041d43f, c);
    gp4::call_regs(0x0041d8ab, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0041d645, FUN_0041d645, "regs(esi:ptr[0x41c]:layout=car_0041e500+rec_00420e1d+car_00424878+car_00424cf5+rec_00413dd5+rec_0041d645) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0042458b+state_00420e1d+blk_00424553+state_0066d8c0+state_0041bce9+state_00444517+state_00424a21+recs_0063bd7c+view_00609860+tilt_0041d645")

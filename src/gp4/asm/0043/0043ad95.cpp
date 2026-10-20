#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043ad95
// Rebuilds the two 64-byte "active" flag tables 0x00607b18 (cars with bit 2 of +0xcc, and the used object
// slots) and 0x00607b58 (every car without bit 7 of +0x7a, and the used object slots) from the object slot
// states 0x0063bcfc[22..63] and the 22 car records at 0x0066fef0, with the highest active index in
// 0x006097ec. When there is one, 0x0043acb6 is run for every used slot 0..63 (with 0x006097fd = 0xff for a
// car slot that is active in the first table); 0x0043af84 always runs last. The rep stosd needs DF clear.
void __cdecl FUN_0043ad95(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    for (uint32_t i = 0; i < 16; ++i) {
        GP4_ARRAY(uint32_t, 0x00607b18)[i] = 0;
    }
    for (uint32_t i = 0; i < 16; ++i) {
        GP4_ARRAY(uint32_t, 0x00607b58)[i] = 0;
    }
    c.eax = 0;
    c.edi = 0x00607b58u + 0x40u;

    GP4_GLOBAL(int32_t, 0x006097ec) = 0;
    uint32_t slot = 0x16;
    do {
        if (GP4_ARRAY(uint8_t, 0x0063bcfc)[slot] != 0) {
            GP4_GLOBAL(uint32_t, 0x006097ec) = slot;
            GP4_ARRAY(uint8_t, 0x00607b18)[slot] = 0xff;
            GP4_ARRAY(uint8_t, 0x00607b58)[slot] = 0xff;
        }
        ++slot;
    } while (slot != 0x40);

    uint8_t* car = reinterpret_cast<uint8_t*>(0x0066fef0);
    uint32_t index = 0;
    do {
        if ((GP4_FIELD(uint8_t, car, 0x7a) & 0x80u) == 0u) {
            GP4_ARRAY(uint8_t, 0x00607b58)[index] = 0xff;
            if ((GP4_FIELD(uint8_t, car, 0xcc) & 4u) != 0u) {
                GP4_ARRAY(uint8_t, 0x00607b18)[index] = 0xff;
                if (GP4_GLOBAL(int32_t, 0x006097ec) < static_cast<int32_t>(index)) {
                    GP4_GLOBAL(int32_t, 0x006097ec) = static_cast<int32_t>(index);
                }
            }
        }
        ++index;
        car += 0x41c;
    } while (index != 0x16);
    c.esi = reinterpret_cast<uintptr_t>(car);
    c.ecx = index;

    if (GP4_GLOBAL(int32_t, 0x006097ec) != 0) {
        uint32_t ebx = 0;
        uint32_t counter = 0x40;
        do {
            if (GP4_ARRAY(uint8_t, 0x0063bcfc)[ebx] != 0) {
                if (static_cast<int32_t>(ebx) < 0x16 && GP4_ARRAY(uint8_t, 0x00607b18)[ebx] == 0xff) {
                    GP4_GLOBAL(uint8_t, 0x006097fd) = 0xff;
                }
                c.ebx = ebx;
                c.ecx = counter;
                gp4::call_regs(0x0043acb6, c);
                GP4_GLOBAL(uint8_t, 0x006097fd) = 0;
            }
            ++ebx;
            --counter;
        } while (counter != 0);
        c.ebx = ebx;
        c.ecx = 0;
    }

    gp4::call_regs(0x0043af84, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0043ad95, FUN_0043ad95, "regs() -> (eax,ebx,ecx,esi,edi,cf,pf,af,zf,sf,of); globals=fuzz:i32+layout=cars_0066fef0+carx_0042fe2f+state_00437888+flags_00408f7b+ring_008943a0+words_008943a0+bytes_0063bcfc+state_0066d8c0+state_0043acb6+slots_0043ad30+recs_0063bd7c+list_0043aed0+objs_0043ad95")

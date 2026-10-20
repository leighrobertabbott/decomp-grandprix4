#include <gp4/gp4.h>

// FUNCTION: GP4 0x00409467
// Computes the lap position of the car record in esi from the segment record in edi: the segment
// number (word +0x1a & 0x7ff) is, when the car's flag byte +0x8d has bit 1 set, converted by
// 0x00405985 (with the sign-extended byte +0x67 in cx and bit 2 of +0x21d set; bit 2 is cleared
// otherwise), reduced by the lap-length word 0x00868558, offset by the start word 0x0066d88a and
// wrapped into 0..lap length. The result goes to word +0x68; word +0xe2 gets (result << 6) + cx.
// 0x0040db6c (a pushal / popal routine) then fills the lane words +0x148..+0x14e; the flags are
// its last flag writer's. Only ax, cx and dx change (dx only through 0x00405985).
void __cdecl FUN_00409467(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* const segment = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));

    uint32_t eaxHigh = r->eax & 0xffff0000u;
    uint32_t ecxHigh = r->ecx & 0xffff0000u;
    uint32_t edx = r->edx;

    uint16_t position = static_cast<uint16_t>(*reinterpret_cast<const uint16_t*>(segment + 0x1a) & 0x7ffu);
    uint16_t sub = static_cast<uint16_t>(static_cast<int16_t>(static_cast<int8_t>(car[0x67])));
    car[0x21d] &= 0xfbu;
    if ((car[0x8d] & 2u) != 0u) {
        car[0x21d] |= 4u;
        gp4::Regs convert = *r;
        convert.st_in = 0;
        convert.st_out = 0;
        convert.eax = eaxHigh | position;
        convert.ecx = ecxHigh | sub;
        gp4::call_regs(0x00405985, convert);
        eaxHigh = convert.eax & 0xffff0000u;
        ecxHigh = convert.ecx & 0xffff0000u;
        edx = convert.edx;
        position = static_cast<uint16_t>(convert.eax);
        sub = static_cast<uint16_t>(convert.ecx);
        if (static_cast<int16_t>(position) >= GP4_GLOBAL(int16_t, 0x00868558)) {
            position = static_cast<uint16_t>(position - GP4_GLOBAL(uint16_t, 0x00868558));
        }
    }
    position = static_cast<uint16_t>(position - GP4_GLOBAL(uint16_t, 0x0066d88a));
    if (static_cast<int16_t>(position) < 0) {
        position = static_cast<uint16_t>(position + GP4_GLOBAL(uint16_t, 0x00868558));
    }
    if (static_cast<int16_t>(position) >= GP4_GLOBAL(int16_t, 0x00868558)) {
        position = static_cast<uint16_t>(position - GP4_GLOBAL(uint16_t, 0x00868558));
    }
    *reinterpret_cast<uint16_t*>(car + 0x68) = position;
    position = static_cast<uint16_t>(position << 6);
    position = static_cast<uint16_t>(position + sub);
    *reinterpret_cast<uint16_t*>(car + 0xe2) = position;

    gp4::Regs lanes = *r;
    lanes.st_in = 0;
    lanes.st_out = 0;
    lanes.eax = eaxHigh | position;
    lanes.ecx = ecxHigh | sub;
    lanes.edx = edx;
    gp4::call_regs(0x0040db6c, lanes);

    r->eax = eaxHigh | position;
    r->ecx = ecxHigh | sub;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | (lanes.eflags & 0x8d5u);
}
GP4_IMPL(0x00409467, FUN_00409467, "regs(eax:u32, ecx:u32, edx:u32, edi:ptr[0x40]:bytes, esi:ptr[0x420]:layout=car_0043c677) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=lanes_0040da62+pend_0040daeb+state_004094e3+pitw_00409467")

#include <gp4/gp4.h>

// The 0x10-byte entry `index` of the table at 0x0062fc28.
static __declspec(noinline) uint8_t* tableEntry(uint32_t index) {
    return GP4_ARRAY(uint8_t, 0x0062fc28) + index * 0x10u;
}

// Fills table entry 0x32 (0x0062ff48) from the first track-segment record (0x008943a0): kind 4, reference
// segment pointer +0xc, side flag +2 (0xff when the byte [0x00868582] is negative), the rotated offset
// (0x00404796 with eax = a lane half-width + 0x50 - 0xc0, negated for the negative side; edx = 0) in +4/+8 and
// the marker byte 7, then marks bit 0x32 of slot 0 through 0x00434063. pushal / popal restore every register;
// the flags are those 0x00434063 leaves.
// FUNCTION: GP4 0x00441c92
void __cdecl FUN_00441c92(gp4::Regs* r) {
    const uint32_t segment = 0x008943a0u;
    uint8_t* record = GP4_ARRAY(uint8_t, 0x008943a0);

    GP4_GLOBAL(uint32_t, 0x0062fc10) = 4u;
    GP4_GLOBAL(uint32_t, 0x0062fc14) = 0x32u;

    uint8_t* entry = tableEntry(0x32u);
    GP4_FIELD(uint32_t, entry, 0xc) = segment;
    entry[3] = 0;

    uint32_t offset;
    if ((GP4_GLOBAL(uint8_t, 0x00868582) & 0x80u) != 0u) {
        entry[2] = 0xff;
        offset = 0u - (GP4_FIELD(uint16_t, record, 0x60) + 0x50u - 0xc0u);
    } else {
        entry[2] = 0;
        offset = GP4_FIELD(uint16_t, record, 0x62) + 0x50u - 0xc0u;
    }

    gp4::Regs rotate = *r;
    rotate.st_in = 0;
    rotate.st_out = 0;
    rotate.eax = offset;
    rotate.edx = 0u;
    rotate.ecx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
    rotate.edi = segment;
    gp4::call_regs(0x00404796, rotate);

    GP4_FIELD(uint32_t, entry, 4) = rotate.eax;
    GP4_FIELD(uint32_t, entry, 8) = rotate.edx;
    entry[1] = 7;
    entry[0] = static_cast<uint8_t>(GP4_GLOBAL(uint32_t, 0x0062fc10));

    gp4::Regs mark = *r;
    mark.st_in = 0;
    mark.st_out = 0;
    mark.eax = 0u;
    mark.edx = GP4_GLOBAL(uint32_t, 0x0062fc14);
    mark.ecx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
    mark.edi = segment;
    mark.eflags = rotate.eflags;
    gp4::call_regs(0x00434063, mark);

    r->eflags = (r->eflags & ~0x8d5u) | (mark.eflags & 0x8d5u);
}
GP4_IMPL(0x00441c92, FUN_00441c92, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=geom_008943a0+words_008943a0")

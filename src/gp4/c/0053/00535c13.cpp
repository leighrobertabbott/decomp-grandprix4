#include <gp4/gp4.h>
#include <gp4/flags.h>

// Returns eax = 1 << shift, where shift is a byte of the table at 0x00829400 selected by the
// word [0x0088f633] (clamped to 8), plus 4 when either link byte [0x007aafd3] / [0x007aafd8]
// is 0xff. Leaves ecx = shift, ebx = 0x9e00 (link) or 0x841e0, and the flags of the last
// cmp / add / shl.
// FUNCTION: GP4 0x00535c13
void __cdecl FUN_00535c13(gp4::Regs* r) {
    uint32_t index = GP4_GLOBAL(uint16_t, 0x0088f633);
    if (index > 8u) {
        index = 8u;
    }
    uint32_t shift = GP4_ARRAY(uint8_t, 0x00829400)[index];

    const uint8_t linkA = GP4_GLOBAL(uint8_t, 0x007aafd3);
    uint32_t arith = gp4::flags::sub8(linkA, 0xffu);
    bool link = linkA == 0xffu;
    if (!link) {
        const uint8_t linkB = GP4_GLOBAL(uint8_t, 0x007aafd8);
        arith = gp4::flags::sub8(linkB, 0xffu);
        link = linkB == 0xffu;
    }

    uint32_t ebx;
    if (link) {
        arith = gp4::flags::add32(shift, 4u);
        shift += 4u;
        ebx = 0x9e00u;
    } else {
        ebx = 0x841e0u;
    }

    const uint32_t count = shift & 31u;
    if (count != 0u) {
        const uint32_t result = 1u << count;
        arith = gp4::flags::szp(result, 32) | (count == 31u ? gp4::flags::OF : 0u);
    }

    r->eax = 1u << count;
    r->ebx = ebx;
    r->ecx = shift;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00535c13, FUN_00535c13, "regs() -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=recv_00445423")

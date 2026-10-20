#include <gp4/gp4.h>
#include <gp4/flags.h>

// Compares the dwords +4 and +8 of the record in ebx: eax = |[ebx+4] - [ebx+8]|. When the distance
// is below 4 the (unaligned) dword at 0x00828293 is set to 0xba, otherwise it is incremented. The
// flags are those of `cmp eax, 4` (set case) or of the `add dword [0x828293], 1`.
// FUNCTION: GP4 0x00534fb2
void __cdecl FUN_00534fb2(gp4::Regs* r) {
    const uint32_t record = r->ebx;
    uint32_t distance = GP4_FIELD(uint32_t, record, 4) - GP4_FIELD(uint32_t, record, 8);
    if (static_cast<int32_t>(distance) < 0) {
        distance = 0u - distance;
    }
    uint32_t flags;
    if (distance < 4u) {
        flags = gp4::flags::sub32(distance, 4u);
        GP4_GLOBAL(uint32_t, 0x00828293) = 0xbau;
    } else {
        const uint32_t before = GP4_GLOBAL(uint32_t, 0x00828293);
        GP4_GLOBAL(uint32_t, 0x00828293) = before + 1u;
        flags = gp4::flags::add32(before, 1u);
    }
    r->eax = distance;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00534fb2, FUN_00534fb2, "regs(ebx:ptr[0x10]:i32[-8..8]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

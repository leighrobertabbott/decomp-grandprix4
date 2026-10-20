#include <gp4/gp4.h>
#include <gp4/flags.h>

// ORs a mask derived from al into five bytes of the record at ebx: al|3 into +0x30 and +0x3c,
// that & 0x3f into +0x38 and +0x34, that & 3 (always 3) into +0x40. Leaves al = 3; the flags are
// those of the last `or byte [ebx + 0x40], al`.
// FUNCTION: GP4 0x005593d9
void __cdecl FUN_005593d9(gp4::Regs* r) {
    uint8_t mask = static_cast<uint8_t>(static_cast<uint8_t>(r->eax) | 3u);

    GP4_FIELD(uint8_t, r->ebx, 0x30) |= mask;
    GP4_FIELD(uint8_t, r->ebx, 0x3c) |= mask;
    mask = static_cast<uint8_t>(mask & 0x3fu);
    GP4_FIELD(uint8_t, r->ebx, 0x38) |= mask;
    GP4_FIELD(uint8_t, r->ebx, 0x34) |= mask;
    mask = static_cast<uint8_t>(mask & 3u);
    const uint8_t last = static_cast<uint8_t>(GP4_FIELD(uint8_t, r->ebx, 0x40) | mask);
    GP4_FIELD(uint8_t, r->ebx, 0x40) = last;

    r->eax = (r->eax & 0xffffff00u) | mask;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(last));
}
GP4_IMPL(0x005593d9, FUN_005593d9, "regs(eax:u32, ebx:ptr[0x48]:bytes) -> (eax, cf, pf, af, zf, sf, of)")

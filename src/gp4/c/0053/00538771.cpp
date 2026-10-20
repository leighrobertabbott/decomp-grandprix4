#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sums the dwords at +0x14 and +0x10 of the record at 0x00829868 and stores the sum to 0x0082fcb0.
// eax/edx are saved and restored by the original; the flags are those of the `add`.
// FUNCTION: GP4 0x00538771
void __cdecl FUN_00538771(gp4::Regs* r) {
    uint8_t* record = GP4_ARRAY(uint8_t, 0x00829868);
    const uint32_t first = GP4_FIELD(uint32_t, record, 0x14);
    const uint32_t second = GP4_FIELD(uint32_t, record, 0x10);
    GP4_GLOBAL(uint32_t, 0x0082fcb0) = first + second;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(first, second));
}
GP4_IMPL(0x00538771, FUN_00538771, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

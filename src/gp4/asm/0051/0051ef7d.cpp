#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks up a 16-bit value two tables deep: w = word [0x009559f8 + 2 * (word [edi + 0x56] * 0x30 +
// eax)] is an index into the 10-byte records at 0x00961b9c, whose word +0 is returned in eax.
// ebx is saved and restored; the flags are those of `and ebx, 0xffff` on w (lea/movzx keep them).
// FUNCTION: GP4 0x0051ef7d
void __cdecl FUN_0051ef7d(gp4::Regs* r) {
    const uint32_t row = GP4_FIELD(uint16_t, r->edi, 0x56);
    const uint32_t index = row * 0x30u + r->eax;
    const uint32_t entry = GP4_ARRAY(const uint16_t, 0x009559f8)[index];
    const uint32_t record = 0x00961b9cu + entry * 10u;
    r->eax = GP4_FIELD(uint16_t, record, 0);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(entry));
}
GP4_IMPL(0x0051ef7d, FUN_0051ef7d, "regs(eax:u32[0..47], edi:ptr[0x5e]:u16[0..7]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=attr_00558e7b")

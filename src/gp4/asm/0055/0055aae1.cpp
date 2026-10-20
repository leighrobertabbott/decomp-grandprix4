#include <gp4/gp4.h>
#include <gp4/flags.h>

// Initialises one of the 0x420-spaced records after [0x866f08] (record bx, offset taken in 16 bits, +0x160): the
// word at +0x1e becomes bx + 0x3ff0, and bits 0xc0 of the byte at +0x48 and 0x80 of the byte at +0x34 are set.
// eax ends as the new word (upper half cleared by `movzx`), edx keeps its upper half and holds the high word of
// the 16-bit `mul dx` in dx, edi is the record address; the flags are those of the last `or byte [edi+0x34], 0x80`.
// FUNCTION: GP4 0x0055aae1
void __cdecl FUN_0055aae1(gp4::Regs* r) {
    const uint32_t bx = r->ebx & 0xffffu;
    const uint32_t product = bx * 0x420u;
    const uint32_t offset = ((product & 0xffffu) + 0x160u) & 0xffffu;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00866f08) + offset));
    const uint32_t word = (bx + 0x3ff0u) & 0xffffu;

    GP4_FIELD(uint16_t, record, 0x1e) = static_cast<uint16_t>(word);
    GP4_FIELD(uint8_t, record, 0x48) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x48) | 0xc0u);
    const uint32_t last = GP4_FIELD(uint8_t, record, 0x34) | 0x80u;
    GP4_FIELD(uint8_t, record, 0x34) = static_cast<uint8_t>(last);

    r->eax = word;
    r->edx = (r->edx & 0xffff0000u) | (product >> 16);
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(last));
}
GP4_IMPL(0x0055aae1, FUN_0055aae1, "regs(ebx:u32[0..0x80], edx:u32) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=layout=state_0040cb38")

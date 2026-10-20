#include <gp4/gp4.h>
#include <gp4/flags.h>

// Opens a gap in a buffer: edi is a record whose dword +4 is the buffer end pointer. The end pointer
// moves up by eax, and the bytes from edx to the old end (inclusive) are copied backwards (std; rep
// movsb) to end the new end pointer. Every register is restored by popal; the flags are those of the
// `inc ecx` after `sub ecx, edx` (ecx = old end - edx), so CF is the borrow of that subtraction.
// FUNCTION: GP4 0x0054e12a
void __cdecl FUN_0054e12a(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint32_t oldEnd = GP4_FIELD(uint32_t, record, 4);
    const uint32_t newEnd = oldEnd + r->eax;
    GP4_FIELD(uint32_t, record, 4) = newEnd;

    const uint32_t span = oldEnd - r->edx;
    const uint32_t subFlags = gp4::flags::sub32(oldEnd, r->edx);
    uint32_t count = span + 1u;

    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(oldEnd));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(newEnd));
    for (; count != 0u; --count) {
        *dst = *src;
        --src;
        --dst;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(span, subFlags));
}
GP4_IMPL(0x0054e12a, FUN_0054e12a, "regs(eax:i32[-8..8], edi:ptr[0x10]:layout=buf_0054e12a, edx:rec[0xa20000,1,0..64]) -> (cf, pf, af, zf, sf, of); globals=layout=text_0054e12a")

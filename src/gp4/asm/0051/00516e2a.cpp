#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00516e2a
// Record (esi) step/clamp, variant that honours the +0x82 bit 3 hold flag: when the dword at +0x1c
// has reached the limit in 0x891500 the result is 2; otherwise, unless bit 3 of the byte at +0x82
// is set, the byte counter at +0x74 is incremented and wraps to 0 once it reaches 3 (a set hold
// flag returns the counter unchanged). edx is saved and restored. Flags are those of the last
// cmp/test the taken path executed.
void __cdecl FUN_00516e2a(gp4::Regs* r) {
    const uint8_t* rec = (const uint8_t*)r->esi;
    const uint32_t counter = rec[0x74];
    const uint32_t value = *(const uint32_t*)(rec + 0x1c);
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);
    const uint32_t hold = rec[0x82];

    uint32_t result;
    uint32_t flags;
    if (value >= limit) {
        result = 2u;
        flags = gp4::flags::sub32(value, limit);
    } else if ((hold & 8u) != 0u) {
        result = counter;
        flags = gp4::flags::logic8(hold & 8u);
    } else {
        result = counter + 1u;
        flags = gp4::flags::sub32(result, 3u);
        if ((int32_t)result >= 3) {
            result = 0u;
        }
    }
    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00516e2a, FUN_00516e2a, "regs(esi:ptr[0x8a]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

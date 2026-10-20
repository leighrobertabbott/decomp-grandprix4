#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies [0x0066d8ec] bytes from the table 0x007c31f2 to the car-code list 0x0066d8f4 and counts (ebp) the entries
// that are negative (bit 7 set) or for which the carry predicate 0x00520023 returns CF = 0. A non-empty scan then
// sets the count [0x0066d8ec] to 0x16 and stores ebp clamped to 2..0x16 in [0x0066fd6c]. edx is saved and restored;
// ebx walks to 0x0066d8f4 + count, ecx ends at 0, al holds the last copied byte. The flags are those of
// `or ecx,ecx` (empty list) or of the final `cmp ebp, 0x16`.
// FUNCTION: GP4 0x00522b58
void __cdecl FUN_00522b58(gp4::Regs* r) {
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x0066d8ec);
    uint32_t eax = r->eax;
    uint32_t ebx = 0x0066d8f4u;
    uint32_t ebp = 0u;
    uint32_t arith = gp4::flags::logic32(count);

    if (count != 0u) {
        const uint8_t* src = GP4_ARRAY(uint8_t, 0x007c31f2);
        uint8_t* dst = GP4_ARRAY(uint8_t, 0x0066d8f4);
        for (uint32_t i = 0u; i != count; ++i) {
            const uint8_t code = src[i];
            dst[i] = code;
            eax = (eax & 0xffffff00u) | code;
            ebx += 1u;
            if ((code & 0x80u) != 0u) {
                ebp += 1u;
            } else {
                gp4::Regs call = *r;
                call.st_in = 0;
                call.st_out = 0;
                gp4::call_regs(0x00520023, call);
                if ((call.eflags & gp4::flags::CF) == 0u) {
                    ebp += 1u;
                }
            }
        }
        GP4_GLOBAL(uint32_t, 0x0066d8ec) = 0x16u;
        if (static_cast<int32_t>(ebp) < 2) {
            ebp = 2u;
        }
        arith = gp4::flags::sub32(ebp, 0x16u);
        if (static_cast<int32_t>(ebp) > 0x16) {
            ebp = 0x16u;
        }
        GP4_GLOBAL(uint32_t, 0x0066fd6c) = ebp;
    }

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = 0u;
    r->ebp = ebp;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00522b58, FUN_00522b58, "regs(eax:u32) -> (eax, ebx, ecx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00443278+state_0040dfe9")

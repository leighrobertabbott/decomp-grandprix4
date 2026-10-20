#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x004058cf
// Three-pass drivetrain solver for the engine record in esi (the sibling of 0x004057cf, which takes
// its pass count from [0x00603078] and keeps every register). [0x00603088] starts as 0x147ae1; each
// pass clamps |[0x00638ce0]| to at least 0x80000 (A), runs 0x00404f99 (eax = A, edx = [0x00603088]) and
// computes, all on the x87 stack,
//   q = fistp( ([0x00602d70] * c(6f2bb4) * [0x00601334] * c(6f2b9c))
//              / ((A * c(6f2b9c))^2 * word(+0xb2)) * c(6f29d4) ).
// Between passes [0x00603088] = ((q << 14) * [0x006f2bec]) / [0x00601334] (imul / idiv). [0x006f2bf4]
// is the scratch dword the original stores A, the zero-extended word +0xb2 and q through.
// Returns q in eax, ecx = 0 (the pass counter), edx = the zero-extended word +0xb2; the flags are the
// "dec ecx" result (ZF, PF set) with the carry the last 0x00404f99 left.
void __cdecl FUN_004058cf(gp4::Regs* r) {
    using namespace gp4::x87;
    uint8_t* engine = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(int32_t, 0x00603088) = 0x147ae1;
    uint32_t count = 3u;
    uint32_t carry;
    int32_t result;
    uint32_t word;

    for (;;) {
        int32_t scaled = GP4_GLOBAL(int32_t, 0x00638ce0);
        if (scaled < 0) {
            scaled = static_cast<int32_t>(0u - static_cast<uint32_t>(scaled));
        }
        if (scaled < 0x80000) {
            scaled = 0x80000;
        }

        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.st_xin_mask = 0;
        call.st_xout_mask = 0;
        call.eax = static_cast<uint32_t>(scaled);
        call.edx = GP4_GLOBAL(uint32_t, 0x00603088);
        call.ecx = count;
        gp4::call_regs(0x00404f99, call);
        carry = call.eflags & 1u;

        const Ext numerator =
            ((ext_i32(GP4_GLOBAL(int32_t, 0x00602d70)) * ext(GP4_GLOBAL(double, 0x006f2bb4)))
             * ext_i32(GP4_GLOBAL(int32_t, 0x00601334))) * ext(GP4_GLOBAL(double, 0x006f2b9c));
        GP4_GLOBAL(int32_t, 0x006f2bf4) = scaled;
        const Ext scaledK = ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)) * ext(GP4_GLOBAL(double, 0x006f2b9c));
        const Ext squared = scaledK * scaledK;
        word = GP4_FIELD(uint16_t, engine, 0xb2);
        GP4_GLOBAL(uint32_t, 0x006f2bf4) = word;
        const Ext denominator = squared * ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
        result = fistp32((numerator / denominator) * ext(GP4_GLOBAL(double, 0x006f29d4)));
        GP4_GLOBAL(int32_t, 0x006f2bf4) = result;

        --count;
        if (count == 0u) {
            break;
        }

        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(static_cast<uint32_t>(result) << 14))
                              * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006f2bec));
        int32_t remainder;
        GP4_GLOBAL(int32_t, 0x00603088) = gp4::idiv64(product, GP4_GLOBAL(int32_t, 0x00601334), &remainder);
    }

    r->eax = static_cast<uint32_t>(result);
    r->ecx = 0u;
    r->edx = word;
    // "dec ecx" to zero: ZF, PF set, SF/OF/AF clear; CF is left as the last callee returned it.
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u | carry;
}
GP4_IMPL(0x004058cf, FUN_004058cf, "regs(esi:ptr[0x118]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_00404ed0+state_00404f33+engine_00413606")

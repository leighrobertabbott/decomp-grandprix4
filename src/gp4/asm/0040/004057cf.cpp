#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x004057cf
// Iterates the drivetrain solver 0x00404f99 [0x00603078] times for the engine record in esi (pushal /
// popal: every register is preserved, CF is the callee's last carry and ZF, PF are set by the final
// "dec ecx" reaching zero). Each pass clamps |dword +0x98| >> 8 to at least 0x80000 into [0x00603084],
// runs 0x404f99 (eax = that value, edx = [0x00603088]) and stores
//   +0x284 = fistp(| [0x00602d70] * c(6f2bb4) / (A * c(6f2b9c) * word(+0xb2)) * c(6f29e4) |)
// with A = [0x00603084] (all on the x87 stack); between passes
//   [0x00603088] = fistp((+0x284 * c(6f2bcc)) / (A * c(6f2b9c)) * c(6f2b5c)).
// [0x00603088] starts as 0x147ae1; [0x006f2bf4] receives the zero-extended word +0xb2.
void __cdecl FUN_004057cf(gp4::Regs* r) {
    using namespace gp4::x87;
    uint8_t* engine = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(int32_t, 0x00603088) = 0x147ae1;
    uint32_t count = GP4_GLOBAL(uint32_t, 0x00603078);
    uint32_t carry;

    for (;;) {
        uint32_t speed = GP4_FIELD(uint32_t, engine, 0x98);
        if (static_cast<int32_t>(speed) < 0) {
            speed = 0u - speed;
        }
        int32_t scaled = static_cast<int32_t>(speed) >> 8;
        if (scaled < 0x80000) {
            scaled = 0x80000;
        }
        GP4_GLOBAL(int32_t, 0x00603084) = scaled;

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

        const Ext product = ext_i32(GP4_GLOBAL(int32_t, 0x00603084)) * ext(GP4_GLOBAL(double, 0x006f2b9c));
        GP4_GLOBAL(uint32_t, 0x006f2bf4) = GP4_FIELD(uint16_t, engine, 0xb2);
        const Ext denominator = product * ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
        const Ext numerator = ext_i32(GP4_GLOBAL(int32_t, 0x00602d70)) * ext(GP4_GLOBAL(double, 0x006f2bb4));
        const Ext result = fabs((numerator / denominator) * ext(GP4_GLOBAL(double, 0x006f29e4)));
        GP4_FIELD(int32_t, engine, 0x284) = fistp32(result);

        --count;
        if (count == 0u) {
            break;
        }

        const Ext force = ext_i32(GP4_FIELD(int32_t, engine, 0x284)) * ext(GP4_GLOBAL(double, 0x006f2bcc));
        const Ext divisor = ext_i32(GP4_GLOBAL(int32_t, 0x00603084)) * ext(GP4_GLOBAL(double, 0x006f2b9c));
        const Ext next = (force / divisor) * ext(GP4_GLOBAL(double, 0x006f2b5c));
        GP4_GLOBAL(int32_t, 0x00603088) = fistp32(next);
    }

    // "dec ecx" to zero: ZF, PF set; CF is left as the callee returned it.
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u | carry;
}
GP4_IMPL(0x004057cf, FUN_004057cf, "regs(esi:ptr[0x28c]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=state_00404ed0+state_00404f33+engine_00413606+tune_004057cf")

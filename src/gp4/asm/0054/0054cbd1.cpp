#include <gp4/gp4.h>
#include <gp4/flags.h>

// Range-checks eax against the signed limit at 0x822248: when eax >= limit the empty routine
// 0x54e232 is called (inside pushfd/pushal with esi = 0x54cbdd; everything is restored), otherwise
// eax is stored at the (unaligned) dword 0x833cc9. The flags are those of `cmp eax, [0x822248]`
// (popfd restores them after the call).
// FUNCTION: GP4 0x0054cbd1
void __cdecl FUN_0054cbd1(gp4::Regs* r) {
    const int32_t value = static_cast<int32_t>(r->eax);
    const int32_t limit = GP4_GLOBAL(int32_t, 0x00822248);
    const uint32_t arith = gp4::flags::sub32(r->eax, static_cast<uint32_t>(limit));
    if (value >= limit) {
        gp4::Regs hook = *r;
        hook.esi = 0x0054cbddu;
        gp4::call_regs(0x0054e232, hook);
    } else {
        GP4_GLOBAL(int32_t, 0x00833cc9) = value;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054cbd1, FUN_0054cbd1, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal routine that sizes the pool of 0x5b8-byte records. GlobalMemoryStatus (thunk 0x005a4900) fills
// the MEMORYSTATUS block at 0x829aa3; while the word [0x7ad88c] is still 0xffff it is set from the total
// physical memory ([0x829aa3 + 8], signed compare): <= 0x2000000 and <= 0x3000000 -> 0x1e, <= 0x4000000 ->
// 0x2a, <= 0x8000000 -> 0x33, else 0x3c. Then count = [0x7ad88c] / 3 goes to [0x7a8e1c], count + 4 to
// [0x7a8e20] and [0x8299a8] = count * 0x5b8 + [0x7a8e24] * (count + 4) + 0x10. popal restores every
// register; the flags are those of the closing `add eax, 8`.
// FUNCTION: GP4 0x005375f0
void __cdecl FUN_005375f0(gp4::Regs* r) {
    gp4::stdcall_<0x005a4900, void>(0x00829aa3u);
    const int32_t total = GP4_FIELD(int32_t, 0x00829aa3u, 8);
    if (GP4_GLOBAL(uint16_t, 0x007ad88c) == 0xffffu) {
        uint16_t blocks;
        if (total <= 0x2000000) {
            blocks = 0x1eu;
        } else if (total <= 0x3000000) {
            blocks = 0x1eu;
        } else if (total <= 0x4000000) {
            blocks = 0x2au;
        } else if (total <= 0x8000000) {
            blocks = 0x33u;
        } else {
            blocks = 0x3cu;
        }
        GP4_GLOBAL(uint16_t, 0x007ad88c) = blocks;
    }

    const uint32_t count = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x007ad88c)) / 3u;
    GP4_GLOBAL(uint32_t, 0x007a8e1c) = count;
    GP4_GLOBAL(uint32_t, 0x007a8e20) = count + 4u;
    const uint32_t rows = GP4_GLOBAL(uint32_t, 0x007a8e24) * GP4_GLOBAL(uint32_t, 0x007a8e20);
    const uint32_t bytes = GP4_GLOBAL(uint32_t, 0x007a8e1c) * 0x5b8u;
    const uint32_t partial = bytes + rows + 8u;
    GP4_GLOBAL(uint32_t, 0x008299a8) = partial + 8u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(partial, 8u));
}
GP4_IMPL(0x005375f0, FUN_005375f0, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

#include <gp4/gp4.h>

// FUNCTION: GP4 0x00420880
// Sets byte DAT_00629bd0 to 0xff when max(|DAT_00638034|, |DAT_00638038|) is at
// least DAT_00629bd8 (signed compare), else to 0. eax leaves holding the maximum
// absolute value, edx |DAT_00638038| and the flags are those of the final compare.
void __cdecl FUN_00420880(gp4::Regs* r) {
    int32_t a = GP4_GLOBAL(int32_t, 0x00638034);
    if (a < 0) {
        a = (int32_t)(0u - (uint32_t)a);
    }
    int32_t b = GP4_GLOBAL(int32_t, 0x00638038);
    if (b < 0) {
        b = (int32_t)(0u - (uint32_t)b);
    }
    if (a < b) {
        a = b;
    }
    const int32_t limit = GP4_GLOBAL(int32_t, 0x00629bd8);
    GP4_GLOBAL(uint8_t, 0x00629bd0) = (a < limit) ? 0x00 : 0xff;

    // flags of: cmp eax, limit
    const uint32_t l = (uint32_t)a;
    const uint32_t rr = (uint32_t)limit;
    const uint32_t res = l - rr;
    uint32_t fl = r->eflags & ~0x8D5u;
    if (l < rr) fl |= 0x001u;
    uint32_t p = res & 0xffu;
    p ^= p >> 4; p ^= p >> 2; p ^= p >> 1;
    if (!(p & 1u)) fl |= 0x004u;
    if ((l ^ rr ^ res) & 0x10u) fl |= 0x010u;
    if (res == 0) fl |= 0x040u;
    if (res & 0x80000000u) fl |= 0x080u;
    if (((l ^ rr) & (l ^ res)) & 0x80000000u) fl |= 0x800u;

    r->eax = l;
    r->edx = (uint32_t)b;
    r->eflags = fl;
}
GP4_IMPL(0x00420880, FUN_00420880, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

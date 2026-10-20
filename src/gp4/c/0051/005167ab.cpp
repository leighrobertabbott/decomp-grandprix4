#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores the entry (([0x00675958] & 0x3f) - 1, clamped to at most 0x15) of the dword table 0x00675970
// in the dword 0x00a6987c. pushal/popal keep every register; the flags are those of
// `cmp ecx, 0x15` on the unclamped index (the clamping `mov` touches none).
// FUNCTION: GP4 0x005167ab
void __cdecl FUN_005167ab(gp4::Regs* r) {
    int32_t index = static_cast<int32_t>(GP4_GLOBAL(uint8_t, 0x00675958) & 0x3fu) - 1;
    const uint32_t arith = gp4::flags::sub32(static_cast<uint32_t>(index), 0x15u);
    if (index > 0x15) {
        index = 0x15;
    }
    GP4_GLOBAL(uint32_t, 0x00a6987c) = GP4_ARRAY(const uint32_t, 0x00675970)[index];
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005167ab, FUN_005167ab, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

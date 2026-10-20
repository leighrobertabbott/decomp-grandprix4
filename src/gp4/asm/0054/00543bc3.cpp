#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
struct ViewBlock {
    int32_t Position;
    int32_t Limit;
    int32_t Size;
};
#pragma pack(pop)

// Clamps a scroll window to its range: a negative position becomes 0, and a window that would end
// past the limit (position + size > limit, signed) is moved back so it ends at the limit
// (position = limit - size). ebx points at the block; eax is saved and restored. The flags are
// those of the last compare: `cmp (position + size), limit` when nothing moves back, else the
// `sub limit, size` of the move.
// FUNCTION: GP4 0x00543bc3
void __cdecl FUN_00543bc3(gp4::Regs* r) {
    ViewBlock* block = reinterpret_cast<ViewBlock*>(static_cast<uintptr_t>(r->ebx));
    if (block->Position < 0) {
        block->Position = 0;
    }
    const uint32_t end = static_cast<uint32_t>(block->Position) + static_cast<uint32_t>(block->Size);
    uint32_t arith = gp4::flags::sub32(end, static_cast<uint32_t>(block->Limit));
    if (static_cast<int32_t>(end) > block->Limit) {
        arith = gp4::flags::sub32(static_cast<uint32_t>(block->Limit), static_cast<uint32_t>(block->Size));
        block->Position = block->Limit - block->Size;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00543bc3, FUN_00543bc3, "regs(ebx:ptr[0x10]:bytes) -> (cf, pf, af, zf, sf, of)")

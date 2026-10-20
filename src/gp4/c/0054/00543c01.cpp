#include <gp4/gp4.h>

#pragma pack(push, 1)
struct ViewBlock {
    uint32_t Position;
    uint32_t Limit;
    uint32_t Previous;
};
#pragma pack(pop)

// Resets the view block at 0x008310cc: position = 0, previous = limit, and the zoom word [0x0083137f]
// = 0x4000 (1.0 in Q14). eax is saved and restored; ebx is left pointing at the block (callers share it).
// FUNCTION: GP4 0x00543c01
void __cdecl FUN_00543c01(gp4::Regs* r) {
    ViewBlock* block = reinterpret_cast<ViewBlock*>(static_cast<uintptr_t>(0x008310cc));
    r->ebx = 0x008310ccu;
    block->Position = 0u;
    block->Previous = block->Limit;
    GP4_GLOBAL(uint32_t, 0x0083137f) = 0x4000u;
}
GP4_IMPL(0x00543c01, FUN_00543c01, "regs() -> (ebx); globals=layout=scroll_00543340")

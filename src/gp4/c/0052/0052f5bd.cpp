#include <gp4/gp4.h>

// pushal/popal routine: writes 0x62a at +0x12 of the node whose pointer lives in the cell 0x00845192,
// then follows that node's first dword to a second node and writes 0xe at its +0x12. All registers
// and flags come back unchanged.
// FUNCTION: GP4 0x0052f5bd
void __cdecl FUN_0052f5bd(gp4::Regs* r) {
    (void)r;
    const uint32_t first = GP4_GLOBAL(uint32_t, 0x00845192);
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(first + 0x12u)) = 0x62au;
    const uint32_t second = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(first));
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(second + 0x12u)) = 0xeu;
}
GP4_IMPL(0x0052f5bd, FUN_0052f5bd, "regs() -> (); globals=layout=cell_00845192")

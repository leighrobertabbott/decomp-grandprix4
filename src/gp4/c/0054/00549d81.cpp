#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds the free list of 255 nodes of 0x14 bytes at 0x00831d89: node k's first dword points at node
// k+1, the last node's is 0 (at 0x00833175). 0x00831d7d / 0x00831d81 are cleared and 0x00831d85
// (list head) is set to the first node. Leaves eax = edi = the terminator node, ecx = 0; the flags
// are those of the final `dec ecx` (result 0) with CF from the last `add edi, 0x14`.

// FUNCTION: GP4 0x00549d81
void __cdecl FUN_00549d81(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00831d7d) = 0u;
    GP4_GLOBAL(uint32_t, 0x00831d81) = 0u;
    uint32_t node = 0x00831d89u;
    GP4_GLOBAL(uint32_t, 0x00831d85) = node;
    uint32_t count = 0xffu;
    uint32_t next;
    uint32_t arith;
    do {
        next = node + 0x14u;
        *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(node)) = next;
        arith = gp4::flags::add32(node, 0x14u);
        node = next;
        --count;
    } while (count != 0u);
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(node)) = 0u;

    r->eax = next;
    r->ecx = 0u;
    r->edi = node;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, arith));
}
GP4_IMPL(0x00549d81, FUN_00549d81, "regs() -> (eax, ecx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

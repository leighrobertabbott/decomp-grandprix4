#include <gp4/gp4.h>
#include <gp4/flags.h>

// Allocates 0x1ae8 bytes from the block pool (0x005389b9). On success the block pointer goes to [0x00831568], the
// block is filled with the 0x54 x 0x50 space/CR/LF text screen (0x00544a15) and CF is cleared (the other flags
// stay those of the allocator's `cmp`); on failure eax = 0 and CF = 1 come back unchanged.
// FUNCTION: GP4 0x005449fd
void __cdecl FUN_005449fd(gp4::Regs* r) {
    r->eax = 0x1ae8u;
    gp4::call_regs(0x005389b9, *r);
    if ((r->eflags & static_cast<uint32_t>(gp4::flags::CF)) != 0u) {
        return;
    }
    GP4_GLOBAL(uint32_t, 0x00831568) = r->eax;
    gp4::call_regs(0x00544a15, *r);
    r->eflags &= ~static_cast<uint32_t>(gp4::flags::CF);
}
GP4_IMPL(0x005449fd, FUN_005449fd, "regs() -> (eax, ecx, edi, cf, pf, af, zf, sf, of); globals=layout=pool_005389b9+pool_005449fd")

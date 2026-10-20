#include <gp4/gp4.h>
#include <gp4/flags.h>

// Increments the counter at [edi+8] up to the limit at [edi+4]: eax = [edi+8] + 1; if (eax <= [edi+4])
// (signed) [edi+8] = eax. Leaves eax and the flags of `cmp eax, [edi+4]`.
// FUNCTION: GP4 0x0054de3e
void __cdecl FUN_0054de3e(gp4::Regs* r) {
    int32_t* rec = reinterpret_cast<int32_t*>(static_cast<uintptr_t>(r->edi));
    const int32_t next = rec[2] + 1;
    const int32_t limit = rec[1];
    r->eax = static_cast<uint32_t>(next);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(static_cast<uint32_t>(next), static_cast<uint32_t>(limit)));
    if (next <= limit) {
        rec[2] = next;
    }
}
GP4_IMPL(0x0054de3e, FUN_0054de3e, "regs(edi:ptr[0x40]:bytes) -> (eax, cf, pf, af, zf, sf, of)")

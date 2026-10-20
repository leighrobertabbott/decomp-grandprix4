#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks up the low six bits of al in the 22-byte table at 0x00831d67 (`repne scasb`). A value of
// 0 (after masking) gives al = 0xff without a search; a hit returns its index 0..21 in eax; a miss
// sets al = 0xff (upper bits of eax are kept). ecx and edi are pushed and popped. The flags are
// those of the last flag writer on the taken exit: `and al, 0x3f` (zero), `sub eax, ecx` (hit) or
// the final scasb compare (miss).
// FUNCTION: GP4 0x00549976
void __cdecl FUN_00549976(gp4::Regs* r) {
    const uint32_t key = r->eax & 0x3fu;
    if (key == 0u) {
        r->eax = (r->eax & 0xffffff00u) | 0xffu;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(0u));
        return;
    }
    const uint8_t* table = GP4_ARRAY(const uint8_t, 0x00831d67);
    uint32_t left = 0x16u;
    uint32_t flags = 0u;
    bool found = false;
    uint32_t last = 0u;
    while (left != 0u) {
        last = *table++;
        --left;
        flags = gp4::flags::sub8(key, last);
        if (key == last) {
            found = true;
            break;
        }
    }
    if (found) {
        r->eax = 0x15u - left;
        flags = gp4::flags::sub32(0x15u, left);
    } else {
        r->eax = (r->eax & 0xffffff00u) | 0xffu;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00549976, FUN_00549976, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_00831d67")

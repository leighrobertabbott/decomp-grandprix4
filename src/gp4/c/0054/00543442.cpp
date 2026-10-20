#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00543442
// Linear search of the six dwords at 0x0083135b for the dword at 0x00831377: the index of the first equal
// entry (6 when none) goes to the dword 0x00831357. eax = the last entry read, ecx = the index, esi = its
// address (0x0083135b + 4 * index), edi = 0x00831377; the flags are those of the equal compare, or of
// `cmp ecx, 6` when the scan ran out.
void __cdecl FUN_00543442(gp4::Regs* r) {
    const uint32_t* entry = GP4_ARRAY(const uint32_t, 0x0083135b);
    uint32_t index = 0u;
    uint32_t eax;
    uint32_t arith;
    for (;;) {
        eax = *entry;
        const uint32_t key = GP4_GLOBAL(uint32_t, 0x00831377);
        arith = gp4::flags::sub32(eax, key);
        if (eax == key) {
            break;
        }
        ++entry;
        ++index;
        arith = gp4::flags::sub32(index, 6u);
        if (index >= 6u) {
            break;
        }
    }
    GP4_GLOBAL(uint32_t, 0x00831357) = index;
    r->eax = eax;
    r->ecx = index;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
    r->edi = 0x00831377u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00543442, FUN_00543442, "regs() -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_0083135b")

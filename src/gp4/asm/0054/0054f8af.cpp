#include <gp4/gp4.h>
#include <gp4/flags.h>

// Compares the 0xf68-byte block at 0x007ab760 with the buffer pointed to by the dword at 0x0083e498
// (`repe cmpsb`, direction flag clear). Stops at the first differing byte or when ecx reaches zero;
// esi/edi are one past the last pair compared, ecx = the bytes left, and the flags are those of the
// last `cmp [esi], [edi]`.
// FUNCTION: GP4 0x0054f8af
void __cdecl FUN_0054f8af(gp4::Regs* r) {
    const uint32_t other = GP4_GLOBAL(uint32_t, 0x0083e498);
    const uint8_t* a = GP4_ARRAY(uint8_t, 0x007ab760);
    const uint8_t* b = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(other));
    uint32_t left = 0xf68u;
    uint32_t index = 0;
    uint32_t flags = 0;
    while (left != 0) {
        flags = gp4::flags::sub8(a[index], b[index]);
        ++index;
        --left;
        if ((flags & gp4::flags::ZF) == 0u) {
            break;
        }
    }
    r->esi = 0x007ab760u + index;
    r->edi = other + index;
    r->ecx = left;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054f8af, FUN_0054f8af, "regs() -> (esi, edi, ecx, cf, pf, af, zf, sf, of); globals=layout=cmp_0054f8af")

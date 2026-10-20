#include <gp4/gp4.h>
#include <gp4/flags.h>

// Picks one of three dwords of the esi record by the signed selector edx: edx < 1 -> +0x20,
// edx == 1 -> +0x24, edx > 1 -> +0x28. eax = the dword; the flags are those of `cmp edx, 1`.
// FUNCTION: GP4 0x00516ddf
void __cdecl FUN_00516ddf(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t kind = static_cast<int32_t>(r->edx);

    uint32_t value;
    if (kind == 1) {
        value = GP4_FIELD(uint32_t, record, 0x24);
    } else if (kind > 1) {
        value = GP4_FIELD(uint32_t, record, 0x28);
    } else {
        value = GP4_FIELD(uint32_t, record, 0x20);
    }

    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(static_cast<uint32_t>(kind), 1u));
}
GP4_IMPL(0x00516ddf, FUN_00516ddf, "regs(edx:i32[-1..3], esi:ptr[0x30]:bytes) -> (eax, cf, pf, af, zf, sf, of)")

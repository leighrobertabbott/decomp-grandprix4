#include <gp4/gp4.h>
#include <gp4/flags.h>

// When edx is 2 or 3: dword [ebx] = max([ebx] - dword [esi+0x26], 0) (signed compare), eax = the
// stored value, flags of the `sub eax, [esi+0x26]`. Otherwise only the flags of `cmp edx, 3` change.
// FUNCTION: GP4 0x00539c6b
void __cdecl FUN_00539c6b(gp4::Regs* r) {
    uint32_t flags = gp4::flags::sub32(r->edx, 3u);
    if (r->edx == 2u || r->edx == 3u) {
        uint32_t* cell = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(r->ebx));
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
        const uint32_t value = *cell;
        const uint32_t amount = GP4_FIELD(uint32_t, record, 0x26);
        flags = gp4::flags::sub32(value, amount);
        const uint32_t result = (static_cast<int32_t>(value) >= static_cast<int32_t>(amount)) ? value - amount : 0u;
        *cell = result;
        r->eax = result;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00539c6b, FUN_00539c6b, "regs(edx:u32{2,3,0,1,4}, ebx:ptr[8]:bytes, esi:ptr[0x30]:bytes) -> (eax, cf, pf, af, zf, sf, of)")

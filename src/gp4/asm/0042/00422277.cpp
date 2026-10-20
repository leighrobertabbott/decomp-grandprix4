#include <gp4/gp4.h>

// FUNCTION: GP4 0x00422277
// For i in 0..3: DAT_00602bf8[i] = dword [esi+0x32c+4i] + dword [esi+0x33c+4i]. eax = &DAT_00602bf8;
// ecx is preserved (push/pop) and the flags are those of the last "cmp ecx, 4" (equal).
void __cdecl FUN_00422277(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t* sums = GP4_ARRAY(uint32_t, 0x00602bf8);

    for (uint32_t i = 0; i != 4u; ++i) {
        sums[i] = GP4_FIELD(uint32_t, record, 0x32c + i * 4u) + GP4_FIELD(uint32_t, record, 0x33c + i * 4u);
    }

    r->eax = 0x00602bf8u;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00422277, FUN_00422277, "regs(esi:ptr[0x344]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

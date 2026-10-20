#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004188a5
void __cdecl FUN_004188a5(gp4::Regs* r) {
    // The entry selected through esi by 0x418901 gets bit 2 of its first dword cleared,
    // then set again when the byte at 0x625aba is non-zero. edi keeps the entry address.
    // Flags: "cmp byte [0x625aba], 0" (zero: ZF|PF) or those of the "or dword [edi], 4" result.
    gp4::Regs callee = *r;
    gp4::call_regs(0x00418901, callee);

    uint32_t* const entry = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(callee.eax));
    *entry &= 0xfffffffbu;
    uint32_t flags;
    if (GP4_GLOBAL(uint8_t, 0x00625aba) == 0u) {
        flags = 0x44u;
    } else {
        *entry |= 4u;
        const uint32_t value = *entry;
        flags = parityFlag(value)
            | (value == 0u ? 0x40u : 0u)
            | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
    }

    r->eax = callee.eax;
    r->ecx = callee.ecx;
    r->edx = callee.edx;
    r->ebx = callee.ebx;
    r->esi = callee.esi;
    r->ebp = callee.ebp;
    r->edi = callee.eax;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004188a5, FUN_004188a5, "regs(esi:ptr[0x20a]:layout=rec_00418cdd) -> (eax, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00418c68")

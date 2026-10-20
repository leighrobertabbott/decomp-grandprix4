#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the language-name table (count at 0x00838b91, string pointers from 0x00838b99) into the
// 11-slot pointer table at 0x00838902 and fills the unused slots with the address 0x00835807 (rep
// movsd / rep stosd). The registers are left as the string instructions leave them (esi past the
// source entries, edi past the destination table, ecx = 0, eax = 0x00835807 when slots were left
// over); the flags are those of the `0xb - count` subtraction (the stosd is skipped on zero).
// FUNCTION: GP4 0x0054e426
void __cdecl FUN_0054e426(gp4::Regs* r) {
    const uint32_t* src = GP4_ARRAY(const uint32_t, 0x00838b99);
    uint32_t* dst = GP4_ARRAY(uint32_t, 0x00835846) + 0xc2f;

    for (uint32_t n = GP4_GLOBAL(uint32_t, 0x00838b91); n != 0; n--) {
        *dst++ = *src++;
    }

    const uint32_t left = 0xbu;
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x00838b91);
    const uint32_t arith = gp4::flags::sub32(left, count);
    uint32_t rest = left - count;
    if (rest != 0) {
        r->eax = 0x00835807u;
        for (; rest != 0; rest--) {
            *dst++ = 0x00835807u;
        }
    }

    r->ecx = 0;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054e426, FUN_0054e426, "regs(eax:u32) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=strs_00838b91")

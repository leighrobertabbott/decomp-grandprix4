#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal / popal around a walk of the 12-byte records chained from the pointer at [esi + 0x18]
// (a null pointer = no records). Each record starts with a word code and the walk stops at the
// first code >= 0x15. For every record below that, the 64-bit pair edx:eax is shifted right by
// one bit (shr edx, 1; rcr eax, 1) and the bit that falls out of eax is stored as a mask byte at
// record +2 (0xff when set, else 0: sbb bl, bl). popal restores every register and not the
// flags, so the flags that escape are those of the last compare/test: `or edi, edi` on a null
// list, else the final `cmp word [rec], 0x15` that ended the walk.
// FUNCTION: GP4 0x0053d1f4
void __cdecl FUN_0053d1f4(gp4::Regs* r) {
    uint32_t low = r->eax;
    uint32_t high = r->edx;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, r->esi, 0x18)));

    uint32_t arith = gp4::flags::logic32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record)));   // or edi, edi
    if (record != nullptr) {
        for (;;) {
            const uint16_t code = *reinterpret_cast<const uint16_t*>(record);
            arith = gp4::flags::sub16(code, 0x15u);                     // cmp word [edi], 0x15
            if (code >= 0x15u) {
                break;
            }
            const uint32_t carry = high & 1u;                           // shr edx, 1
            high >>= 1;
            const uint32_t out = low & 1u;                              // rcr eax, 1
            low = (low >> 1) | (carry << 31);
            record[2] = out ? 0xffu : 0x00u;                            // sbb bl, bl; mov [edi + 2], bl
            record += 0xc;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053d1f4, FUN_0053d1f4, "regs(eax:u32, edx:u32, esi:ptr[0x20]:layout=list_0053d1f4) -> (cf, pf, af, zf, sf, of)")

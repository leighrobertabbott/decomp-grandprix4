#include <gp4/gp4.h>

// FUNCTION: GP4 0x00409e29
// 64-bit / 32-bit unsigned or signed division with 16-bit registers: dividend cx:bx:dx:ax (ax lowest),
// divisor si:di. Signed mode when the word at DAT_00623e14 is negative (the sign of the quotient is
// kept in DAT_00623e10 = si ^ cx). Quotient in dx:ax, remainder in cx:bx. When the high dword of the
// dividend (cx:bx) is >= the divisor the quotient does not fit and the routine returns early with
// the byte DAT_00623e16 = 0xe0 and the (sign-corrected) operands unchanged otherwise.
// The routine ends with `add esp, 0x20`, so the flags it returns (all clear except PF) come from
// the stack pointer, not from the data.
void __cdecl FUN_00409e29(gp4::Regs* r) {
    uint16_t ax = static_cast<uint16_t>(r->eax);
    uint16_t dx = static_cast<uint16_t>(r->edx);
    uint16_t bx = static_cast<uint16_t>(r->ebx);
    uint16_t cx = static_cast<uint16_t>(r->ecx);
    uint16_t si = static_cast<uint16_t>(r->esi);
    uint16_t di = static_cast<uint16_t>(r->edi);

    const bool isSigned = static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00623e14)) < 0;
    uint16_t signWord = 0;
    if (isSigned) {
        const uint16_t originalSi = si;
        if (static_cast<int16_t>(si) < 0) {
            // not si / neg di / jne / inc si: |divisor| as a 32-bit two's complement negate
            si = static_cast<uint16_t>(~si);
            di = static_cast<uint16_t>(0u - di);
            if (di == 0) {
                si = static_cast<uint16_t>(si + 1u);
            }
        }
        signWord = static_cast<uint16_t>(originalSi ^ cx);
        GP4_GLOBAL(uint16_t, 0x00623e10) = signWord;
        if (static_cast<int16_t>(cx) < 0) {
            // sub / sbb chain: negate the 64-bit dividend
            uint64_t dividend = (static_cast<uint64_t>(cx) << 48) | (static_cast<uint64_t>(bx) << 32)
                | (static_cast<uint64_t>(dx) << 16) | ax;
            dividend = 0u - dividend;
            ax = static_cast<uint16_t>(dividend);
            dx = static_cast<uint16_t>(dividend >> 16);
            bx = static_cast<uint16_t>(dividend >> 32);
            cx = static_cast<uint16_t>(dividend >> 48);
        }
    }

    const uint32_t divisor = (static_cast<uint32_t>(si) << 16) | di;
    uint32_t remainder = (static_cast<uint32_t>(cx) << 16) | bx;
    if (remainder >= divisor) {
        // quotient overflow (or division by zero): mov byte [..], 0x70 / add byte [..], 0x70
        GP4_GLOBAL(uint8_t, 0x00623e16) = 0xe0;
    } else {
        uint32_t low = (static_cast<uint32_t>(dx) << 16) | ax;
        uint32_t quotient = 0;
        for (int step = 0; step < 32; ++step) {
            const bool carry = (remainder >> 31) != 0u;
            remainder = (remainder << 1) | (low >> 31);
            low <<= 1;
            quotient <<= 1;
            if (carry || remainder >= divisor) {
                quotient |= 1u;
                remainder -= divisor;
            }
        }
        if (isSigned && static_cast<int16_t>(signWord) < 0) {
            quotient = 0u - quotient;
        }
        ax = static_cast<uint16_t>(quotient);
        dx = static_cast<uint16_t>(quotient >> 16);
        bx = static_cast<uint16_t>(remainder);
        cx = static_cast<uint16_t>(remainder >> 16);
    }

    r->eax = (r->eax & 0xffff0000u) | ax;
    r->edx = (r->edx & 0xffff0000u) | dx;
    r->ebx = (r->ebx & 0xffff0000u) | bx;
    r->ecx = (r->ecx & 0xffff0000u) | cx;
    r->esi = (r->esi & 0xffff0000u) | si;
    r->edi = (r->edi & 0xffff0000u) | di;
    r->eflags = r->eflags & ~0x8d5u;
}
GP4_IMPL(0x00409e29, FUN_00409e29, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32) -> (eax,ebx,ecx,edx,esi,edi,cf,pf,af,zf,sf,of); globals=fuzz:i32")

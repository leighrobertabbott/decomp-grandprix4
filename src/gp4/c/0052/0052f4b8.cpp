#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks the (translated) character in al up in four 0x105-byte records at 0x0082474b whose first
// byte is a character, comparing both sides through the translation table (0x0054d755). On a match
// the 0x104-byte record is copied to 0x0081e7f2 (the current path buffer); otherwise the buffer is
// reset to "<char>" with a terminating 0 at +3 (0x0081e7f5). eax keeps the high half, ah = translated
// key, al = last translated record byte; esi/edi/ecx are left by the copy or the scan.
// The flags are those of the final "cmp ah,al" on a match, else of the last "dec ecx".
// FUNCTION: GP4 0x0052f4b8
void __cdecl FUN_0052f4b8(gp4::Regs* r) {
    uint32_t eax = r->eax;

    gp4::Regs first = *r;
    gp4::call_regs(0x0054d755, first);
    const uint32_t key = first.eax & 0xffu;
    eax = (first.eax & 0xffff0000u) | (key << 8) | key;

    const uint8_t* record = GP4_ARRAY(uint8_t, 0x0082474b);
    uint32_t count = 4u;
    uint32_t arith = 0u;
    bool found = false;
    uint32_t translated = 0u;
    do {
        gp4::Regs next = *r;
        next.eax = (eax & 0xffffff00u) | record[0];
        gp4::call_regs(0x0054d755, next);
        translated = next.eax & 0xffu;
        eax = (eax & 0xffffff00u) | translated;
        arith = gp4::flags::sub8(key, translated);
        if (key == translated) {
            found = true;
            break;
        }
        record += 0x105;
        arith = gp4::flags::add32((uint32_t)(uintptr_t)record - 0x105u, 0x105u);
        arith = gp4::flags::dec32(count, arith);
        --count;
    } while (count != 0u);

    if (found) {
        uint8_t* dest = GP4_ARRAY(uint8_t, 0x0081e7f2);
        for (uint32_t i = 0u; i < 0x104u; ++i) {
            dest[i] = record[i];
        }
        r->edi = 0x0081e7f2u + 0x104u;
        record += 0x104;
        count = 0u;
    } else {
        GP4_GLOBAL(uint8_t, 0x0081e7f2) = (uint8_t)key;
        GP4_GLOBAL(uint8_t, 0x0081e7f5) = 0u;
    }

    r->eax = eax;
    r->ecx = count;
    r->esi = (uint32_t)(uintptr_t)record;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0052f4b8, FUN_0052f4b8, "regs(eax:u32) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=drives_0082474b+xlat_00833e18")

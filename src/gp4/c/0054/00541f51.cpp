#include <gp4/gp4.h>
#include <gp4/flags.h>

// Eases the 0x300 bytes at 0x007a9480 toward a target: for each byte, delta = (int8)(target - cur)
// with target the next byte of the source block (when byte 0x007a9790 == 0; the block pointer is
// [0x007a9788], loaded into esi) or 0 (when it is nonzero); cur += delta / (int8)[0x007a9780]
// (signed 8-bit idiv). Leaves ecx = 0, edi = end of the table, esi advanced past the source block
// (only in the source mode), edx = [0x007a9780], bl = the mode byte, ax = the last idiv result
// (al quotient, ah remainder; the high half of eax is kept); the flags are those of `or ecx, ecx`.

// FUNCTION: GP4 0x00541f51
void __cdecl FUN_00541f51(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007a9790);
    const uint8_t* source = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    if (mode == 0u) {
        source = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x007a9788)));
    }
    uint8_t* table = GP4_ARRAY(uint8_t, 0x007a9480);
    const uint32_t divisor_word = GP4_GLOBAL(uint32_t, 0x007a9780);
    const int divisor = (int8_t)divisor_word;
    uint32_t ax = 0u;

    uint32_t count = 0x300u;
    do {
        uint8_t target = 0u;
        if (mode == 0u) {
            target = *source;
            source += 1;
        }
        const int diff = (int8_t)(uint8_t)(target - *table);
        const int quotient = diff / divisor;
        const int remainder = diff % divisor;
        *table = (uint8_t)(*table + (uint8_t)quotient);
        ax = ((uint32_t)(uint8_t)remainder << 8) | (uint8_t)quotient;
        table += 1;
        --count;
    } while (count != 0u);

    r->eax = (r->eax & 0xffff0000u) | ax;
    r->ebx = (r->ebx & 0xffffff00u) | mode;
    r->ecx = 0u;
    r->edx = divisor_word;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(source));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(table));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x00541f51, FUN_00541f51, "regs(eax:u32, ebx:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00541f51")

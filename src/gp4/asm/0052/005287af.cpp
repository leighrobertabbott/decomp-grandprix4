#include <gp4/gp4.h>
#include <gp4/flags.h>

// Maps the type byte at +0x0c of the esi record to a mode byte stored at 0x81d1d4: 6 -> 0, 9 -> 1,
// 0xc -> 2, anything else -> 3. al ends as the type byte and bl as the mode; the flags are those of
// the last `cmp al, k` executed (k = 6, 9 or 0xc).
// FUNCTION: GP4 0x005287af
void __cdecl FUN_005287af(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t type = GP4_FIELD(uint8_t, record, 0x0c);
    uint32_t mode;
    uint32_t arith;
    if (type == 6u) {
        arith = gp4::flags::sub8(type, 6u);
        mode = 0u;
    } else if (type == 9u) {
        arith = gp4::flags::sub8(type, 9u);
        mode = 1u;
    } else {
        arith = gp4::flags::sub8(type, 0x0cu);
        mode = (type == 0x0cu) ? 2u : 3u;
    }
    GP4_GLOBAL(uint8_t, 0x0081d1d4) = static_cast<uint8_t>(mode);
    r->eax = (r->eax & 0xffffff00u) | type;
    r->ebx = (r->ebx & 0xffffff00u) | mode;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005287af, FUN_005287af, "regs(eax:u32, ebx:u32, esi:ptr[0x40]:u8{=6,9,0x0c,0,7,0x80}) -> (eax, ebx, cf, pf, af, zf, sf, of)")

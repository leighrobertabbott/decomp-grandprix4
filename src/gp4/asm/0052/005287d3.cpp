#include <gp4/gp4.h>
#include <gp4/flags.h>

// Maps the type byte at +0x0c of the esi record to a mode byte stored at 0x81d1d4: 7 -> 0, 0xa -> 1,
// 0xd -> 2, anything else -> 3 (sibling of 0x5287af for the neighbouring type codes). al ends as the
// type byte and bl as the mode; the flags are those of the last `cmp al, k` executed (k = 7, 0xa or
// 0xd).
// FUNCTION: GP4 0x005287d3
void __cdecl FUN_005287d3(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t type = GP4_FIELD(uint8_t, record, 0x0c);
    uint32_t mode;
    uint32_t arith;
    if (type == 7u) {
        arith = gp4::flags::sub8(type, 7u);
        mode = 0u;
    } else if (type == 0x0au) {
        arith = gp4::flags::sub8(type, 0x0au);
        mode = 1u;
    } else {
        arith = gp4::flags::sub8(type, 0x0du);
        mode = (type == 0x0du) ? 2u : 3u;
    }
    GP4_GLOBAL(uint8_t, 0x0081d1d4) = static_cast<uint8_t>(mode);
    r->eax = (r->eax & 0xffffff00u) | type;
    r->ebx = (r->ebx & 0xffffff00u) | mode;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005287d3, FUN_005287d3, "regs(eax:u32, ebx:u32, esi:ptr[0x40]:u8{=7,0x0a,0x0d,0,6,0x80}) -> (eax, ebx, cf, pf, af, zf, sf, of)")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// CRC-16 (polynomial 0x1021, initial value 0, MSB first) of the ecx bytes at esi. The result is the
// low word of eax (the high half is kept) and esi ends past the data; ecx is saved and restored.
// Flags: with no data those of `xor ax, ax`; otherwise those of the last instruction that wrote them in
// the final bit step, `xor ax, 0x1021` (after a carry) or the `shl ax, 1` itself.
// FUNCTION: GP4 0x0054b959
void __cdecl FUN_0054b959(gp4::Regs* r) {
    const uint8_t* data = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t remaining = r->ecx;
    uint32_t crc = 0u;
    uint32_t arith = gp4::flags::logic16(0u);

    while (remaining != 0u) {
        crc ^= static_cast<uint32_t>(*data) << 8;
        ++data;
        for (uint32_t bit = 0u; bit < 8u; ++bit) {
            const uint32_t before = crc & 0xffffu;
            const uint32_t shifted = (before << 1) & 0xffffu;
            const uint32_t carry = (before >> 15) & 1u;
            crc = shifted;
            arith = gp4::flags::szp(shifted, 16) | (carry != 0u ? gp4::flags::CF : 0u)
                  | (((before ^ shifted) & 0x8000u) != 0u ? gp4::flags::OF : 0u);
            if (carry != 0u) {
                crc = shifted ^ 0x1021u;
                arith = gp4::flags::logic16(crc);
            }
        }
        --remaining;
    }

    r->eax = (r->eax & 0xffff0000u) | (crc & 0xffffu);
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(data));
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054b959, FUN_0054b959, "regs(eax:u32, ecx:u32[0..16], esi:ptr[0x40]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of)")

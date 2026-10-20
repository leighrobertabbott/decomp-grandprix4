#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline uint8_t* copyBytes(uint8_t* dst, const uint8_t* src, uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
    return dst + count;
}

// Builds a packet at the buffer [0x00830010]: the dword [0x00833c98], the 16 bytes at 0x007ab4bc,
// the 22 bytes of the slot table at 0x007c2a48, then 24 bytes of the 22 * 24 byte string table at
// 0x007c2b3c for every table slot whose top two bits are exactly 10 (0x80), and an 8 byte trailer
// (0x12435687, 0x66). The packet length (end - start, 50 + 24n bytes) is stored at [0x00830014] and
// comes back in eax. On return ebx = end of the slot table, esi = end of the string table, ecx = 0,
// edi = end of the packet and the flags are those of `sub eax, [0x00830010]` (length = end - start).
// FUNCTION: GP4 0x005401a2
void __cdecl FUN_005401a2(gp4::Regs* r) {
    const uint32_t start = GP4_GLOBAL(uint32_t, 0x00830010);
    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(start));

    *reinterpret_cast<uint32_t*>(out) = GP4_GLOBAL(uint32_t, 0x00833c98);
    out += 4;
    out = copyBytes(out, GP4_ARRAY(const uint8_t, 0x007ab4bc), 0x10u);
    out = copyBytes(out, GP4_ARRAY(const uint8_t, 0x007c2a48), 0x16u);

    const uint8_t* slots = GP4_ARRAY(const uint8_t, 0x007c2a48);
    const uint8_t* text = GP4_ARRAY(const uint8_t, 0x007c2b3c);
    for (uint32_t slot = 0; slot < 0x16u; ++slot) {
        if ((slots[slot] & 0xc0u) == 0x80u) {
            out = copyBytes(out, text, 0x18u);
        }
        text += 0x18;
    }
    *reinterpret_cast<uint32_t*>(out) = 0x12435687u;
    *reinterpret_cast<uint32_t*>(out + 4) = 0x66u;
    out += 8;

    const uint32_t end = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out));
    const uint32_t length = end - GP4_GLOBAL(uint32_t, 0x00830010);
    GP4_GLOBAL(uint32_t, 0x00830014) = length;

    r->eax = length;
    r->ebx = 0x007c2a48u + 0x16u;
    r->ecx = 0u;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(text));
    r->edi = end;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(end, GP4_GLOBAL(uint32_t, 0x00830010)));
}
GP4_IMPL(0x005401a2, FUN_005401a2, "regs() -> (eax, ebx, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pkt_00830010+slots_005401a2")

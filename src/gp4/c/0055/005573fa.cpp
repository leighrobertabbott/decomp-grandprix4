#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x005573fa
// When the mode byte 0x0086857c is 2: reads a 16-bit count (dx) and a nibble index at esi (esi
// advances by 4) and takes the table byte 0x0088f5b2[index & 0xf]. cl == 0x27 shifts it into the
// high nibble (keeping the low one: ch = 0x0f), any other cl keeps its low nibble (ch = 0xf0).
// For `count` consecutive 0x160-byte segment records from ebx (the count is signed 16-bit after
// the first decrement: 0 and anything above 0x8000 do nothing) byte +0x24 becomes
// (byte & ch) | value; the walk wraps by [0x00866ebc] once ebx reaches the ring end [0x00866eb8].
// Ends with eax = value, ch set, dx = the exhausted counter, ebx = the walk position; the flags
// are those of the last `sub dx, 1`. In any other mode only esi += 4 happens.
void __cdecl FUN_005573fa(gp4::Regs* r) {
    const uint32_t esi_in = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2u) {
        r->esi = esi_in + 4u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi_in, 4u));
        return;
    }

    const uint16_t* words = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(esi_in));
    uint16_t count = words[0];
    const uint32_t index = words[1] & 0xfu;
    uint8_t value = GP4_ARRAY(uint8_t, 0x0088f5b2)[index];
    uint8_t keep;
    if ((r->ecx & 0xffu) == 0x27u) {
        value = static_cast<uint8_t>(value << 4);
        keep = 0x0f;
    } else {
        value = static_cast<uint8_t>(value & 0x0fu);
        keep = 0xf0;
    }

    uint32_t rec = r->ebx;
    uint32_t flags = gp4::flags::sub16(count, 1u);
    count = static_cast<uint16_t>(count - 1u);
    while ((count & 0x8000u) == 0u) {
        uint8_t cell = GP4_FIELD(uint8_t, rec, 0x24);
        cell = static_cast<uint8_t>(cell & keep);
        cell = static_cast<uint8_t>(cell | value);
        GP4_FIELD(uint8_t, rec, 0x24) = cell;
        rec += 0x160u;
        if (rec >= GP4_GLOBAL(uint32_t, 0x00866eb8)) {
            rec -= GP4_GLOBAL(uint32_t, 0x00866ebc);
        }
        flags = gp4::flags::sub16(count, 1u);
        count = static_cast<uint16_t>(count - 1u);
    }

    r->eax = value;
    r->ecx = (r->ecx & 0xffff00ffu) | (static_cast<uint32_t>(keep == 0x0fu ? 0x0fu : 0xf0u) << 8);
    r->edx = (r->edx & 0xffff0000u) | count;
    r->ebx = rec;
    r->esi = esi_in + 4u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005573fa, FUN_005573fa, "regs(eax:u32, ecx:u32{0x27,0x27,0x26,0,0xdeadbe27,0x1234,0xff}, edx:u32, ebx:rec[0x8943a0,0x160,0..29], esi:ptr[8]:u16[0..40]) -> (eax, ecx, edx, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0")

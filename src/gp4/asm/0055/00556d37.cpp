#include <gp4/gp4.h>
#include <gp4/flags.h>

// Segment-ring command: esi points at an 8-byte command record (word 0 = a signed length, word +4 = flag
// bits). Wrapped in pushal/popal, so every register but esi comes back unchanged; esi advances by 8 and the
// flags are those of that final `add esi, 8`. When (word +4) & 0xc8 is non-zero:
//   len = ((int16)word0 >> 1) * word[0x934dfc] (16 bit), dx = 0x60 (0xb0 when len is negative), |len| in ax;
//   the list at 0x88efd0 + ((flags >> 4) | (bit 3 ? 0x10 : 0)) is walked until a negative byte; for each type
//   byte (3 becomes 4 when |len| < 0x10) the ring cursor starts at edi, moves back 0x18c0 bytes per entry
//   (wrapping up by [0x866ebc] below [0x866ec0]) and the word (type * 16 + dx) at 0x88eff0 is applied to
//   that record by 0x00556dee.
// FUNCTION: GP4 0x00556d37
void __cdecl FUN_00556d37(gp4::Regs* r) {
    const uint32_t item = r->esi;
    const uint16_t length = GP4_FIELD(uint16_t, item, 0);
    const uint16_t selector = static_cast<uint16_t>(GP4_FIELD(uint16_t, item, 4) & 0xc8u);

    if (selector != 0u) {
        const int16_t half = static_cast<int16_t>(length) >> 1;
        const uint16_t scaled = static_cast<uint16_t>(static_cast<int32_t>(half) * static_cast<int32_t>(static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00934dfc))));
        uint16_t magnitude = scaled;
        if (static_cast<int16_t>(magnitude) < 0) {
            magnitude = static_cast<uint16_t>(0u - magnitude);
        }
        uint16_t base = 0x60u;
        if (static_cast<int16_t>(scaled) < 0) {
            base = static_cast<uint16_t>(base + 0x50u);
        }
        uint16_t pick = static_cast<uint16_t>(selector >> 4);
        if (((selector >> 3) & 1u) != 0u) {
            pick = static_cast<uint16_t>(pick | 0x10u);
        }

        uint32_t record = r->edi;
        const uint8_t* list = &GP4_ARRAY(uint8_t, 0x0088efd0)[static_cast<int16_t>(pick)];
        for (;;) {
            uint16_t type = *list++;
            if (static_cast<int8_t>(type) < 0) {
                break;
            }
            record -= 0x18c0u;
            if (!(record >= GP4_GLOBAL(uint32_t, 0x00866ec0))) {
                record += GP4_GLOBAL(uint32_t, 0x00866ebc);
            }
            if (type == 3u && static_cast<int16_t>(magnitude) < 0x10) {
                type = 4u;
            }
            GP4_GLOBAL(uint16_t, 0x0088eff0) = static_cast<uint16_t>((type << 4) + base);

            gp4::Regs c = {};
            c.eax = magnitude;
            c.ebx = record;
            c.ecx = type;
            c.edx = base;
            c.esi = 0x0088eff0u;
            c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(list));
            c.eflags = r->eflags;
            gp4::call_regs(0x00556dee, c);
        }
    }

    r->esi = item + 8u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(item, 8u));
}
GP4_IMPL(0x00556d37, FUN_00556d37, "regs(esi:ptr[0x10]:u16{0,8,0x40,0x48,0x80,0x88,0xc0,0xc8,0xff,0x7fff,0x8000,0xffff,3,0x1234}, edi:rec[0x8943a0,0x160,18..29]) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+ring_0043d30c+ptr_0086850c")

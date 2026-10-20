#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream command handler (mode byte 0x0086857c): in any mode but 2 the cursor esi just skips six
// bytes. In mode 2 three words are read at esi (esi += 6): id, row count, colour. The bit
// (1 << id) masked with dword [record + 0x2c] is handed to 0x005583a3 (edi = the record). For
// `count` consecutive 0x160-byte records starting at ebx (wrapping by [0x866ebc] once the address
// reaches the ring end [0x866eb8]) the colour nibble is stored into the cell the byte table
// 0x00963068 assigns to the id (0xff = none; value / 2 is the byte of the record at +0x58, an odd
// value picks the high nibble), id 10 also raises flag bits +0x48 / +0x70, and the bit is or-ed into
// record dword +0xac. 0x005583a3 then gets the bit for the record the walk ended on. A zero count
// stops after loading eax = colour, ecx = 0, edx = id. ebx and edi are saved and restored; the
// flags are those of the final `add esi, 6`.
// FUNCTION: GP4 0x005586e1
void __cdecl FUN_005586e1(gp4::Regs* r) {
    const uint32_t source = r->esi;
    r->esi = source + 6u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(source, 6u));
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2u) {
        return;
    }

    const uint32_t id = GP4_FIELD(uint16_t, source, 0);
    uint32_t count = GP4_FIELD(uint16_t, source, 2);
    const uint32_t colour = GP4_FIELD(uint16_t, source, 4);
    r->eax = colour;
    r->ecx = count;
    r->edx = id;
    if (count == 0u) {
        return;
    }

    const uint32_t bit = 1u << (id & 31u);
    const uint32_t low = colour & 0xfu;
    const uint32_t high = (low << 4) & 0xffu;
    uint32_t rec = r->ebx;

    {
        gp4::Regs c = *r;
        c.eax = bit & GP4_FIELD(uint32_t, rec, 0x2c);
        c.edi = rec;
        c.esi = id;
        c.ebp = bit;
        c.eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(c.eax));
        gp4::call_regs(0x005583a3, c);
    }

    do {
        if (id == 10u) {
            GP4_FIELD(uint32_t, rec, 0x48) |= 0x800u;
            GP4_FIELD(uint32_t, rec, 0x48) |= 0x400u;
            GP4_FIELD(uint8_t, rec, 0x70) |= 2u;
        }
        const uint32_t cell = GP4_ARRAY(const uint8_t, 0x00963068)[id];
        if (cell != 0xffu) {
            const uint32_t at = rec + (cell >> 1);
            if ((cell & 1u) == 0u) {
                GP4_FIELD(uint8_t, at, 0x58) = static_cast<uint8_t>(GP4_FIELD(uint8_t, at, 0x58) & 0xf0u);
                GP4_FIELD(uint8_t, at, 0x58) = static_cast<uint8_t>(GP4_FIELD(uint8_t, at, 0x58) | low);
            } else {
                GP4_FIELD(uint8_t, at, 0x58) = static_cast<uint8_t>(GP4_FIELD(uint8_t, at, 0x58) & 0x0fu);
                GP4_FIELD(uint8_t, at, 0x58) = static_cast<uint8_t>(GP4_FIELD(uint8_t, at, 0x58) | high);
            }
        }
        GP4_FIELD(uint32_t, rec, 0xac) |= bit;
        rec += 0x160u;
        if (rec >= GP4_GLOBAL(uint32_t, 0x00866eb8)) {
            rec -= GP4_GLOBAL(uint32_t, 0x00866ebc);
        }
        --count;
    } while (count != 0u);

    const uint32_t last = bit & GP4_FIELD(uint32_t, rec, 0x2c);
    {
        gp4::Regs c = *r;
        c.eax = last;
        c.ecx = 0u;
        c.edx = (high << 8) | low;
        c.esi = id;
        c.ebp = bit;
        c.edi = rec;
        c.ebx = rec;
        c.eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(last));
        gp4::call_regs(0x005583a3, c);
    }

    r->eax = last;
    r->ecx = 0u;
    r->edx = (high << 8) | low;
    r->ebp = bit;
}
GP4_IMPL(0x005586e1, FUN_005586e1, "regs(eax:u32, ecx:u32, edx:u32, ebx:rec[0x8943a0,0x160,0..29], ebp:u32, esi:ptr[8]:u16{=0,1,2,3,5,8,10,15,24,31}) -> (eax, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+mode_0086857c+wave_005587a0")

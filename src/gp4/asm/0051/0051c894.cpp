#include <gp4/gp4.h>
#include <gp4/flags.h>

// Word at 0x9e1b20 + index*2: 0xffff (none) or a byte offset into the 0x20-byte texture entry table
// at 0x6975ec.
static __declspec(noinline) uint32_t Word9e1b20(uint32_t index) {
    return GP4_ARRAY(uint16_t, 0x009e1b20)[index];
}

// pushal/popal wrapper. ebx = a texture slot number: its table word is 0xffff (nothing: the flags are
// those of `cmp bx, -1`) or the byte offset of an entry of the table at 0x6975ec; edi = the entry's
// first dword (the image buffer), plus 0x10000 when the entry's word +0x14 has bit 7 clear, is handed to
// the row copier pair 0x0051c865 (ebx = the entry address). Every GPR comes back unchanged; the flags are
// those the copier leaves.
// FUNCTION: GP4 0x0051c894
void __cdecl FUN_0051c894(gp4::Regs* r) {
    const uint32_t offset = Word9e1b20(r->ebx);
    uint32_t f = gp4::flags::sub16(offset, 0xffffu);
    if (offset != 0xffffu) {
        const uint32_t entry = 0x006975ecu + offset;
        uint32_t edi = GP4_FIELD(uint32_t, entry, 0);
        f = gp4::flags::logic16(GP4_FIELD(uint16_t, entry, 0x14) & 0x80u);
        if ((GP4_FIELD(uint16_t, entry, 0x14) & 0x80u) == 0u) {
            f = gp4::flags::add32(edi, 0x10000u);
            edi += 0x10000u;
        }
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.ebx = entry;
        c.edi = edi;
        c.eflags = gp4::flags::merge(c.eflags, f);
        gp4::call_regs(0x0051c865, c);
        f = c.eflags;
    }
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x0051c894, FUN_0051c894, "regs(ebx:u32{=0x43d,0x43e,0x43f,0x440}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=args_0051c865+slots_0051c894")

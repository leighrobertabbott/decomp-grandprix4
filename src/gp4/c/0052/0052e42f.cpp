#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) uint32_t name_entry_address(uint32_t index) {
    return index * 4u + 0x00826dbeu;
}

// Builds the file name at 0x0082471a from the record at [0x00826d11]. If byte +0x52 is not 0xff the
// first 0x30 bytes are copied and a NUL appended. Otherwise the base name, ended by '.' or NUL, is
// copied (the scan's `loop` jumps back onto the `mov ecx, 0x30` that precedes it, so the counter is
// reset every character and the name length is not bounded), '.' is appended and then three
// extension characters follow: the 4-byte entry 0x00826dbe + 4 * byte(+0x50) is read byte by byte;
// a '?' is replaced by the letter 0x0052f4f9 derives from [0x007ad894], and each character goes
// through the case table 0x0054d755; a NUL ends the name.
// al/bl are the only partial registers written, ecx ends 0, esi/edi are the final cursors; the flags
// are `cmp byte [esi+0x52], 0xff` on the copy path, else those of the last 0x0054d755 call.
// FUNCTION: GP4 0x0052e42f
void __cdecl FUN_0052e42f(gp4::Regs* r) {
    uint32_t src = GP4_GLOBAL(uint32_t, 0x00826d11);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x0082471a);
    const uint8_t mode = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(src + 0x52u));
    uint32_t flags = gp4::flags::sub8(mode, 0xffu);

    if (mode != 0xffu) {
        const uint8_t* from = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(src));
        for (uint32_t i = 0; i != 0x30u; ++i) {
            dst[i] = from[i];
        }
        dst[0x30] = 0;
        r->eax = r->eax & 0xffffff00u;
        r->ecx = 0;
        r->esi = src + 0x30u;
        r->edi = 0x0082471au + 0x31u;
        r->eflags = gp4::flags::merge(r->eflags, flags);
        return;
    }

    uint8_t* out = dst;
    const uint8_t* in = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(src));
    for (;;) {
        const uint8_t ch = *in++;
        flags = gp4::flags::sub8(ch, 0x2eu);
        if (ch == 0x2eu) {
            break;
        }
        flags = gp4::flags::logic8(ch);
        if (ch == 0u) {
            break;
        }
        *out++ = ch;
    }
    *out++ = 0x2eu;

    src = GP4_GLOBAL(uint32_t, 0x00826d11);
    const uint32_t nameIndex = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(src + 0x50u));
    const uint8_t* name = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(name_entry_address(nameIndex)));

    gp4::Regs letter = *r;
    letter.eax = GP4_GLOBAL(uint32_t, 0x007ad894);
    letter.eflags = flags;
    gp4::call_regs(0x0052f4f9, letter);
    const uint8_t fill = static_cast<uint8_t>(letter.eax);
    const uint32_t ebx = (r->ebx & 0xffffff00u) | fill;

    uint32_t eax = letter.eax;
    flags = letter.eflags;
    for (uint32_t n = 0; n != 3u; ++n) {
        uint8_t ch = *name++;
        if (ch == 0x3fu) {
            ch = fill;
        }
        gp4::Regs map = *r;
        map.eax = (eax & 0xffffff00u) | ch;
        map.ebx = ebx;
        map.eflags = flags;
        gp4::call_regs(0x0054d755, map);
        eax = map.eax;
        flags = map.eflags;
        *out++ = static_cast<uint8_t>(map.eax);
    }
    *out++ = 0;

    r->eax = eax & 0xffffff00u;
    r->ebx = ebx;
    r->ecx = 0;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(name));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out));
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052e42f, FUN_0052e42f, "regs(eax:u32, ebx:u32) -> (eax, ebx, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=rec_0052e42f")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Restores a 0x300-stride indexed block from the record at esi: {+0 base (0 = default via 0x005418cd),
// +4 byte count, +8 block index, +0xc dword -> [0x830824], +0x10 the bytes}. The count bytes are copied
// (`rep movsb`, direction flag clear) to base + index * 0x300, then [0x830828] is incremented.
// Leaves eax = the +0xc dword, ecx = 0, esi = source end, edi = destination end; the flags are those of
// the final `inc dword [0x830828]` with CF kept from `add esi, 0x10`.
// FUNCTION: GP4 0x00541949
void __cdecl FUN_00541949(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(r->esi);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = GP4_FIELD(uint32_t, rec, 0);
    gp4::call_regs(0x005418cd, c);

    const uint32_t count = GP4_FIELD(uint32_t, rec, 4);
    const uint32_t index = GP4_FIELD(uint32_t, rec, 8);
    const uint32_t tail = GP4_FIELD(uint32_t, rec, 0xc);
    GP4_GLOBAL(uint32_t, 0x00830824) = tail;

    const uint32_t addFlags = gp4::flags::add32(r->esi, 0x10u);
    const uint8_t* in = rec + 0x10;
    uint8_t* out = reinterpret_cast<uint8_t*>(c.eax + index * 0x300u);
    for (uint32_t n = count; n != 0u; --n) {
        *out++ = *in++;
    }

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x00830828);
    GP4_GLOBAL(uint32_t, 0x00830828) = before + 1u;

    r->eax = tail;
    r->ebx = c.ebx;
    r->ecx = 0u;
    r->edx = c.edx;
    r->esi = reinterpret_cast<uint32_t>(in);
    r->edi = reinterpret_cast<uint32_t>(out);
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(before, addFlags));
}
GP4_IMPL(0x00541949, FUN_00541949, "regs(esi:ptr[0x310]:layout=rec_00541949) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00541914")

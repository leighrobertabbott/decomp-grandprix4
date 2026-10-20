#include <gp4/gp4.h>
#include <gp4/flags.h>

// Key lookup in the 12-entry table of 0x005341d4 for the key byte *[ebx+4]. With the switch dword
// [0x007a979c] zero the lookup's ecx is only stored in [0x00828170]. Otherwise the lookup's eax is
// stored through the pointer [ebx+8], the bit (1 << cl) is set in the masks [0x007ab0ec] and [0x007a9798],
// the unaligned counter 0x00828293 is incremented and the item chain at 0x00852837 is walked with
// eax = 1 (0x0053e4db) before the next selectable menu item is chosen (0x0053cc19).
// FUN_005341d4 gives eax/ecx and the flags; the arm without the switch leaves exactly those.

// One helper per arm so each keeps its own call to 0x005341d4.
static __declspec(noinline) void LookupStoreIndex(gp4::Regs& c, uint32_t record) {
    const uint32_t keyPointer = GP4_FIELD(uint32_t, record, 4);
    c.eax = (keyPointer & 0xffffff00u) | GP4_FIELD(uint8_t, keyPointer, 0);
    gp4::call_regs(0x005341d4, c);
    GP4_GLOBAL(uint32_t, 0x00828170) = c.ecx;
}

static __declspec(noinline) void LookupStoreValue(gp4::Regs& c, uint32_t record) {
    const uint32_t keyPointer = GP4_FIELD(uint32_t, record, 4);
    c.eax = (keyPointer & 0xffffff00u) | GP4_FIELD(uint8_t, keyPointer, 0);
    gp4::call_regs(0x005341d4, c);
    c.edx = GP4_FIELD(uint32_t, record, 8);
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(c.edx)) = c.eax;
}

// FUNCTION: GP4 0x00534f57
void __cdecl FUN_00534f57(gp4::Regs* r) {
    const uint32_t record = r->ebx;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    if (GP4_GLOBAL(uint32_t, 0x007a979c) == 0u) {
        LookupStoreIndex(c, record);
    } else {
        LookupStoreValue(c, record);

        const uint32_t bit = 1u << (c.ecx & 31u);
        c.eax = bit;
        const uint32_t maskA = GP4_GLOBAL(uint32_t, 0x007ab0ec) | bit;
        GP4_GLOBAL(uint32_t, 0x007ab0ec) = maskA;
        const uint32_t maskB = GP4_GLOBAL(uint32_t, 0x007a9798) | bit;
        GP4_GLOBAL(uint32_t, 0x007a9798) = maskB;
        const uint32_t orFlags = gp4::flags::logic32(maskB);
        const uint32_t counter = GP4_FIELD(uint32_t, 0x00828293u, 0);
        GP4_FIELD(uint32_t, 0x00828293u, 0) = counter + 1u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(counter, orFlags));

        c.eax = 1u;
        c.esi = 0x00852837u;
        gp4::call_regs(0x0053e4db, c);
        gp4::call_regs(0x0053cc19, c);
    }

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00534f57, FUN_00534f57, "regs(ebx:ptr[0x10]:u32{=0x82829b,0x82829c,0x82829d,0x8282a3,0x8282a7,0x8282ab,0x8282af,0x8282b3,0x8282b7,0x8282c3,0x8282c7,0x828214}) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=head_00852837+tab_0082829b+menu_0084300a+sel_0053cc19")

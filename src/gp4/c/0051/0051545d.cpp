#include <gp4/gp4.h>
#include <gp4/flags.h>

// esi = a record whose dword +0x28 is an elapsed time in ms. Selects the readout table entry 7 (0x00514def),
// writes the time readout through 0x005142c3 (edi = output cursor), then clears bit 4 of the byte +0x89 of the
// record and sets 0x00954169 to 0xff. Registers come from the readout callee; the flags are those of the `and`.
// FUNCTION: GP4 0x0051545d
void __cdecl FUN_0051545d(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = 7u;
    gp4::call_regs(0x00514def, c);
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
    c.eax = GP4_FIELD(uint32_t, record, 0x28);
    gp4::call_regs(0x005142c3, c);

    uint8_t* after = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
    const uint8_t value = static_cast<uint8_t>(GP4_FIELD(uint8_t, after, 0x89) & 0xefu);
    GP4_FIELD(uint8_t, after, 0x89) = value;
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xffu;
    *r = c;
    r->eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(value));
}
GP4_IMPL(0x0051545d, FUN_0051545d, "regs(esi:ptr[0x98]:i32[-100..1200000], edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu-entry refresh (esi = the 0x22e-byte item record, edi = the readout buffer): loads the colour bytes
// of table row 8 into 0x0088fd94.. (0x00514def), then picks the value to show: record byte +0x226 when
// bit 4 of byte +0x8b is set (0x004080c1), else the table byte 0x0066d962[0x2c * k + 0x0066d936[k]] with
// k = min(((byte +0x7c) - 1) & 0x3f, 0x15). 0x00510e71 prints it as two digits at edi; then bit 6 of
// byte +0x89 is cleared and 0x00954169 is set to 0xff. Registers come from the printing routine, the
// flags are those of the final `and byte [esi+0x89], 0xbf`.
// FUNCTION: GP4 0x005153db
void __cdecl FUN_005153db(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    c.eax = 8u;
    gp4::call_regs(0x00514def, c);

    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.esi));
    const uint32_t state = record[0x8b];
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(state & 0x10u));
    if ((state & 0x10u) != 0u) {
        gp4::call_regs(0x004080c1, c);
    } else {
        uint32_t row = (static_cast<uint32_t>(record[0x7c]) - 1u) & 0x3fu;
        if (static_cast<int32_t>(row) > 0x15) {
            row = 0x15u;
        }
        const uint32_t column = GP4_ARRAY(uint8_t, 0x0066d936)[row];
        const uint32_t base = row * 0x2cu;
        c.edx = column;
        c.eax = (base & 0xffffff00u) | GP4_ARRAY(uint8_t, 0x0066d962)[column + base];
    }
    gp4::call_regs(0x00510e71, c);

    uint8_t* flags_byte = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi)) + 0x89;
    const uint32_t cleared = *flags_byte & 0xbfu;
    *flags_byte = static_cast<uint8_t>(cleared);
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xffu;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(cleared));
    *r = c;
}
GP4_IMPL(0x005153db, FUN_005153db, "regs(esi:ptr[0x22e]:bytes, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+hudpos_00514def+ctabs_005153db")

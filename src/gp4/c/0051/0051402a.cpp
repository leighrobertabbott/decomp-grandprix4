#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the mode dispatcher 0x005141b8 with the mode dword 0x0088ff30 set in turn.
static void RunMode(gp4::Regs& c, uint32_t mode) {
    GP4_GLOBAL(uint32_t, 0x0088ff30) = mode;
    gp4::call_regs(0x005141b8, c);
}

// Unless the master byte 0x0066d7a4 is set: for the car [0x00679d30] (skipped when its byte +0x8d has
// bit 4 set and its byte +0x84 is 7) the readout labels are drawn by 0x00514e5d, then the input
// handlers of modes 0xb, 2, 5, 6, 8 run through 0x005141b8 (0x0088fdb0 = 0x80000000, word 0x0088ff04
// = 0xffff before them), 0x0040f8d9 runs, then modes 0xa (0x0088ff00 = -1), 7 (byte 0x0088fefc =
// 0xff), 0xc (byte 0x0066d8ab = 0x80), 0 (0x0088ff3c = -1), mode 4 nine times with al = the key byte
// 0x0088ff47[ecx] (ecx = 0..8) and mode 1. Finally bit 5 of car byte +0x83 is cleared. pushal/popal
// restore every GPR; the flags are those of the final `and` (or of the master-byte test).
// FUNCTION: GP4 0x0051402a
void __cdecl FUN_0051402a(gp4::Regs* r) {
    const uint32_t master = GP4_GLOBAL(uint8_t, 0x0066d7a4);
    if (master != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(master));
        return;
    }

    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00679d30)));
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));

    const uint32_t bits = car[0x8d] & 0x10u;
    uint32_t arith = gp4::flags::logic8(bits);
    bool skip = false;
    if (bits != 0u) {
        arith = gp4::flags::sub8(car[0x84], 7u);
        skip = (car[0x84] == 7u);
    }

    if (!skip) {
        c.eflags = gp4::flags::merge(c.eflags, arith);
        gp4::call_regs(0x00514e5d, c);
        GP4_GLOBAL(uint32_t, 0x0088fdb0) = 0x80000000u;
        RunMode(c, 0xbu);
        GP4_GLOBAL(uint16_t, 0x0088ff04) = 0xffffu;
        RunMode(c, 2u);
        RunMode(c, 5u);
        RunMode(c, 6u);
        RunMode(c, 8u);
        gp4::call_regs(0x0040f8d9, c);
        GP4_GLOBAL(uint32_t, 0x0088ff00) = 0xffffffffu;
        RunMode(c, 0xau);
        GP4_GLOBAL(uint8_t, 0x0088fefc) = 0xffu;
        RunMode(c, 7u);
        GP4_GLOBAL(uint8_t, 0x0066d8ab) = 0x80u;
        RunMode(c, 0xcu);
        GP4_GLOBAL(uint32_t, 0x0088ff3c) = 0xffffffffu;
        RunMode(c, 0u);
        c.ecx = 0u;
        for (;;) {
            c.eax = (c.eax & 0xffffff00u) | GP4_ARRAY(uint8_t, 0x0088ff47)[c.ecx];
            RunMode(c, 4u);
            c.ecx = c.ecx + 1u;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.ecx, 9u));
            if (c.ecx == 9u) {
                break;
            }
        }
        RunMode(c, 1u);
    }

    uint8_t* const record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
    const uint8_t cleared = static_cast<uint8_t>(record[0x83] & 0xdfu);
    record[0x83] = cleared;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(cleared));
}
GP4_IMPL(0x0051402a, FUN_0051402a, "regs(eax:u32, ecx:u32, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=readout_00515187+glyph_0051d329+carbg_0066fef0+order_00418f8f+state_0040f898+engine_00413606+state_00404ed0+state_00404f33+drive_004054e6+lanes_00429fcd+state_0041242f+player_0040f8d9+disp_005141b8+h4_005223fe")

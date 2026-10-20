#include <gp4/gp4.h>
#include <gp4/flags.h>

// Circuit record [0x7ad894] of the 17 x 0xe8-byte block at 0x7ab760 (the first record's driver name is
// "Mika Hakkinen"); out of line so only the block base is seen as a global.
static __declspec(noinline) uint8_t* CircuitRecord(uint32_t circuit) {
    return GP4_ARRAY(uint8_t, 0x007ab760) + circuit * 0xe8u;
}

// `rep movsb` of n bytes (direction flag clear).
static __declspec(noinline) void CopyBytes(uint8_t* dst, const uint8_t* src, uint32_t n) {
    for (uint32_t i = 0u; i != n; ++i) {
        dst[i] = src[i];
    }
}

// Copies the name of driver `driver` (24-byte entries from "michael schumacher" at 0x7c2b3c) to
// rec + nameOff, then the name of that driver's team (13-byte entries from "ferrari" at 0x7c2d4c,
// team = 0x5484eb(driver)) to rec + teamOff. Registers follow the original's movsb/esi/edi use.
static inline void CopyDriverAndTeam(gp4::Regs* r, uint8_t* rec, uint32_t driver, uint32_t nameOff,
                                     uint32_t teamOff) {
    const uint8_t* name = GP4_ARRAY(const uint8_t, 0x007c2b3c) + driver * 0x18u;
    CopyBytes(rec + nameOff, name, 0x18u);
    r->esi = reinterpret_cast<uint32_t>(name) + 0x18u;
    r->edi = reinterpret_cast<uint32_t>(rec) + nameOff + 0x18u;
    r->ecx = 0u;
    r->eax = driver;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(reinterpret_cast<uint32_t>(rec), nameOff));
    gp4::call_regs(0x005484eb, *r);
    const uint8_t* team = GP4_ARRAY(const uint8_t, 0x007c2d4c) + r->eax * 0xdu;
    CopyBytes(rec + teamOff, team, 0xdu);
    r->esi = reinterpret_cast<uint32_t>(team) + 0xdu;
    r->edi = reinterpret_cast<uint32_t>(rec) + teamOff + 0xdu;
    r->ecx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(reinterpret_cast<uint32_t>(rec), teamOff));
}

// Updates the two record slots of the current circuit [0x7ad894] (17 x 0xe8-byte records at
// 0x7ab760) when 0x52ceef finds at most one flagged driver of each kind: after refreshing the date
// word [0x82459c] (0x52acfa), slot 1 (+0x00 name, +0x18 team, +0x27 = 0xff, +0x28 time, +0x2c date)
// takes driver [0x7c2f1c] and time [0x7c2f20] when [0x7c2e7f] >= 1 and the time is below the
// stored one (unsigned); slot 2 (+0x2e, +0x46, +0x53, +0x54, +0x58) takes driver [0x7c2f1e] and
// time [0x7c2f24] when the word [0x7ad8a2] >= 100 and that time is below the stored one.
// FUNCTION: GP4 0x0054f8c2
void __cdecl FUN_0054f8c2(gp4::Regs* r) {
    gp4::call_regs(0x0052ceef, *r);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(r->eax, 1u));
    if (r->eax > 1u) {
        return;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(r->edx, 1u));
    if (r->edx > 1u) {
        return;
    }
    gp4::call_regs(0x0052acfa, *r);

    const uint32_t circuit = GP4_GLOBAL(uint32_t, 0x007ad894);
    r->eax = circuit * 0xe8u;
    uint8_t* rec = CircuitRecord(circuit);
    r->ebx = reinterpret_cast<uint32_t>(rec);

    const uint8_t gate1 = GP4_GLOBAL(uint8_t, 0x007c2e7f);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(gate1, 1u));
    if (static_cast<int8_t>(gate1) >= 1) {
        const uint32_t time1 = GP4_GLOBAL(uint32_t, 0x007c2f20);
        r->eax = time1;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(time1, GP4_FIELD(uint32_t, rec, 0x28)));
        if (time1 < GP4_FIELD(uint32_t, rec, 0x28)) {
            CopyDriverAndTeam(r, rec, GP4_GLOBAL(uint16_t, 0x007c2f1c), 0x00u, 0x18u);
            r->eax = GP4_GLOBAL(uint32_t, 0x007c2f20);
            GP4_FIELD(uint32_t, rec, 0x28) = r->eax;
            const uint16_t date = GP4_GLOBAL(uint16_t, 0x0082459c);
            r->eax = (r->eax & 0xffff0000u) | date;
            GP4_FIELD(uint16_t, rec, 0x2c) = date;
            GP4_FIELD(uint8_t, rec, 0x27) = 0xffu;
        }
    }

    const uint16_t gate2 = GP4_GLOBAL(uint16_t, 0x007ad8a2);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub16(gate2, 0x64u));
    if (gate2 < 0x64u) {
        return;
    }
    const uint32_t time2 = GP4_GLOBAL(uint32_t, 0x007c2f24);
    r->eax = time2;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(time2, GP4_FIELD(uint32_t, rec, 0x54)));
    if (time2 >= GP4_FIELD(uint32_t, rec, 0x54)) {
        return;
    }
    CopyDriverAndTeam(r, rec, GP4_GLOBAL(uint16_t, 0x007c2f1e), 0x2eu, 0x46u);
    r->eax = GP4_GLOBAL(uint32_t, 0x007c2f24);
    GP4_FIELD(uint32_t, rec, 0x54) = r->eax;
    const uint16_t date = GP4_GLOBAL(uint16_t, 0x0082459c);
    r->eax = (r->eax & 0xffff0000u) | date;
    GP4_FIELD(uint16_t, rec, 0x58) = date;
    GP4_FIELD(uint8_t, rec, 0x53) = 0xffu;
}
GP4_IMPL(0x0054f8c2, FUN_0054f8c2, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894+neg22_00520023")

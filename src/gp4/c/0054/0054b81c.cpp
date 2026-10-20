#include <gp4/gp4.h>
#include <gp4/flags.h>

// Inverse of 0x0054b7c3: restores the setup entries of the drivers whose flag byte (the 22 bytes at
// 0x007c2a48) has bit 6 set from the 22 * 0x3c byte snapshot at 0x00822300: the 4-byte header goes
// to the entry (0x0052a260) of that driver at the current circuit [0x007ad894], and the 0x38-byte
// sub-record to the one (0x0052a279) selected by snapshot byte 0 (reduced with `and 1` when
// snapshot byte 2 is nonzero). Each iteration is a pushal/popal pair, so only the loop counters
// survive: ebx = 0x007c2a5e, edx = 0x16, esi = 0x528 and the flags of `cmp esi, 0x528` (equal).
// FUNCTION: GP4 0x0054b81c
void __cdecl FUN_0054b81c(gp4::Regs* r) {
    const uint8_t* flag_bytes = GP4_ARRAY(const uint8_t, 0x007c2a48);
    const uint8_t* snapshot = GP4_ARRAY(const uint8_t, 0x00822300);
    uint32_t driver = 0;
    do {
        if ((flag_bytes[driver] & 0x40u) != 0u) {
            gp4::Regs head = {};
            head.eax = driver;
            head.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
            gp4::call_regs(0x0052a260, head);
            uint8_t* header = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(head.eax));
            uint32_t selector = snapshot[0];
            if (snapshot[2] != 0u) {
                selector &= 1u;
            }
            for (uint32_t i = 0; i < 4u; ++i) {
                header[i] = snapshot[i];
            }
            gp4::Regs body = {};
            body.eax = driver;
            body.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
            body.ecx = selector;
            gp4::call_regs(0x0052a279, body);
            uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(body.eax));
            for (uint32_t i = 0; i < 0x38u; ++i) {
                record[i] = snapshot[4u + i];
            }
        }
        snapshot += 0x3c;
        ++driver;
    } while (driver != 0x16u);
    const uint32_t consumed = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(snapshot)) - 0x00822300u;
    r->ebx = 0x007c2a48u + driver;
    r->edx = driver;
    r->esi = consumed;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(consumed, 0x528u));
}
GP4_IMPL(0x0054b81c, FUN_0054b81c, "regs() -> (ebx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=drv22_0052d402+trk_007ad894+circ_007adb50+stamp_00535a20")

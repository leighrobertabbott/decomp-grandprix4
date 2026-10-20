#include <gp4/gp4.h>
#include <gp4/flags.h>

// Snapshots the setup entries of the 22 drivers (current circuit [0x007ad894]) into the 22 * 0x3c
// byte buffer at 0x00822300: for each driver the 4-byte header of the entry (0x0052a260) followed by
// the 0x38-byte sub-record (0x0052a279) selected by the header's byte 0 (reduced with `and 1` when
// the header's byte 2 is nonzero). Ends with eax = address of the last sub-record, esi = its end,
// ebx = the circuit, ecx = 0, edx = 0x16, edi = 0x528 (bytes written) and the flags of
// `cmp edi, 0x528` (equal).
// FUNCTION: GP4 0x0054b7c3
void __cdecl FUN_0054b7c3(gp4::Regs* r) {
    uint8_t* out = GP4_ARRAY(uint8_t, 0x00822300);
    uint32_t driver = 0;
    uint32_t sub_record = 0;
    uint32_t circuit = 0;
    do {
        circuit = GP4_GLOBAL(uint32_t, 0x007ad894);
        gp4::Regs head = {};
        head.eax = driver;
        head.ebx = circuit;
        gp4::call_regs(0x0052a260, head);
        const uint8_t* header = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(head.eax));
        uint32_t selector = header[0];
        if (header[2] != 0u) {
            selector &= 1u;
        }
        for (uint32_t i = 0; i < 4u; ++i) {
            out[i] = header[i];
        }
        out += 4;
        circuit = GP4_GLOBAL(uint32_t, 0x007ad894);
        gp4::Regs body = {};
        body.eax = driver;
        body.ebx = circuit;
        body.ecx = selector;
        gp4::call_regs(0x0052a279, body);
        sub_record = body.eax;
        const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(sub_record));
        for (uint32_t i = 0; i < 0x38u; ++i) {
            out[i] = record[i];
        }
        out += 0x38;
        ++driver;
    } while (driver != 0x16u);
    const uint32_t written = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out)) - 0x00822300u;
    r->eax = sub_record;
    r->ebx = circuit;
    r->ecx = 0u;
    r->edx = driver;
    r->esi = sub_record + 0x38u;
    r->edi = written;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(written, 0x528u));
}
GP4_IMPL(0x0054b7c3, FUN_0054b7c3, "regs() -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=trk_007ad894+circ_007adb50")

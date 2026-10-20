#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the three 0x3c-byte records at 0x827160. A record whose byte +5 is 3 gets dword +0xc = 0
// and +0x10 = 6, then for each bit k = 0..6 set in its byte +4 the dword at +0x28 + 4k is set to
// 0x2000 (bit 1: 0x1555). The k = 5 / 6 stores spill into the next record (+0x3c / +0x40 are its
// +0 / +4..+7), which is read afterwards. edx ends as the last byte +4 loaded (else unchanged),
// ebx = end of the table (0x827214), ecx = 0; the flags are those of the final `dec ecx` (1 -> 0)
// with CF from the preceding `add ebx, 0x3c`.
// FUNCTION: GP4 0x00530ae7
void __cdecl FUN_00530ae7(gp4::Regs* r) {
    uint32_t record = 0x00827160u;
    uint32_t edx = r->edx;
    uint32_t arith = 0u;
    uint32_t remaining = 3u;
    do {
        uint8_t* p = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record));
        if (p[5] == 3u) {
            GP4_FIELD(uint32_t, p, 0xc) = 0u;
            GP4_FIELD(uint32_t, p, 0x10) = 6u;
            edx = p[4];
            for (uint32_t k = 0u; k != 7u; ++k) {
                if (((edx >> k) & 1u) != 0u) {
                    if (k == 1u) {
                        GP4_FIELD(uint32_t, p, 0x2c) = 0x1555u;
                    } else {
                        GP4_FIELD(uint32_t, p, 0x28 + k * 4u) = 0x2000u;
                    }
                }
            }
        }
        arith = gp4::flags::add32(record, 0x3cu);
        record += 0x3cu;
        remaining -= 1u;
    } while (remaining != 0u);
    r->ebx = record;
    r->ecx = 0u;
    r->edx = edx;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, arith));
}
GP4_IMPL(0x00530ae7, FUN_00530ae7, "regs() -> (ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rec_00530ae7")

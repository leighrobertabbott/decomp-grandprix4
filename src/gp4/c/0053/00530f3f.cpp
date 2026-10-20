#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks [0x007a97a4]+1 device records of 0x3c bytes from 0x00827160 in step with the dwords at
// 0x007ad880. A record whose byte +5 is 2 is skipped (its dword stays); otherwise the dword is 0
// when byte +4 is 0, else the key selected by the accepted routine 0x00530f75 (ebx = record).
// Leaves ebx / edi one past the last record / dword, ecx = 0, eax = the last selected key (unchanged
// when nothing was selected) and the flags of the final `dec ecx` (CF from `add ebx, 0x3c`).
// FUNCTION: GP4 0x00530f3f
void __cdecl FUN_00530f3f(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t record = 0x00827160u;
    uint32_t count = GP4_GLOBAL(uint32_t, 0x007a97a4) + 1u;
    uint32_t slot = 0x007ad880u;
    uint32_t arith = 0u;
    do {
        const uint8_t* const rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(record));
        if (rec[5] != 2u) {
            if (rec[4] == 0u) {
                *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(slot)) = 0u;
            } else {
                gp4::Regs c = {};
                c.ebx = record;
                gp4::call_regs(0x00530f75, c);
                eax = c.eax;
                *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(slot)) = eax;
            }
        }
        slot += 4u;
        arith = gp4::flags::add32(record, 0x3cu);
        record += 0x3cu;
        arith = gp4::flags::dec32(count, arith);
        --count;
    } while (count != 0u);
    r->eax = eax;
    r->ebx = record;
    r->ecx = 0u;
    r->edi = slot;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00530f3f, FUN_00530f3f, "regs(eax:u32) -> (eax, ebx, ecx, edi, cf, pf, af, zf, sf, of); globals=layout=count_007a97a4+keys_00530f3f+rec_00530ae7+flag_00827164")

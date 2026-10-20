#include <gp4/gp4.h>

struct Unknown_0043faf5_Record {
    uint8_t Unknown00[0x73];
    uint8_t Flags73;
    uint8_t Unknown74[0x8c - 0x74];
    int8_t Mark8C;
};

// Walks the 0x160-byte records from edi towards esi (ebx keeps the first record for the exit
// helper 0x0043fac9). A record whose bit 6 of +0x73 is set or whose signed byte +0x8c is negative
// ends the walk; otherwise FUN_0043fab0 gives a signed delta (eax). The walk continues only while
// |delta| >= [0x00627ac4], delta has the sign of ebp (xor not negative) and |delta| >= |ebp| / 2;
// each accepted record gets +0x8c = 0x40 and bit 1 of +0x73 copied from [0x00627b65]. Reaching esi
// returns CF = 1, every other exit CF = 0; the other flags come from FUN_0043fac9.
// FUNCTION: GP4 0x0043faf5
void __cdecl FUN_0043faf5(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t edi = r->edi;
    const uint32_t first = r->edi;
    const uint32_t esi = r->esi;
    const uint32_t ebp = r->ebp;
    bool reached = false;

    for (;;) {
        Unknown_0043faf5_Record* record = reinterpret_cast<Unknown_0043faf5_Record*>(static_cast<uintptr_t>(edi));
        if ((record->Flags73 & 0x40u) != 0u) {
            break;
        }
        if (record->Mark8C < 0) {
            break;
        }

        gp4::Regs delta = *r;
        delta.edi = edi;
        gp4::call_regs(0x0043fab0, delta);
        eax = delta.eax;

        edx = eax;
        if (static_cast<int32_t>(eax) < 0) {
            eax = 0u - eax;
        }
        if (static_cast<int32_t>(eax) < GP4_GLOBAL(int32_t, 0x00627ac4)) {
            break;
        }

        eax = edx ^ ebp;
        if (static_cast<int32_t>(eax) < 0) {
            break;
        }

        eax = ebp;
        if (static_cast<int32_t>(eax) < 0) {
            eax = 0u - eax;
        }
        eax = static_cast<uint32_t>(static_cast<int32_t>(eax) >> 1);
        if (static_cast<int32_t>(edx) < 0) {
            edx = 0u - edx;
        }
        if (static_cast<int32_t>(edx) < static_cast<int32_t>(eax)) {
            break;
        }

        record->Mark8C = 0x40;
        if (GP4_GLOBAL(uint8_t, 0x00627b65) != 0u) {
            record->Flags73 = static_cast<uint8_t>(record->Flags73 | 2u);
        } else {
            record->Flags73 = static_cast<uint8_t>(record->Flags73 & 0xfdu);
        }
        edi += 0x160u;
        if (edi == esi) {
            reached = true;
            break;
        }
    }

    gp4::Regs finish = *r;
    finish.ebx = first;
    finish.edi = edi;
    gp4::call_regs(0x0043fac9, finish);

    r->eax = eax;
    r->edx = edx;
    r->edi = edi;
    r->eflags = (r->eflags & ~0x8d5u) | ((finish.eflags & 0x8d4u) | (reached ? 1u : 0u));
}
GP4_IMPL(0x0043faf5, FUN_0043faf5, "regs(eax:u32, edx:u32, ebp:i32[-1024..1024], edi:rec[0x8943a0,0x160,0..10], esi:rec[0x8943a0,0x160,11..25]) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_0043faf5")

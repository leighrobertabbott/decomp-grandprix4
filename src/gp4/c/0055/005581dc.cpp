#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Stream record decoder (R3, mode byte [0x86857c] == 2). A record at esi is 6 words, plus a 7th when
// bit 0x200 of the word [0x86855c] is set. When the mode is not 2 it only skips the record
// (esi += 0xe / 0xc; the flags are those of that add). Otherwise it bumps the slot counter [0x874d28]
// (new index must be below 0x10, else int3), stores words 0/1 into the word tables 0x874bd4/0x874bb4,
// turns words 2..5 into masks with 0x005581ba (OR of the 0x875dc8 dwords selected by the bits) and
// stores m2, m3 & ~m2... into the dword tables 0x874c14/0x874c54/0x874c94/0x874cd4 and, with the
// 0x200 bit, word 6 into 0x874bf4 + index * 4. ecx is saved/restored; edx = index, ebp = the first
// mask, eax = the last mask (low word replaced by word 6 with the 0x200 bit).
// FUNCTION: GP4 0x005581dc
void __cdecl FUN_005581dc(gp4::Regs* r) {
    if ((GP4_GLOBAL(uint16_t, 0x0086855c) & 0x200u) != 0u) {
        if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2u) {
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(r->esi, 0xeu));
            r->esi += 0xeu;
            return;
        }
    } else if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(r->esi, 0xcu));
        r->esi += 0xcu;
        return;
    }

    uint32_t index = GP4_GLOBAL(uint32_t, 0x00874d28) + 1u;
    GP4_GLOBAL(uint32_t, 0x00874d28) = index;
    if (index >= 0x10u) {
        __debugbreak();
    }
    const uint16_t* words = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->esi));
    GP4_ARRAY(uint16_t, 0x00874bd4)[index] = words[0];
    GP4_ARRAY(uint16_t, 0x00874bb4)[index] = words[1];

    gp4::Regs c = *r;
    c.edx = index;
    c.eax = (c.eax & 0xffff0000u) | words[2];
    gp4::call_regs(0x005581ba, c);
    const uint32_t first = c.eax;
    GP4_ARRAY(uint32_t, 0x00874c14)[index] = first;

    c.eax = (first & 0xffff0000u) | words[3];
    gp4::call_regs(0x005581ba, c);
    const uint32_t second = ~c.eax & first;
    GP4_ARRAY(uint32_t, 0x00874c54)[index] = second;

    c.eax = (second & 0xffff0000u) | words[4];
    gp4::call_regs(0x005581ba, c);
    const uint32_t third = ~c.eax & second;
    GP4_ARRAY(uint32_t, 0x00874c94)[index] = third;

    c.eax = (third & 0xffff0000u) | words[5];
    gp4::call_regs(0x005581ba, c);
    uint32_t eax = ~c.eax & first;
    GP4_ARRAY(uint32_t, 0x00874cd4)[index] = eax;

    uint32_t esi = r->esi + 0xcu;
    uint32_t arith;
    if ((GP4_GLOBAL(uint16_t, 0x0086855c) & 0x200u) != 0u) {
        eax = (eax & 0xffff0000u) | words[6];
        arith = gp4::flags::add32(esi, 2u);
        esi += 2u;
        *reinterpret_cast<uint16_t*>(GP4_ARRAY(uint8_t, 0x00874bf4) + index * 4u) = words[6];
    } else {
        arith = gp4::flags::logic16(0u);
    }
    r->eax = eax;
    r->edx = index;
    r->ebp = first;
    r->esi = esi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005581dc, FUN_005581dc, "regs(esi:ptr[0x18]) -> (eax, edx, ebp, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slot_005581dc")

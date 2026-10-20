#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks the key in al up in the 12 four-byte entries at 0x0082829b (+0 key, +1 value, +3 shift count).
// Hit: [0x82816c] = entry[1], eax = 1 << (entry[3] & 31), ecx = entry[3] (the loop counter is below 0x100),
// esi = the entry, CF clear (`clc`; the other flags are those of the `shl`, or of the equal `cmp al,[esi]`
// when the shift count is a multiple of 32). Miss: esi = entry table end, ecx = 0, eax unchanged, CF set; the
// other flags are those of the last `add esi, 4`.
// FUNCTION: GP4 0x005341a9
void __cdecl FUN_005341a9(gp4::Regs* r) {
    const uint8_t key = static_cast<uint8_t>(r->eax);
    const uint8_t* entry = GP4_ARRAY(const uint8_t, 0x0082829b);

    uint32_t arith = 0;
    for (uint32_t left = 12; left != 0u; --left) {
        if (key == entry[0]) {
            const uint32_t shift = entry[3] & 31u;
            const uint32_t result = 1u << shift;
            GP4_GLOBAL(uint32_t, 0x0082816c) = entry[1];
            if (shift == 0u) {
                arith = gp4::flags::sub8(key, key);
            } else {
                arith = gp4::flags::logic32(result) | (shift == 31u ? gp4::flags::OF : 0u);
            }
            r->eax = result;
            r->ecx = entry[3];
            r->esi = reinterpret_cast<uint32_t>(entry);
            r->eflags = gp4::flags::merge(r->eflags, arith & ~gp4::flags::CF);
            return;
        }
        arith = gp4::flags::add32(reinterpret_cast<uint32_t>(entry), 4u);
        entry += 4;
    }

    r->ecx = 0u;
    r->esi = reinterpret_cast<uint32_t>(entry);
    r->eflags = gp4::flags::merge(r->eflags, arith | gp4::flags::CF);
}
GP4_IMPL(0x005341a9, FUN_005341a9, "regs(eax:u32{1,2,3,4,5,6,0x10,0x20,0x80,0xff,0x7f}) -> (eax, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tab_0082829b")

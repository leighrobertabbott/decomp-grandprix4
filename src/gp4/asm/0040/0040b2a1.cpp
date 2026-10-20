#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint8_t flipBit6IfSigned(uint8_t value) {
    return (value & 0x80u) != 0u ? static_cast<uint8_t>(value ^ 0x40u) : value;
}

// FUNCTION: GP4 0x0040b2a1
void __cdecl FUN_0040b2a1(gp4::Regs* r) {
    // Inverse of 0x0040b264: reload the three global blocks from the caller's
    // buffer (esi), keeping a set of fields at their current values.
    const uint32_t keep679d30 = GP4_GLOBAL(uint32_t, 0x00679d30);
    const uint32_t keep66d81c = GP4_GLOBAL(uint32_t, 0x0066d81c);
    const uint16_t keep66d8b4 = GP4_GLOBAL(uint16_t, 0x0066d8b4);
    const uint8_t keep66d89e = GP4_GLOBAL(uint8_t, 0x0066d89e);
    const uint32_t keep66d83c = GP4_GLOBAL(uint32_t, 0x0066d83c);
    const uint32_t keep66d854 = GP4_GLOBAL(uint32_t, 0x0066d854);
    const uint32_t keep66d848 = GP4_GLOBAL(uint32_t, 0x0066d848);
    const uint16_t keep66d894 = GP4_GLOBAL(uint16_t, 0x0066d894);
    const uint8_t keep66d896 = GP4_GLOBAL(uint8_t, 0x0066d896);
    const uint8_t keep66d897 = GP4_GLOBAL(uint8_t, 0x0066d897);
    const uint16_t keep66d898 = GP4_GLOBAL(uint16_t, 0x0066d898);
    const uint16_t keep66d89a = GP4_GLOBAL(uint16_t, 0x0066d89a);
    const uint16_t keep66d8ba = GP4_GLOBAL(uint16_t, 0x0066d8ba);
    const uint16_t keep7ad892 = GP4_GLOBAL(uint16_t, 0x007ad892);
    const uint16_t keep7ad890 = GP4_GLOBAL(uint16_t, 0x007ad890);
    const uint16_t keep7ad89c = GP4_GLOBAL(uint16_t, 0x007ad89c);
    const uint8_t keep7ad814 = GP4_GLOBAL(uint8_t, 0x007ad814);
    const uint16_t keep7c2eb8 = GP4_GLOBAL(uint16_t, 0x007c2eb8);

    const uint8_t* in = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    for (uint32_t i = 0u; i != 0x007adb4cu - 0x007ad890u; ++i) {
        GP4_ARRAY(uint8_t, 0x007ad890)[i] = *in++;
    }
    for (uint32_t i = 0u; i != 0x007c3ae4u - 0x007c28bcu; ++i) {
        GP4_ARRAY(uint8_t, 0x007c28bc)[i] = *in++;
    }
    for (uint32_t i = 0u; i != 0x0066fef0u - 0x0066d7a4u; ++i) {
        GP4_ARRAY(uint8_t, 0x0066d7a4)[i] = *in++;
    }

    // 22 records of 0x41c bytes: byte +0x120 survives the reload, and byte +0x7c
    // gets bit 6 flipped when bit 7 is set.
    uint8_t* record = &GP4_ARRAY(uint8_t, 0x0066fef0)[0];
    for (uint32_t k = 0u; k != 0x16u; ++k) {
        const uint8_t keep120 = record[0x120];
        for (uint32_t i = 0u; i != 0x41cu; ++i) {
            record[i] = in[i];
        }
        record[0x120] = keep120;
        record[0x7c] = flipBit6IfSigned(record[0x7c]);
        record += 0x41c;
        in += 0x41c;
    }
    for (uint32_t i = 0u; i != 0x006975bcu - 0x00675958u; ++i) {
        record[i] = in[i];
    }

    // swap the reloaded pairs
    {
        const uint32_t a = GP4_GLOBAL(uint32_t, 0x00679d34);
        const uint32_t b = GP4_GLOBAL(uint32_t, 0x00679d38);
        GP4_GLOBAL(uint32_t, 0x00679d38) = a;
        GP4_GLOBAL(uint32_t, 0x00679d34) = b;
    }
    {
        const uint32_t a = GP4_GLOBAL(uint32_t, 0x0066d820);
        const uint32_t b = GP4_GLOBAL(uint32_t, 0x0066d828);
        GP4_GLOBAL(uint32_t, 0x0066d828) = a;
        GP4_GLOBAL(uint32_t, 0x0066d820) = b;
    }
    {
        const uint8_t a = flipBit6IfSigned(GP4_GLOBAL(uint8_t, 0x0066d896));
        const uint8_t b = GP4_GLOBAL(uint8_t, 0x0066d87d);
        GP4_GLOBAL(uint8_t, 0x0066d87d) = a;
        GP4_GLOBAL(uint8_t, 0x0066d896) = flipBit6IfSigned(b);
    }
    {
        const uint16_t a = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x0066d8b4) & 0x7fu);
        const uint16_t b = GP4_GLOBAL(uint16_t, 0x0066d8b6);
        GP4_GLOBAL(uint16_t, 0x0066d8b6) = a;
        GP4_GLOBAL(uint16_t, 0x0066d8b4) = b;
    }

    // restore the kept fields (pop order of the original)
    GP4_GLOBAL(uint16_t, 0x007c2eb8) = keep7c2eb8;
    GP4_GLOBAL(uint8_t, 0x007ad814) = keep7ad814;
    GP4_GLOBAL(uint16_t, 0x007ad89c) = keep7ad89c;
    GP4_GLOBAL(uint16_t, 0x007ad890) = keep7ad890;
    GP4_GLOBAL(uint16_t, 0x007ad892) = keep7ad892;
    GP4_GLOBAL(uint16_t, 0x0066d8ba) = keep66d8ba;
    GP4_GLOBAL(uint16_t, 0x0066d89a) = keep66d89a;
    GP4_GLOBAL(uint16_t, 0x0066d898) = keep66d898;
    GP4_GLOBAL(uint8_t, 0x0066d896) = keep66d896;
    GP4_GLOBAL(uint8_t, 0x0066d897) = keep66d897;
    GP4_GLOBAL(uint16_t, 0x0066d894) = keep66d894;
    GP4_GLOBAL(uint32_t, 0x0066d848) = keep66d848;
    GP4_GLOBAL(uint32_t, 0x0066d854) = keep66d854;
    GP4_GLOBAL(uint32_t, 0x0066d83c) = keep66d83c;
    GP4_GLOBAL(uint8_t, 0x0066d89e) = keep66d89e;
    GP4_GLOBAL(uint16_t, 0x0066d8b4) = static_cast<uint16_t>(
        GP4_GLOBAL(uint16_t, 0x0066d8b4) | (keep66d8b4 & 0xfcffu));
    GP4_GLOBAL(uint32_t, 0x0066d81c) = keep66d81c;
    GP4_GLOBAL(uint32_t, 0x00679d30) = keep679d30;

    // five 22-byte tables: flip bit 6 of every byte with bit 7 set
    static const uint32_t tables[5] = { 0x0066d8f4u, 0x00675958u, 0x007c31dcu, 0x007c31f2u, 0x007c2a48u };
    for (uint32_t t = 0u; t != 5u; ++t) {
        gp4::Regs regs = *r;
        regs.esi = tables[t];
        regs.ecx = 0x16u;
        gp4::call_regs(0x0040b503, regs);
    }

    GP4_GLOBAL(uint8_t, 0x007c2ea7) ^= 1u;

    // bit 6 -> bit 7, bit 7 -> bit 6
    const uint8_t value = GP4_GLOBAL(uint8_t, 0x0066d87c);
    const uint8_t high = static_cast<uint8_t>((value << 1) & 0x80u);
    const uint8_t low = static_cast<uint8_t>((value >> 1) & 0x40u);
    const uint8_t result = static_cast<uint8_t>(high | low);
    GP4_GLOBAL(uint8_t, 0x0066d87c) = result;

    // last flag writer: or al, ah (popal leaves the flags)
    const uint32_t flags = parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040b2a1, FUN_0040b2a1, "regs(esi:ptr[176900]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32; wall=1200")

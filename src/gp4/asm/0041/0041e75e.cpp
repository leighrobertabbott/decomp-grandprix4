#include <gp4/gp4.h>

// Word of the Q14 sine/cosine table at 0x007c5c5c for the 16-bit angle `angle` (shr 2, even byte offset).
static inline int32_t tableWord(uint32_t angle) {
    uint8_t* table = GP4_ARRAY(uint8_t, 0x007c5c5c);
    const uint32_t offset = ((angle & 0xffffu) >> 2) & 0xfffeu;
    return static_cast<int32_t>(GP4_FIELD(int16_t, table, offset));
}

// FUNCTION: GP4 0x0041e75e
// With a = word [edi] (an angle, 0x4000 = quarter turn) and the table word C(x) = Q14 value of |x|:
//   ebp = (C(0x4000 - a) * [esi+0xa4] >> 18) + (C(|a|) * [esi+0xa8] >> 18)
// then for each of the four 0x300-byte records from 0x006380d8: +0x15c = [+0x168] * ebp >> 18 and
// +0x160 = [+0x16c] * ebp >> 18. Registers left behind: eax/edx = the last product (low/high dword),
// ebx = 0x00638cd8, ebp, flags of the final cmp ebx, 0x638cd8 (equal).
void __cdecl FUN_0041e75e(gp4::Regs* r) {
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* anglePtr = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint16_t angle = GP4_FIELD(const uint16_t, anglePtr, 0);

    // neg ax / add ax, 0x4000 / test / neg: |0x4000 - angle| in 16 bits
    uint16_t cosAngle = static_cast<uint16_t>(0x4000u - angle);
    if (static_cast<int16_t>(cosAngle) < 0) {
        cosAngle = static_cast<uint16_t>(0u - cosAngle);
    }
    const int64_t cosProduct = static_cast<int64_t>(tableWord(cosAngle)) * static_cast<int64_t>(GP4_FIELD(const int32_t, source, 0xa4));
    uint32_t ebp = static_cast<uint32_t>(static_cast<uint64_t>(cosProduct) >> 18);

    uint16_t sinAngle = angle;
    if (static_cast<int16_t>(sinAngle) < 0) {
        sinAngle = static_cast<uint16_t>(0u - sinAngle);
    }
    const int64_t sinProduct = static_cast<int64_t>(tableWord(sinAngle)) * static_cast<int64_t>(GP4_FIELD(const int32_t, source, 0xa8));
    ebp += static_cast<uint32_t>(static_cast<uint64_t>(sinProduct) >> 18);

    uint8_t* record = GP4_ARRAY(uint8_t, 0x006380d8);
    uint8_t* const end = GP4_ARRAY(uint8_t, 0x00638cd8);
    int64_t product = 0;
    uint32_t eax = r->eax;
    do {
        product = static_cast<int64_t>(GP4_FIELD(const int32_t, record, 0x168)) * static_cast<int64_t>(static_cast<int32_t>(ebp));
        eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 18);
        GP4_FIELD(uint32_t, record, 0x15c) = eax;
        product = static_cast<int64_t>(GP4_FIELD(const int32_t, record, 0x16c)) * static_cast<int64_t>(static_cast<int32_t>(ebp));
        eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 18);
        GP4_FIELD(uint32_t, record, 0x160) = eax;
        record += 0x300;
    } while (record != end);

    r->eax = eax;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end));
    r->ebp = ebp;
    // cmp ebx, 0x638cd8 with equal operands: ZF = PF = 1, the rest clear.
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0041e75e, FUN_0041e75e, "regs(esi:ptr[0xb0]:bytes, edi:ptr[0x10]:bytes) -> (eax, ebx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=recs_006380d8")

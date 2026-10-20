#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of a 32-bit `sub left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// ZF|PF: `cmp x, x` / `or eax, eax` of zero (every other arithmetic flag clear).
static const uint32_t kZeroResultFlags = 0x44u;

static inline void leave(gp4::Regs* r, uint32_t eax, uint32_t ebx, uint32_t edi, uint32_t flags) {
    r->eax = eax;
    r->ebx = ebx;
    r->edi = edi;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}

// Follows the chain of car records ebx -> word [ebx+0x23e] (offset into the car array at 0x66fef0)
// looking for the first one whose time difference to esi (word [ebx+0xe2] - [esi+0xe2], or the
// result of 0x0040dc8a when either record has bit 1 of [+0x9c] set) reaches [0x00603b28]. Gives up
// when a record is already in the list at 0x00604294 (edi entries), when the word at +0x68 exceeds
// bp, after 8 steps, at a zero link or a link back to esi. The match is appended to the list
// (edi + 1), recorded as the offset word [esi+0x23e] and leaves eax = that offset. The record
// [0x00604280] is skipped over by following its own link. ecx is preserved; the flags are those
// of the instruction that decided the exit.
// FUNCTION: GP4 0x0042dc79
void __cdecl FUN_0042dc79(gp4::Regs* r) {
    uint8_t* subject = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t bodyAddress = r->ebx;
    uint32_t eax = r->eax;
    uint32_t count = r->edi;
    const int16_t limit = static_cast<int16_t>(r->ebp);
    uint32_t* const list = GP4_ARRAY(uint32_t, 0x00604294);

    if (bodyAddress == 0u) {
        leave(r, eax, bodyAddress, count, kZeroResultFlags);
        return;
    }
    if (bodyAddress == GP4_GLOBAL(uint32_t, 0x00604280)) {
        leave(r, eax, bodyAddress, count, kZeroResultFlags);
        return;
    }

    uint32_t steps = 0u;
    for (;;) {
        uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(bodyAddress));
        const int16_t bodyLimit = GP4_FIELD(int16_t, body, 0x68);
        if (limit < bodyLimit) {
            leave(r, eax, bodyAddress, count, subFlags16(static_cast<uint16_t>(limit), static_cast<uint16_t>(bodyLimit)));
            return;
        }

        uint32_t scan = count - 1u;
        while (static_cast<int32_t>(scan) >= 0) {
            if (bodyAddress == list[scan]) {
                leave(r, scan, bodyAddress, count, kZeroResultFlags);
                return;
            }
            scan -= 1u;
        }
        eax = scan;

        uint16_t difference;
        if ((GP4_FIELD(uint8_t, subject, 0x9c) & 2u) == 0u && (GP4_FIELD(uint8_t, body, 0x9c) & 2u) == 0u) {
            difference = static_cast<uint16_t>(GP4_FIELD(uint16_t, body, 0xe2) - GP4_FIELD(uint16_t, subject, 0xe2));
        } else {
            gp4::Regs call = *r;
            call.eax = eax;
            call.ebx = bodyAddress;
            call.ecx = steps;
            call.st_in = 0;
            call.st_out = 0;
            gp4::call_regs(0x0040dc8a, call);
            difference = static_cast<uint16_t>(call.eax);
        }
        eax = (eax & 0xffff0000u) | difference;

        if (static_cast<int16_t>(difference) >= GP4_GLOBAL(int16_t, 0x00603b28)) {
            list[count] = bodyAddress;
            count += 1u;
            const uint32_t offset = bodyAddress - 0x0066fef0u;
            GP4_FIELD(uint16_t, subject, 0x23e) = static_cast<uint16_t>(offset);
            leave(r, offset, bodyAddress, count, subFlags32(bodyAddress, 0x0066fef0u));
            return;
        }

        steps += 1u;
        if (static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00603b84)) < static_cast<int32_t>(steps)) {
            GP4_GLOBAL(uint32_t, 0x00603b84) = steps;
        }
        if (steps == 8u) {
            leave(r, eax, bodyAddress, count, kZeroResultFlags);
            return;
        }

        uint32_t link = GP4_FIELD(uint16_t, body, 0x23e);
        if (link == 0u) {
            leave(r, 0u, bodyAddress, count, kZeroResultFlags);
            return;
        }
        eax = link + 0x0066fef0u;
        if (r->esi == eax) {
            leave(r, eax, bodyAddress, count, kZeroResultFlags);
            return;
        }
        bodyAddress = eax;

        if (bodyAddress == GP4_GLOBAL(uint32_t, 0x00604280)) {
            body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(bodyAddress));
            link = GP4_FIELD(uint16_t, body, 0x23e);
            if (link == 0u) {
                leave(r, 0u, bodyAddress, count, kZeroResultFlags);
                return;
            }
            eax = link + 0x0066fef0u;
            if (r->esi == eax) {
                leave(r, eax, bodyAddress, count, kZeroResultFlags);
                return;
            }
            bodyAddress = eax;
        }
    }
}
GP4_IMPL(0x0042dc79, FUN_0042dc79, "regs(eax:u32, ebx:rec[0x66fef0,0x41c,0..21], ebp:u32, esi:rec[0x66fef0,0x41c,0..21], edi:u32[0..7]) -> (eax, ebx, edi, cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0+chain_0042dc79")

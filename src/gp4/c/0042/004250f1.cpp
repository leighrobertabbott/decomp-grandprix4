#include <gp4/gp4.h>

static inline uint8_t* bytesAt(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

// FUNCTION: GP4 0x004250f1
// For each of the [0x00609864] view records (0x64 bytes at 0x00621aa0): points the record at its
// two orientation bodies (0x00622cf8 + (12 + i) * 0xd0 and + (7 + i) * 0xd0) and at its source
// record (0x0063db8c + [0x00609870 + i * 4] * 0x34), runs the view transform 0x004020ad and the
// position copy 0x004251e7, then copies the source's dwords +0x24 / +0x2c / +0x28 to +0x3c / +0x48 /
// +0x54 and derives the ratios +0x58 = (+0x54 << 14) / (+0x54 + [0x00608254]) and +0x5c =
// low32(+0x54 * [0x00608254]) * ((([0x00609868] + 0x4000) << 5)) / (low32([0x00608164] * (+0x54 +
// [0x00608254])) or 1). The registers the last iteration leaves carry out; the flags are the equal
// `cmp ecx, [0x00609864]` (or the test of a zero count): ZF and PF.
void __cdecl FUN_004250f1(gp4::Regs* r) {
    if (GP4_GLOBAL(uint32_t, 0x00609864) != 0u) {
        gp4::Regs state = *r;
        state.st_in = 0;
        state.st_out = 0;

        uint32_t index = 0;
        do {
            const uint32_t record = index * 0x64u + 0x00621aa0u;
            uint8_t* view = bytesAt(record);
            GP4_GLOBAL(uint32_t, 0x00609860) = record;
            GP4_FIELD(uint32_t, view, 0x40) = (index + 12u) * 0xd0u + 0x00622cf8u;
            GP4_FIELD(uint32_t, view, 0x44) = (index + 7u) * 0xd0u + 0x00622cf8u;
            const uint32_t source = GP4_ARRAY(uint32_t, 0x00609870)[index] * 0x34u + 0x0063db8cu;
            GP4_GLOBAL(uint32_t, 0x0060982c) = source;
            GP4_FIELD(uint32_t, view, 0x60) = source;

            state.eax = source;
            state.ecx = index;
            state.edi = record;
            gp4::call_regs(0x004020ad, state);
            gp4::call_regs(0x004251e7, state);

            uint8_t* dest = bytesAt(GP4_GLOBAL(uint32_t, 0x00609860));
            uint8_t* from = bytesAt(GP4_GLOBAL(uint32_t, 0x0060982c));
            GP4_FIELD(uint32_t, dest, 0x3c) = GP4_FIELD(uint32_t, from, 0x24);
            GP4_FIELD(uint32_t, dest, 0x48) = GP4_FIELD(uint32_t, from, 0x2c);
            GP4_FIELD(uint32_t, dest, 0x54) = GP4_FIELD(uint32_t, from, 0x28);

            const int32_t value = GP4_FIELD(int32_t, dest, 0x54);
            int32_t remainder = 0;
            uint32_t divisor = static_cast<uint32_t>(value) + GP4_GLOBAL(uint32_t, 0x00608254);
            GP4_FIELD(int32_t, dest, 0x58) =
                gp4::idiv64(static_cast<int64_t>(value) * 0x4000, static_cast<int32_t>(divisor), &remainder);

            divisor = GP4_FIELD(uint32_t, dest, 0x54) + GP4_GLOBAL(uint32_t, 0x00608254);
            const int64_t scaled = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00608164))
                * static_cast<int64_t>(static_cast<int32_t>(divisor));
            divisor = static_cast<uint32_t>(scaled);
            if (divisor == 0u) {
                divisor = 1u;
            }

            const int64_t product = static_cast<int64_t>(GP4_FIELD(int32_t, dest, 0x54))
                * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00608254));
            const int32_t multiplier = static_cast<int32_t>((GP4_GLOBAL(uint32_t, 0x00609868) + 0x4000u) << 5);
            const int64_t numerator = static_cast<int64_t>(static_cast<int32_t>(product))
                * static_cast<int64_t>(multiplier);
            const int32_t quotient = gp4::idiv64(numerator, static_cast<int32_t>(divisor), &remainder);
            GP4_FIELD(int32_t, dest, 0x5c) = quotient;

            state.eax = static_cast<uint32_t>(quotient);
            state.ebx = GP4_GLOBAL(uint32_t, 0x00609860);
            state.edx = static_cast<uint32_t>(remainder);
            state.ebp = divisor;
            state.edi = GP4_GLOBAL(uint32_t, 0x0060982c);
            ++index;
        } while (index != GP4_GLOBAL(uint32_t, 0x00609864));

        r->eax = state.eax;
        r->ebx = state.ebx;
        r->ecx = index;
        r->edx = state.edx;
        r->ebp = state.ebp;
        r->edi = state.edi;
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004250f1, FUN_004250f1, "regs() -> (eax, ebx, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=layout=view_00609860+recs_00621aa0+state_004250f1")

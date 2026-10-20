#include <gp4/gp4.h>

static inline uint32_t parity8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of `neg` (0 - value).
static inline uint32_t negFlags(uint32_t value) {
    const uint32_t result = 0u - value;
    return (value != 0u ? 0x01u : 0u)
        | parity8(result)
        | ((value ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

// esi = car state, edi = the angle word passed on to 0x0041e75e. The four dwords +0x2ec..+0x2f8 are
// latched into 0x00622920..0x0062292c (ring index 0x0062291c and sums 0x00622930..0x0062293c cleared)
// and 0x00420880 runs. Unless byte +0xe4 is 3 with both quotients 0x0062244c / 0x00622450 zero, the
// dwords +0x2c4 / +0x8c4 of the 0x300-byte records at 0x006380d8 nonzero and every +0x2ec+4k zero with
// +0x32c+4k negative, the per-record dwords +0x164 are cleared, 0x00601d38 = OR of the records' +0x6c
// (nonzero: their +0xb4 cleared), byte 0x00601ea8 = 0, and either (byte 0x00622424 set) +0x2ec..+0x2f8
// become record +0x94 minus half of 0x00637fb4 (records 0, 1) or 0x00637fc0 (records 2, 3), or
// 0x0041f489 (bit 7 of +0x21c and byte 0x006228c1), 0x0041e75e and 0x004208bd run.
// The four 0x300-byte records at 0x006380d8, addressed through a base register as the original does.
static __declspec(noinline) uint8_t* cornerRecords() {
    return GP4_ARRAY(uint8_t, 0x006380d8);
}

// FUNCTION: GP4 0x00420a21
void __cdecl FUN_00420a21(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* records = cornerRecords();

    GP4_GLOBAL(uint32_t, 0x0062291c) = 0u;
    GP4_GLOBAL(uint32_t, 0x00622920) = GP4_FIELD(uint32_t, car, 0x2ec);
    GP4_GLOBAL(uint32_t, 0x00622924) = GP4_FIELD(uint32_t, car, 0x2f0);
    GP4_GLOBAL(uint32_t, 0x00622928) = GP4_FIELD(uint32_t, car, 0x2f4);
    GP4_GLOBAL(uint32_t, 0x0062292c) = GP4_FIELD(uint32_t, car, 0x2f8);
    GP4_GLOBAL(uint32_t, 0x00622930) = 0u;
    GP4_GLOBAL(uint32_t, 0x00622934) = 0u;
    GP4_GLOBAL(uint32_t, 0x00622938) = 0u;
    GP4_GLOBAL(uint32_t, 0x0062293c) = 0u;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = GP4_FIELD(uint32_t, car, 0x2f8);
    gp4::call_regs(0x00420880, c);

    if (GP4_FIELD(uint8_t, car, 0xe4) == 3u && GP4_GLOBAL(uint32_t, 0x0062244c) == 0u
        && GP4_GLOBAL(uint32_t, 0x00622450) == 0u) {
        c.ebx = 0x006380d8u;
        if (GP4_FIELD(uint32_t, records, 0x2c4) != 0u && GP4_FIELD(uint32_t, records, 0x8c4) != 0u) {
            uint32_t k = 0u;
            for (;;) {
                c.ecx = k;
                if (GP4_FIELD(uint32_t, car, 0x2ec + k * 4u) != 0u
                    || GP4_FIELD(int32_t, car, 0x32c + k * 4u) >= 0) {
                    break;
                }
                ++k;
                if (k == 4u) {
                    // every corner idle: nothing to reset
                    r->eax = c.eax;
                    r->edx = c.edx;
                    r->ebx = c.ebx;
                    r->ecx = k;
                    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;  // cmp ecx, 4 (equal)
                    return;
                }
            }
        }
    }

    GP4_GLOBAL(uint32_t, 0x006258f0) = 0u;
    GP4_FIELD(uint32_t, records, 0x164) = 0u;
    GP4_FIELD(uint32_t, records, 0x464) = 0u;
    GP4_FIELD(uint32_t, records, 0x764) = 0u;
    GP4_FIELD(uint32_t, records, 0xa64) = 0u;
    const uint32_t active = GP4_FIELD(uint32_t, records, 0x6c) | GP4_FIELD(uint32_t, records, 0x36c)
        | GP4_FIELD(uint32_t, records, 0x66c) | GP4_FIELD(uint32_t, records, 0x96c);
    GP4_GLOBAL(uint32_t, 0x00601d38) = active;
    c.eax = active;
    if (active != 0u) {
        c.eax = 0u;
        GP4_FIELD(uint32_t, records, 0xb4) = 0u;
        GP4_FIELD(uint32_t, records, 0x3b4) = 0u;
        GP4_FIELD(uint32_t, records, 0x6b4) = 0u;
        GP4_FIELD(uint32_t, records, 0x9b4) = 0u;
    }
    c.ebx = 0x006380d8u;
    GP4_GLOBAL(uint8_t, 0x00601ea8) = 0u;

    if (GP4_GLOBAL(uint8_t, 0x00622424) != 0u) {
        const uint32_t front = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00637fb4) >> 1);
        GP4_FIELD(uint32_t, car, 0x2ec) = 0u - (front - GP4_FIELD(uint32_t, records, 0x94));
        GP4_FIELD(uint32_t, car, 0x2f0) = 0u - (front - GP4_FIELD(uint32_t, records, 0x394));
        const uint32_t rear = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00637fc0) >> 1);
        GP4_FIELD(uint32_t, car, 0x2f4) = 0u - (rear - GP4_FIELD(uint32_t, records, 0x694));
        const uint32_t last = rear - GP4_FIELD(uint32_t, records, 0x994);
        GP4_FIELD(uint32_t, car, 0x2f8) = 0u - last;
        r->eax = 0u - last;
        r->edx = rear;
        r->ebx = c.ebx;
        r->ecx = c.ecx;
        r->eflags = (r->eflags & ~0x8d5u) | negFlags(last);
        return;
    }

    c.eax = GP4_GLOBAL(uint32_t, 0x006228ac);
    GP4_GLOBAL(uint32_t, 0x006228ac) = c.eax;
    if ((GP4_FIELD(uint8_t, car, 0x21c) & 0x80u) != 0u && GP4_GLOBAL(uint8_t, 0x006228c1) != 0u) {
        gp4::call_regs(0x0041f489, c);
    }
    gp4::call_regs(0x0041e75e, c);
    gp4::call_regs(0x004208bd, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00420a21, FUN_00420a21, "regs(esi:ptr[0x3a8]:layout=car_00420a21, edi:ptr[0x10]:bytes, ecx:u32, ebp:u32) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=recs_006380d8+state_0041f489+integ_004200a7+state_004208bd+state_004094e3+idle_00420a21")

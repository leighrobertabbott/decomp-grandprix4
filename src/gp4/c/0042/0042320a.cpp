#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t ParityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (~parity & 1u) << 2;
}

// Flags of the 32-bit `cmp lhs, rhs`.
static inline uint32_t SubtractFlags(uint32_t lhs, uint32_t rhs) {
    const uint32_t result = lhs - rhs;
    return (lhs < rhs ? 1u : 0u) | ParityFlag(result)
        | ((lhs ^ rhs ^ result) & 0x10u) | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x0042320a
// Walks ecx = 0 .. [0x006097cc]-1 over the 0x14-byte points behind [0x006097ac] (skipping those with
// a nonzero dword +0xc while ecx < [0x006082b4]) and, from ecx == [0x006082b4], the fixed points at
// 0x00620fc0, with edi the matching 0x20-byte slot at 0x006211a0. Each point's depth along the matrix
// column 0x00622e98 + (0x60, 0x78, 0x90) (80-bit sum, fistp to 0x006f2bf4 and slot +4) plus
// [0x00607f2c] - [0x00607fbc]: a negative value sets bit ecx of 0x00608278, counts in 0x00608274 and
// gives the penetration force min((-v * [0x00608010]) >> 18, [0x0060808c]) (0x00623e28, summed in
// 0x00608184); point +0x10 goes to slot +0x18, 0x00422db7 fills the slot, the damping
// ((-(slot +0x10) >> 5) * [0x006080bc] >> 14) * [0x00608080] >> 18 is added to 0x00608190 and
// 0x00623e28, max(0x00623e28, 0) goes to slot +0x1c, 0x00423048 splits it, the three forces
// (0x00623e24/28/2c) are summed and rotated by 0x00401e2e into 0x00623e18/1c/20, then 0x00422f77 and
// 0x00421f53 accumulate them. Afterwards [0x00608104] counts up and, once it reaches [0x00608110],
// 0x00421fc3 and 0x00421ef8 run.
void __cdecl FUN_0042320a(gp4::Regs* r) {
    using namespace gp4::x87;

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    uint8_t* point = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x006097ac)));
    uint8_t* slot = GP4_ARRAY(uint8_t, 0x006211a0);
    uint8_t* matrix = GP4_ARRAY(uint8_t, 0x00622e98);
    uint32_t i = 0;

    do {
        bool skip = false;
        const uint32_t split = GP4_GLOBAL(uint32_t, 0x006082b4);
        if (i < split) {
            if (GP4_FIELD(uint32_t, point, 0xc) != 0u) skip = true;
        } else if (i == split) {
            point = GP4_ARRAY(uint8_t, 0x00620fc0);
        }
        if (!skip) {
            s.edx = 0x00622e98u;
            Ext depth = (ext_i32(GP4_FIELD(int32_t, point, 0)) - ext_i32(GP4_GLOBAL(int32_t, 0x006097b4)))
                * ext(GP4_FIELD(double, matrix, 0x60));
            const Ext y = (ext_i32(GP4_FIELD(int32_t, point, 4)) - ext_i32(GP4_GLOBAL(int32_t, 0x006097b8)))
                * ext(GP4_FIELD(double, matrix, 0x78));
            depth = depth + y;
            const Ext z = (ext_i32(GP4_FIELD(int32_t, point, 8)) - ext_i32(GP4_GLOBAL(int32_t, 0x006097bc)))
                * ext(GP4_FIELD(double, matrix, 0x90));
            depth = depth + z;
            GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(depth);
            const uint32_t rounded = GP4_GLOBAL(uint32_t, 0x006f2bf4);
            GP4_FIELD(uint32_t, slot, 4) = rounded;
            const uint32_t balance = rounded + GP4_GLOBAL(uint32_t, 0x00607f2c) - GP4_GLOBAL(uint32_t, 0x00607fbc);
            s.eax = balance;
            if (static_cast<int32_t>(balance) < 0) {
                GP4_ARRAY(uint32_t, 0x00608278)[static_cast<int32_t>(i) >> 5] |= 1u << (i & 31u);
                GP4_GLOBAL(uint32_t, 0x00608274) += 1u;

                const int64_t pressure = static_cast<int64_t>(static_cast<int32_t>(0u - balance))
                    * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00608010));
                int32_t force = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(pressure) >> 18));
                s.edx = static_cast<uint32_t>(static_cast<uint64_t>(pressure) >> 32);
                if (force > GP4_GLOBAL(int32_t, 0x0060808c)) {
                    force = GP4_GLOBAL(int32_t, 0x0060808c);
                }
                GP4_GLOBAL(int32_t, 0x00623e28) = force;
                GP4_GLOBAL(uint32_t, 0x00608184) += static_cast<uint32_t>(force);

                const uint32_t scale = GP4_FIELD(uint32_t, point, 0x10);
                GP4_FIELD(uint32_t, slot, 0x18) = scale;
                s.eax = scale;
                s.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(point));
                s.ecx = i;
                s.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slot));
                gp4::call_regs(0x00422db7, s);

                const int32_t lift = static_cast<int32_t>(0u - GP4_FIELD(uint32_t, slot, 0x10)) >> 5;
                const int64_t first = static_cast<int64_t>(lift) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006080bc));
                const int32_t damped = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14));
                const int64_t second = static_cast<int64_t>(damped) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00608080));
                const uint32_t damping = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 18);
                s.edx = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);
                GP4_GLOBAL(uint32_t, 0x00608190) += damping;
                GP4_GLOBAL(uint32_t, 0x00623e28) += damping;
                int32_t limit = GP4_GLOBAL(int32_t, 0x00623e28);
                if (limit < 0) {
                    limit = 0;
                }
                GP4_FIELD(int32_t, slot, 0x1c) = limit;
                s.eax = static_cast<uint32_t>(limit);
                gp4::call_regs(0x00423048, s);

                GP4_GLOBAL(uint32_t, 0x0060818c) += GP4_GLOBAL(uint32_t, 0x00623e24);
                GP4_GLOBAL(uint32_t, 0x00608194) += GP4_GLOBAL(uint32_t, 0x00623e2c);
                s.eax = GP4_GLOBAL(uint32_t, 0x00623e2c);
                s.edx = 0x00622e98u;
                GP4_FIELD(double, matrix, 0xa0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e24));
                GP4_FIELD(double, matrix, 0xa8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e28));
                GP4_FIELD(double, matrix, 0xb0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e2c));
                gp4::call_regs(0x00401e2e, s);

                GP4_GLOBAL(int32_t, 0x00623e18) = fistp32(ext(GP4_FIELD(double, matrix, 0xb8)));
                GP4_GLOBAL(int32_t, 0x00623e1c) = fistp32(ext(GP4_FIELD(double, matrix, 0xc0)));
                GP4_GLOBAL(int32_t, 0x00623e20) = fistp32(ext(GP4_FIELD(double, matrix, 0xc8)));
                gp4::call_regs(0x00422f77, s);
                gp4::call_regs(0x00421f53, s);
            }
        }
        slot += 0x20;
        point += 0x14;
        ++i;
    } while (i != GP4_GLOBAL(uint32_t, 0x006097cc));

    GP4_GLOBAL(int32_t, 0x00608104) += 1;
    const int32_t frames = GP4_GLOBAL(int32_t, 0x00608104);
    const int32_t period = GP4_GLOBAL(int32_t, 0x00608110);
    s.eax = static_cast<uint32_t>(frames);
    s.eflags = (s.eflags & ~0x8d5u) | SubtractFlags(static_cast<uint32_t>(frames), static_cast<uint32_t>(period));
    s.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(point));
    s.ecx = i;
    s.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slot));
    if (frames >= period) {
        gp4::call_regs(0x00421fc3, s);
        gp4::cdecl_<0x00421ef8, void>();
    }

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->ebp = s.ebp;
    r->edi = s.edi;
    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x0042320a, FUN_0042320a, "regs(edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=view_00609860+state_00424011+state_00421fc3+state_0042320a")

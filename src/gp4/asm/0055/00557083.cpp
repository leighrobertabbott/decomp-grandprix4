#include <gp4/gp4.h>
#include <gp4/flags.h>

// One helper per callee so every call keeps its own address.
static __declspec(noinline) void Call5591f9(gp4::Regs* r) { gp4::call_regs(0x005591f9, *r); }
static __declspec(noinline) void Call559212(gp4::Regs* r) { gp4::call_regs(0x00559212, *r); }
static __declspec(noinline) void Call55922b(gp4::Regs* r) { gp4::call_regs(0x0055922b, *r); }
static __declspec(noinline) void Call559244(gp4::Regs* r) { gp4::call_regs(0x00559244, *r); }
static __declspec(noinline) void Call559253(gp4::Regs* r) { gp4::call_regs(0x00559253, *r); }
static __declspec(noinline) void Call559287(gp4::Regs* r) { gp4::call_regs(0x00559287, *r); }
static __declspec(noinline) void Call559310(gp4::Regs* r) { gp4::call_regs(0x00559310, *r); }

static inline void SetFlags(gp4::Regs* r, uint32_t arith) {
    r->eflags = gp4::flags::merge(r->eflags, arith);
}

static inline void SetAl(gp4::Regs* r, uint8_t al) {
    r->eax = (r->eax & 0xffffff00u) | al;
}

static inline void SetAx(gp4::Regs* r, uint16_t ax) {
    r->eax = (r->eax & 0xffff0000u) | ax;
}

static inline uint8_t* Record(const gp4::Regs* r) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
}

// Next record of the 0x160-byte segment ring: ebx += 0x160, wrapped back by the ring size
// [0x866ebc] when it reaches the ring end [0x866eb8] (unsigned).
static inline void NextRecord(gp4::Regs* r) {
    uint32_t p = r->ebx + 0x160u;
    const uint32_t end = GP4_GLOBAL(uint32_t, 0x00866eb8);
    uint32_t arith = gp4::flags::sub32(p, end);
    if (p >= end) {
        const uint32_t size = GP4_GLOBAL(uint32_t, 0x00866ebc);
        arith = gp4::flags::sub32(p, size);
        p -= size;
    }
    r->ebx = p;
    SetFlags(r, arith);
}

// Segment flag-bit pattern writer over the ring starting at record ebx, for the channel bit dl
// (dh = ~dl). Repeats [0x874b50] times (at least once): one record set by 0x559212 (0x874b54 reset
// to 0x874b52 first when 0x874b4f bit 7 is set) + nibble 0x559287 + pan 0x559310 (ax = 7), then
// [0x874b58] - 1 records through 0x55922b (or, on the last one when 0x874b4f bit 6 is set,
// 0x559212 with 0x874b54 -= 0x874b56) + 0x559287, one record through 0x5591f9 + 0x559287 +
// 0x559310 (ax = 6), and between repeats [0x874b59] - 1 records cleared by 0x559244. Then it clears
// (0x559244) records until one has bit dl in +0x3c, and clears that record through 0x559253 unless
// +0x2c has bit dl. Leaves ebx on the last record, dh = ~dl, eax from the last al/ax writer.
// FUNCTION: GP4 0x00557083
void __cdecl FUN_00557083(gp4::Regs* r) {
    const uint8_t dl = static_cast<uint8_t>(r->edx);
    r->edx = (r->edx & 0xffff00ffu) | (static_cast<uint32_t>(static_cast<uint8_t>(~dl)) << 8);

    uint8_t al = GP4_GLOBAL(uint8_t, 0x00874b58);
    GP4_GLOBAL(uint8_t, 0x00934de4) = al;
    al = GP4_GLOBAL(uint8_t, 0x00874b59);
    GP4_GLOBAL(uint8_t, 0x00934de8) = al;
    SetAl(r, al);
    GP4_GLOBAL(uint16_t, 0x00874b54) = GP4_GLOBAL(uint16_t, 0x00874b52);

    const uint8_t first = static_cast<uint8_t>(Record(r)[0x2c] & dl);
    SetFlags(r, gp4::flags::logic8(first));
    if (first != 0u) {
        Call5591f9(r);
    }

    for (;;) {
        NextRecord(r);
        const uint8_t reset = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x00874b4f) & 0x80u);
        SetFlags(r, gp4::flags::logic8(reset));
        if (reset != 0u) {
            GP4_GLOBAL(uint16_t, 0x00874b54) = GP4_GLOBAL(uint16_t, 0x00874b52);
        }
        Call559212(r);
        Call559287(r);
        SetAx(r, 7u);
        Call559310(r);

        for (;;) {
            NextRecord(r);
            const uint8_t before = GP4_GLOBAL(uint8_t, 0x00934de4);
            const uint8_t left = static_cast<uint8_t>(before - 1u);
            GP4_GLOBAL(uint8_t, 0x00934de4) = left;
            SetFlags(r, gp4::flags::sub8(before, 1u));
            if (left == 0u) {
                break;
            }
            SetFlags(r, gp4::flags::sub8(left, 1u));
            bool ramp = false;
            if (left == 1u) {
                const uint8_t bit = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x00874b4f) & 0x40u);
                SetFlags(r, gp4::flags::logic8(bit));
                ramp = bit != 0u;
            }
            if (ramp) {
                Call559212(r);
                Call559287(r);
                const uint16_t step = GP4_GLOBAL(uint16_t, 0x00874b56);
                SetAx(r, step);
                const uint16_t pos = GP4_GLOBAL(uint16_t, 0x00874b54);
                GP4_GLOBAL(uint16_t, 0x00874b54) = static_cast<uint16_t>(pos - step);
                SetFlags(r, gp4::flags::sub16(pos, step));
            } else {
                Call55922b(r);
                Call559287(r);
            }
        }

        al = GP4_GLOBAL(uint8_t, 0x00874b58);
        GP4_GLOBAL(uint8_t, 0x00934de4) = al;
        SetAl(r, al);
        Call5591f9(r);
        Call559287(r);
        SetAx(r, 6u);
        Call559310(r);

        const uint16_t repeats = GP4_GLOBAL(uint16_t, 0x00874b50);
        SetFlags(r, gp4::flags::sub16(repeats, 1u));
        if (static_cast<int16_t>(repeats) <= 1) {
            break;
        }

        for (;;) {
            const uint8_t before = GP4_GLOBAL(uint8_t, 0x00934de8);
            const uint8_t left = static_cast<uint8_t>(before - 1u);
            GP4_GLOBAL(uint8_t, 0x00934de8) = left;
            SetFlags(r, gp4::flags::sub8(before, 1u));
            if (left == 0u) {
                break;
            }
            NextRecord(r);
            Call559244(r);
        }

        al = GP4_GLOBAL(uint8_t, 0x00874b59);
        GP4_GLOBAL(uint8_t, 0x00934de8) = al;
        SetAl(r, al);
        const uint16_t count = GP4_GLOBAL(uint16_t, 0x00874b50);
        const uint16_t next = static_cast<uint16_t>(count - 1u);
        GP4_GLOBAL(uint16_t, 0x00874b50) = next;
        SetFlags(r, gp4::flags::sub16(count, 1u));
        if (next == 0u) {
            break;
        }
    }

    for (;;) {
        NextRecord(r);
        const uint8_t found = static_cast<uint8_t>(Record(r)[0x3c] & dl);
        SetFlags(r, gp4::flags::logic8(found));
        if (found != 0u) {
            break;
        }
        Call559244(r);
    }

    const uint8_t kept = static_cast<uint8_t>(Record(r)[0x2c] & dl);
    SetFlags(r, gp4::flags::logic8(kept));
    if (kept == 0u) {
        Call559253(r);
    }
}
GP4_IMPL(0x00557083, FUN_00557083, "regs(eax:u32, ecx:u32, edx:u32{=0x10,0x20,0x40,0x80,0xabcd0040,0x5a5a0010}, ebx:rec[0x8943a0,0x160,0..25], edi:ptr[0x6a]:u8{=0,0,0,1,0x7f,0x80,0xff}) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+bits_0055616f+flags_00874b3e+pan_0055933e+walk_00557083")

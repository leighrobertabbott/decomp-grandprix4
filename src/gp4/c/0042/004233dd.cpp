#include <gp4/gp4.h>
#include <gp4/x87.h>

// One-operand `imul dword` followed by `shrd eax, edx, n`: bits n..n+31 of the signed product,
// edx keeps the high half.
static inline uint32_t mulShr(uint32_t a, uint32_t b, unsigned shift, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a))
        * static_cast<int64_t>(static_cast<int32_t>(b));
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> shift);
}

// Clamps the magnitude of a signed value to an unsigned limit (`neg; cmp; jb; mov; neg`).
static inline uint32_t clampMagnitude(uint32_t value, uint32_t limit) {
    if (static_cast<int32_t>(value) < 0) {
        uint32_t magnitude = 0u - value;
        if (magnitude >= limit) {
            magnitude = limit;
        }
        return 0u - magnitude;
    }
    if (value >= limit) {
        value = limit;
    }
    return value;
}

// FUNCTION: GP4 0x004233dd
// For each of the [0x00609864] view records (0x64 bytes from 0x00621aa0): every point of the
// [0x006097cc] point records (0x14 bytes from [0x006097ac], switching to 0x00620fc0 at index
// [0x006082b4]; records before it with dword +0xc set are skipped) is projected on the third
// column of the body +0x44 plus +0x20; a negative depth counts a contact, builds the two terms
// [0x00608298] (pushed through 0x00401e2e into the gains 0x623e18..20 for 0x00422f77) and
// accumulates the point. The averaged term may register the record (0x00423160); the sum goes
// through the body +0x40 (0x00401f02) into 0x608180..88 and its negation into +0x4c.
void __cdecl FUN_004233dd(gp4::Regs* r) {
    using namespace gp4::x87;

    if (GP4_GLOBAL(uint32_t, 0x00609864) == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;      // test of a zero dword
        return;
    }

    uint8_t* view = GP4_ARRAY(uint8_t, 0x00621aa0);
    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;
    uint32_t edi = r->edi;
    uint32_t outer = 0;

    do {
        GP4_GLOBAL(uint32_t, 0x00608288) = 0u;
        GP4_GLOBAL(uint32_t, 0x00609850) = 0u;
        GP4_GLOBAL(uint32_t, 0x00609854) = 0u;
        GP4_GLOBAL(uint32_t, 0x00609858) = 0u;
        uint8_t* point = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x006097ac)));
        edi = 0x006211a0u;
        uint32_t index = 0;
        do {
            bool skip = false;
            if (index < GP4_GLOBAL(uint32_t, 0x006082b4)) {
                skip = GP4_FIELD(uint32_t, point, 0xc) != 0u;
            } else if (index == GP4_GLOBAL(uint32_t, 0x006082b4)) {
                point = GP4_ARRAY(uint8_t, 0x00620fc0);
            }
            if (!skip) {
                uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, view, 0x44)));
                Ext depth = (ext_i32(GP4_FIELD(int32_t, point, 0)) - ext_i32(GP4_GLOBAL(int32_t, 0x006097b4)))
                    * ext(GP4_FIELD(double, body, 0x68));
                depth = depth + (ext_i32(GP4_FIELD(int32_t, point, 4)) - ext_i32(GP4_GLOBAL(int32_t, 0x006097b8)))
                    * ext(GP4_FIELD(double, body, 0x80));
                depth = depth + (ext_i32(GP4_FIELD(int32_t, point, 8)) - ext_i32(GP4_GLOBAL(int32_t, 0x006097bc)))
                    * ext(GP4_FIELD(double, body, 0x98));
                depth = depth + ext_i32(GP4_FIELD(int32_t, view, 0x20));
                GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(depth);
                eax = GP4_GLOBAL(uint32_t, 0x006f2bf4);
                if (static_cast<int32_t>(eax) < 0) {
                    GP4_GLOBAL(uint8_t, 0x00608280) = 0xffu;
                    GP4_GLOBAL(uint32_t, 0x00608288) += 1u;

                    uint32_t term = mulShr(0u - eax, GP4_GLOBAL(uint32_t, 0x00608014), 18, &edx);
                    if (static_cast<int32_t>(term) > GP4_GLOBAL(int32_t, 0x00608090)) {
                        term = GP4_GLOBAL(uint32_t, 0x00608090);
                    }
                    term = mulShr(term, GP4_FIELD(uint32_t, view, 0x58), 14, &edx);
                    GP4_GLOBAL(uint32_t, 0x00609850) += term;
                    GP4_GLOBAL(uint32_t, 0x00608298) = term;

                    gp4::Regs c = *r;
                    c.st_in = 0;
                    c.st_out = 0;
                    c.eax = term;
                    c.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(point));
                    c.ecx = index;
                    c.edx = edx;
                    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(view));
                    c.edi = edi;
                    c.ebp = ebp;
                    gp4::call_regs(0x00422ece, c);

                    uint32_t gap = GP4_FIELD(uint32_t, view, 0x3c) - c.eax;
                    uint32_t lateral;
                    if (static_cast<int32_t>(gap) < 0) {
                        gap = static_cast<uint32_t>(static_cast<int32_t>(gap) >> 5);
                        lateral = mulShr(gap, GP4_GLOBAL(uint32_t, 0x00608084), 18, &edx);
                    } else {
                        lateral = mulShr(gap, GP4_FIELD(uint32_t, view, 0x5c), 18, &edx);
                    }
                    GP4_GLOBAL(uint32_t, 0x00609854) = clampMagnitude(
                        GP4_GLOBAL(uint32_t, 0x00609854) + lateral, GP4_GLOBAL(uint32_t, 0x00608034));
                    lateral = clampMagnitude(lateral, GP4_GLOBAL(uint32_t, 0x00608044));
                    GP4_GLOBAL(uint32_t, 0x00608298) += lateral;

                    body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, view, 0x44)));
                    GP4_FIELD(double, body, 0xa0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x0060829c));
                    GP4_FIELD(double, body, 0xa8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x0060829c));
                    GP4_FIELD(double, body, 0xb0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00608298));
                    c.eax = lateral;
                    c.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
                    gp4::call_regs(0x00401e2e, c);
                    GP4_GLOBAL(int32_t, 0x00623e18) = fistp32(GP4_FIELD(double, body, 0xb8));
                    GP4_GLOBAL(int32_t, 0x00623e1c) = fistp32(GP4_FIELD(double, body, 0xc0));
                    GP4_GLOBAL(int32_t, 0x00623e20) = fistp32(GP4_FIELD(double, body, 0xc8));
                    gp4::call_regs(0x00422f77, c);
                    edx = c.edx;

                    GP4_GLOBAL(uint32_t, 0x00609834) += GP4_FIELD(uint32_t, point, 0);
                    GP4_GLOBAL(uint32_t, 0x00609838) += GP4_FIELD(uint32_t, point, 4);
                    eax = GP4_FIELD(uint32_t, point, 8);
                    GP4_GLOBAL(uint32_t, 0x0060983c) += eax;
                    GP4_GLOBAL(uint32_t, 0x00609840) += 1u;
                }
            }
            edi += 0x20u;
            point += 0x14;
            ++index;
        } while (index != GP4_GLOBAL(uint32_t, 0x006097cc));
        ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(point));

        ebp = GP4_GLOBAL(uint32_t, 0x00608288);
        if (ebp == 0u) {
            ebp = 1u;
        }
        int32_t remainder;
        const int32_t average = gp4::idiv64(static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00609854)),
                                            static_cast<int32_t>(ebp), &remainder);
        edx = static_cast<uint32_t>(remainder);
        if (average >= GP4_GLOBAL(int32_t, 0x00608040)) {
            int32_t weight = average;
            if (weight > 0x4000000) {
                weight = 0x4000000;
            }
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            c.eax = static_cast<uint32_t>(weight) << 4;
            c.ebx = ebx;
            c.ecx = index;
            c.edx = edx;
            c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(view));
            c.edi = edi;
            c.ebp = ebp;
            gp4::call_regs(0x00423160, c);
        }
        GP4_GLOBAL(uint32_t, 0x0060985c) = static_cast<uint32_t>(average) + GP4_GLOBAL(uint32_t, 0x00609850);
        GP4_GLOBAL(uint32_t, 0x00609830) = 0u;
        if (GP4_GLOBAL(uint32_t, 0x0060985c) != 0u) {
            const uint32_t total = GP4_GLOBAL(uint32_t, 0x0060985c);
            GP4_GLOBAL(uint32_t, 0x00609830) += total;
            GP4_GLOBAL(uint32_t, 0x00608298) = total;
            uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, view, 0x40)));
            GP4_FIELD(double, body, 0xb8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x0060829c));
            GP4_FIELD(double, body, 0xc0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x0060829c));
            GP4_FIELD(double, body, 0xc8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00608298));
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            c.eax = total;
            c.ebx = ebx;
            c.ecx = index;
            c.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
            c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(view));
            c.edi = edi;
            c.ebp = ebp;
            gp4::call_regs(0x00401f02, c);
            GP4_GLOBAL(int32_t, 0x00623e24) = fistp32(GP4_FIELD(double, body, 0xa0));
            GP4_GLOBAL(int32_t, 0x00623e28) = fistp32(GP4_FIELD(double, body, 0xa8));
            GP4_GLOBAL(int32_t, 0x00623e2c) = fistp32(GP4_FIELD(double, body, 0xb0));
            GP4_GLOBAL(uint32_t, 0x00608180) += GP4_GLOBAL(uint32_t, 0x00623e24);
            GP4_GLOBAL(uint32_t, 0x00608184) += mulShr(GP4_GLOBAL(uint32_t, 0x00623e28),
                                                       GP4_GLOBAL(uint32_t, 0x0060986c), 14, &edx);
            GP4_GLOBAL(uint32_t, 0x00608188) += GP4_GLOBAL(uint32_t, 0x00623e2c);
        }
        GP4_FIELD(uint32_t, view, 0x4c) = 0u - GP4_GLOBAL(uint32_t, 0x00609830);
        eax = GP4_GLOBAL(uint32_t, 0x00608288);
        GP4_GLOBAL(uint32_t, 0x0060828c) += eax;
        view += 0x64;
        ++outer;
    } while (outer != GP4_GLOBAL(uint32_t, 0x00609864));

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = outer;
    r->edx = edx;
    r->ebp = ebp;
    r->edi = edi;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;          // the loop-exit compare was equal
}
GP4_IMPL(0x004233dd, FUN_004233dd, "regs() -> (eax, ebx, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=view_00609860+recs_00621aa0+state_00424011+slots_0041891a+match_00423160+view60_00621aa0")

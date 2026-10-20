#include <gp4/gp4.h>
#include <gp4/flags.h>

// The four 0x300-byte wheel records at 0x006380d8, walked through a base register as the original does
// (`mov ebx, 0x6380d8` ... `add ebx, 0x300; cmp ebx, 0x638cd8`).
static __declspec(noinline) uint8_t* wheelRecords() {
    return GP4_ARRAY(uint8_t, 0x006380d8);
}

// The original opens with `cmp dword [0x66d814], 0x11172; jne next` - a compare whose both outcomes
// continue at the same instruction and whose flags the following `test` replaces. The clock read is
// kept (into a private stack slot of the caller) so the reconstruction touches the same global.
static __declspec(noinline) void readClock(uint32_t* slot, uint32_t clock) {
    *slot = clock;
}

// imul by a Q14 weight, then `shrd eax, edx, 14`: bits 14..45 of the signed 64-bit product.
static inline uint32_t mulQ14(int32_t value, int32_t weight) {
    const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(weight);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// Per-frame wheel contact update for the car in esi. Byte 0x00638d60 = 0xff when bit 1 of +0x8d or
// bit 0 of +0x1ce is set (else 0); 0x00601eac and the bit string 0x00601eb0 are cleared. For each wheel
// k (ebx = record k, ecx = k, edi = 12k): 0x0041e156 moves the contact point (record +0x1e0/+0x1e4 =
// 0x00623e30/0x00623e38), 0x0041e069 locates it on the track (index 0x0062789c = k; +8/+0xc/+0x154
// from 0x006278d0/0x006278cc/0x00623e34), 0x0041e6ec publishes it (+0x58 = 0x00602004, +0x24 =
// 0x00601f18, byte +0x2d = surface code 0x00601f1e & 0xf). The surface pair tables 0x00602d8c/0x00602dbc
// (weight 0x4000 - +0xd4) and 0x00602da4/0x00602dd4 (weight +0xd4), indexed by the code (0 when bit 7
// of the car's +0x7b), are blended in Q14 into +0xe8 and +0xe4. Wheel 0 runs 0x0041e500. With bit 1 of
// +0x162 the car's per-wheel +0x37c takes 0x00602004, +0x30c/+0x34c are cleared and +0x2fc = +0x2dc.
// Unless byte 0x006246e0 is set or the wheel's bit differs between 0x00601eb0 and the car's byte
// +0x2bb, record +0x6c = ((+0x58 - car +0x37c) << 16) / [0x0060384c] (the shl 7 wraps first); otherwise
// +0x6c = 0 and car +0x37c = +0x58. Afterwards bit 1 of +0x162 is cleared, the car's +0x76 bits
// 0x80..0x10 mark wheels with code 4 (when byte 0x00625ab8 is 0xff), +0x8f bit 0x80 marks a nonzero
// code on wheel 0 and bits 0x40..0x10 a zero code on wheels 1..3. When 0x00601eb0 equals the previous
// +0x2bb and is 0 or 0xf, 0x0041e417 runs and byte 0x006228c1 = 0xff (else 0); +0x2bb = 0x00601eb0.
// FUNCTION: GP4 0x0041e192
void __cdecl FUN_0041e192(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t clockSlot;
    readClock(&clockSlot, GP4_GLOBAL(uint32_t, 0x0066d814));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint8_t latch = 0xffu;
    if ((car[0x8d] & 0x02u) == 0u && (car[0x1ce] & 0x01u) == 0u) {
        latch = 0u;
    }
    c.eax = (c.eax & 0xffffff00u) | latch;
    GP4_GLOBAL(uint8_t, 0x00638d60) = latch;

    c.edi = 0u;
    uint8_t* record = wheelRecords();
    GP4_GLOBAL(uint8_t, 0x00601eac) = 0u;
    GP4_GLOBAL(uint32_t, 0x00601eb0) = 0u;

    uint32_t wheel = 0u;
    do {
        c.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record));
        c.ecx = wheel;
        gp4::call_regs(0x0041e156, c);
        GP4_FIELD(uint32_t, record, 0x1e0) = GP4_GLOBAL(uint32_t, 0x00623e30);
        c.eax = GP4_GLOBAL(uint32_t, 0x00623e38);
        GP4_FIELD(uint32_t, record, 0x1e4) = c.eax;
        GP4_GLOBAL(uint32_t, 0x0062789c) = wheel;
        gp4::call_regs(0x0041e069, c);
        GP4_FIELD(uint32_t, record, 0x8) = GP4_GLOBAL(uint32_t, 0x006278d0);
        GP4_FIELD(uint32_t, record, 0xc) = GP4_GLOBAL(uint32_t, 0x006278cc);
        c.eax = GP4_GLOBAL(uint32_t, 0x00623e34);
        GP4_FIELD(uint32_t, record, 0x154) = c.eax;
        gp4::call_regs(0x0041e6ec, c);
        GP4_FIELD(uint32_t, record, 0x58) = GP4_GLOBAL(uint32_t, 0x00602004);
        GP4_FIELD(uint32_t, record, 0x24) = GP4_GLOBAL(uint32_t, 0x00601f18);

        uint32_t code = GP4_GLOBAL(uint8_t, 0x00601f1e) & 0x0fu;
        record[0x2d] = static_cast<uint8_t>(code);
        const int32_t weight = GP4_FIELD(int32_t, record, 0xd4);
        if ((car[0x7b] & 0x80u) != 0u) {
            code = 0u;
        }
        const int32_t rest = 0x4000 - weight;
        const uint32_t firstA = mulQ14(GP4_ARRAY(int32_t, 0x00602d8c)[code], rest);
        const uint32_t firstB = mulQ14(GP4_ARRAY(int32_t, 0x00602da4)[code], weight);
        GP4_FIELD(uint32_t, record, 0xe8) = firstB + firstA;
        const uint32_t secondA = mulQ14(GP4_ARRAY(int32_t, 0x00602dbc)[code], rest);
        const uint32_t secondB = mulQ14(GP4_ARRAY(int32_t, 0x00602dd4)[code], weight);
        c.eax = secondB + secondA;
        GP4_FIELD(uint32_t, record, 0xe4) = c.eax;
        c.edx = secondA;
        c.ebp = static_cast<uint32_t>(rest);
        c.ecx = wheel;

        if (wheel == 0u) {
            gp4::call_regs(0x0041e500, c);
        }

        if ((car[0x162] & 0x02u) != 0u) {
            GP4_FIELD(uint32_t, car, 0x37c + wheel * 4u) = GP4_GLOBAL(uint32_t, 0x00602004);
            GP4_FIELD(uint32_t, car, 0x30c + wheel * 4u) = 0u;
            GP4_FIELD(uint32_t, car, 0x34c + wheel * 4u) = 0u;
            c.eax = GP4_FIELD(uint32_t, car, 0x2dc + wheel * 4u);
            GP4_FIELD(uint32_t, car, 0x2fc + wheel * 4u) = c.eax;
        }

        bool settled = GP4_GLOBAL(uint8_t, 0x006246e0) != 0u;
        if (!settled) {
            c.eax = GP4_GLOBAL(uint32_t, 0x00601eb0) ^ static_cast<uint32_t>(car[0x2bb]);
            c.edx = GP4_GLOBAL(uint32_t, 0x0062789c);
            settled = ((c.eax >> (c.edx & 31u)) & 1u) != 0u;
        }
        if (settled) {
            c.eax = GP4_FIELD(uint32_t, record, 0x58);
            GP4_FIELD(uint32_t, car, 0x37c + wheel * 4u) = c.eax;
            GP4_FIELD(uint32_t, record, 0x6c) = 0u;
        } else {
            const uint32_t shifted = (GP4_FIELD(uint32_t, record, 0x58) - GP4_FIELD(uint32_t, car, 0x37c + wheel * 4u)) << 7;
            const int64_t dividend = static_cast<int64_t>(static_cast<int32_t>(shifted)) * 512;
            int32_t remainder;
            const int32_t rate = gp4::idiv64(dividend, GP4_GLOBAL(int32_t, 0x0060384c), &remainder);
            c.eax = static_cast<uint32_t>(rate);
            c.edx = static_cast<uint32_t>(remainder);
            GP4_FIELD(int32_t, record, 0x6c) = rate;
        }

        ++wheel;
        c.edi += 0xcu;
        record += 0x300;
    } while (reinterpret_cast<uintptr_t>(record) != 0x00638cd8u);
    c.ecx = wheel;

    car[0x162] &= 0xfdu;

    uint8_t* records = wheelRecords();
    c.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(records));
    if (GP4_GLOBAL(uint8_t, 0x00625ab8) == 0xffu) {
        if (records[0x2d] == 4u) car[0x76] |= 0x80u;
        if (records[0x32d] == 4u) car[0x76] |= 0x40u;
        if (records[0x62d] == 4u) car[0x76] |= 0x20u;
        if (records[0x92d] == 4u) car[0x76] |= 0x10u;
    }
    if (records[0x2d] != 0u) car[0x8f] |= 0x80u;
    if (records[0x32d] == 0u) car[0x8f] |= 0x40u;
    if (records[0x62d] == 0u) car[0x8f] |= 0x20u;
    if (records[0x92d] == 0u) car[0x8f] |= 0x10u;

    const uint32_t bits = GP4_GLOBAL(uint32_t, 0x00601eb0);
    const uint32_t previous = car[0x2bb];
    c.edx = previous;
    c.eax = bits ^ previous;
    uint32_t flags;
    if (c.eax != 0u) {
        flags = gp4::flags::logic32(c.eax);
        GP4_GLOBAL(uint8_t, 0x006228c1) = 0u;
    } else {
        c.eax = GP4_GLOBAL(uint32_t, 0x00601eb0);
        if (c.eax == 0u || c.eax == 0x0fu) {
            gp4::call_regs(0x0041e417, c);
            flags = c.eflags;
            GP4_GLOBAL(uint8_t, 0x006228c1) = 0xffu;
        } else {
            flags = gp4::flags::sub32(c.eax, 0x0fu);
            GP4_GLOBAL(uint8_t, 0x006228c1) = 0u;
        }
    }
    c.eax = GP4_GLOBAL(uint32_t, 0x00601eb0);
    car[0x2bb] = static_cast<uint8_t>(c.eax);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0041e192, FUN_0041e192, "regs(esi:ptr[0x420]:layout=car_00420d3f+car_0041e500+car_0041e192) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041befb+state_0041e500+ring_008943a0+records_008943a0+state_004094e3+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+state_004381ab+trackgeo_008943a0+state_0040705f+frac_0040705f+state_00407af6+state_0040759e+trackfar_008943a0+state_0041dbb0+state_0040342a+tbl_00868598+steps_0041dada+state_0041d996+state_0041d9b6+mesh_00453330+state_0041d9eb+state_0041da34+ptr_00602008+ptr_00602008+win_0041e069+mesh_0041e192+step_0041e192; maxinsn=16000000; wall=1800")

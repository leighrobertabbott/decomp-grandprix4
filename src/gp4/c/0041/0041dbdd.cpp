#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of SHL r32, count (count > 1) in the pinned oracle model: CF = last bit out, AF = 0,
// OF = bit31(value << (count - 1) ^ result).
static inline uint32_t shlFlags32(uint32_t value, uint32_t count) {
    const uint32_t result = value << count;
    const uint32_t previous = value << (count - 1u);
    return (previous >> 31)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

enum PathKind { PathOutside, PathFull, PathPartial };

// edi = the current 0x160-byte track-segment record. Copies the position dwords 0x6278c0/0x6278c8
// to 0x623e30/0x623e38 and 0x623e34 to 0x627834. With [0x6278cc] != 0 it walks the record ring
// through 0x0041dada (edx = 3 when [0x6278b0] < 0, else 2), storing its eax/edx in
// 0x62782c/0x627830 (edi moves to the walked record); otherwise 0x62782c/0x627830 are copied from
// 0x6278b0/0x6278b4. The magnitude of [0x62782c] >> 8 minus (word +0x60 or +0x62 of the record
// + 0x50, by the sign) selects one of three paths: outside (0x0041d9b6, code 0), full
// (0x0040342a then 0x0041d9eb, rebased by 0x627834 - 0x627838) or partial (0x0040342a then the
// 0x00403337 blend, plus a 0x0041d9eb term when its code is 5). The result is clamped to
// +-[0x627840] (the excess goes to 0x623e34), shifted left by 4 into 0x602004, and the low byte
// of the path's code goes to 0x601f1e. edi is saved and restored.
// FUNCTION: GP4 0x0041dbdd
void __cdecl FUN_0041dbdd(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00623e30) = GP4_GLOBAL(uint32_t, 0x006278c0);
    GP4_GLOBAL(uint32_t, 0x00623e38) = GP4_GLOBAL(uint32_t, 0x006278c8);
    GP4_GLOBAL(uint32_t, 0x00627834) = GP4_GLOBAL(uint32_t, 0x00623e34);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = GP4_GLOBAL(uint32_t, 0x00623e34);

    if (GP4_GLOBAL(uint32_t, 0x006278cc) != 0u) {
        c.edx = GP4_GLOBAL(int32_t, 0x006278b0) < 0 ? 3u : 2u;
        gp4::call_regs(0x0041dada, c);
        GP4_GLOBAL(uint32_t, 0x0062782c) = c.eax;
        GP4_GLOBAL(uint32_t, 0x00627830) = c.edx;
    } else {
        GP4_GLOBAL(uint32_t, 0x0062782c) = GP4_GLOBAL(uint32_t, 0x006278b0);
        c.eax = GP4_GLOBAL(uint32_t, 0x006278b4);
        GP4_GLOBAL(uint32_t, 0x00627830) = c.eax;
    }

    const int32_t lateral = GP4_GLOBAL(int32_t, 0x0062782c);
    uint32_t distance = static_cast<uint32_t>(lateral >> 8);
    if (static_cast<int32_t>(distance) < 0) {
        distance = 0u - distance;
    }

    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi));
    uint32_t side;
    uint32_t width;
    uint8_t mask;
    if (lateral < 0) {
        side = 3u;
        width = GP4_FIELD(uint16_t, record, 0x60);
        mask = 8u;
    } else {
        side = 2u;
        width = GP4_FIELD(uint16_t, record, 0x62);
        mask = 4u;
    }
    c.eax = width + 0x50u;
    c.ecx = distance - c.eax;

    PathKind path;
    bool decided = false;
    if ((GP4_FIELD(uint8_t, record, 0x2c) & mask) != 0u) {
        c.edx = side;
        c.eax = GP4_GLOBAL(uint32_t, 0x00627830);
        gp4::call_regs(0x0041da34, c);
        if ((c.eflags & 1u) != 0u) {
            GP4_GLOBAL(uint32_t, 0x006278a0) = c.eax;
            c.edx = side;
            gp4::call_regs(0x00403189, c);
            if (static_cast<int32_t>(c.ecx) < static_cast<int32_t>(c.eax)) {
                path = PathOutside;
            } else if (static_cast<int32_t>(c.ecx) < static_cast<int32_t>(c.edx)) {
                path = PathPartial;
            } else {
                path = PathFull;
            }
            decided = true;
        }
    }
    if (!decided) {
        path = static_cast<int32_t>(c.ecx) < 0 ? PathOutside : PathFull;
    }

    if (path == PathOutside) {
        gp4::call_regs(0x0041d9b6, c);
        c.edx = 0u;
    } else {
        c.edx = side;
        gp4::call_regs(0x0040342a, c);
        GP4_GLOBAL(uint32_t, 0x00627838) = c.eax;
        GP4_GLOBAL(uint32_t, 0x0062783c) = c.edx;
        if (path == PathFull) {
            gp4::call_regs(0x0041d9eb, c);
            c.eax = c.eax + GP4_GLOBAL(uint32_t, 0x00627834) - GP4_GLOBAL(uint32_t, 0x00627838);
            c.ecx = GP4_GLOBAL(uint32_t, 0x00627838);
            GP4_GLOBAL(uint32_t, 0x00623e34) = c.ecx;
            gp4::call_regs(0x0041dbb0, c);
        } else {
            c.eax = GP4_GLOBAL(uint32_t, 0x006278a0);
            c.edx = side;
            gp4::call_regs(0x00403337, c);
            c.eax = c.eax + c.ecx - GP4_GLOBAL(uint32_t, 0x00627838);
            c.ecx = GP4_GLOBAL(uint32_t, 0x00627838);
            GP4_GLOBAL(uint32_t, 0x00623e34) = c.ecx;
            gp4::call_regs(0x0041dbb0, c);
            if (c.edx == 5u) {
                const uint32_t partial = c.eax;
                c.edx = side;
                gp4::call_regs(0x0041d9eb, c);
                c.ebp = partial;
                c.eax += partial;
            }
        }
    }

    const uint32_t code = c.edx;
    uint32_t limit = GP4_GLOBAL(uint32_t, 0x00627840);
    uint32_t value = c.eax;
    if (static_cast<int32_t>(value) < 0) {
        limit = 0u - limit;
        if (static_cast<int32_t>(value) < static_cast<int32_t>(limit)) {
            GP4_GLOBAL(uint32_t, 0x00623e34) += value - limit;
            value = limit;
        }
    } else if (static_cast<int32_t>(value) > static_cast<int32_t>(limit)) {
        GP4_GLOBAL(uint32_t, 0x00623e34) += value - limit;
        value = limit;
    }

    const uint32_t result = value << 4;
    GP4_GLOBAL(uint32_t, 0x00602004) = result;
    GP4_GLOBAL(uint8_t, 0x00601f1e) = static_cast<uint8_t>(code);

    r->eax = result;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = code;
    r->esi = c.esi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | shlFlags32(value, 4u);
}
GP4_IMPL(0x0041dbdd, FUN_0041dbdd, "regs(edi:rec[0x8943a0,0x160,0..11]) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041dbb0+records_008943a0+state_0040342a+tbl_00868598+trackgeo_008943a0+ring_008943a0+state_0040759e+steps_0041dada+state_0041d996+state_0041d9b6+mesh_00453330+state_0041d9eb+state_0041da34")

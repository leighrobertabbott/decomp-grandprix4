#include <gp4/gp4.h>
#include <gp4/flags.h>

// edi = record; the twin of 0x00555474 for the other side. 0x00553ad4 stores a rotated point in
// 0x934df0/0x934df4; both are scaled down by 32, and the words +0xd6/+0xda/+0xde get
// (point.x - [+0xf0]/32)/8, (0x00555454 result)/8 and (point.z - [+0xf8]/32)/8.
// 0x8754ac/0x8754b4 = word*8 + dword/32 for (+0xba, +0xf0) and (+0xc2, +0xf8). ebp = the larger
// magnitude of their offsets from the point; below 0x400 the word +0xd2 = word +0xa, else the angle
// of the offset (0x00401315) - 0x4000. eax/edx keep the offsets (ax replaced).
// FUNCTION: GP4 0x00555555
void __cdecl FUN_00555555(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    gp4::call_regs(0x00553ad4, *r);

    int32_t& pointX = GP4_GLOBAL(int32_t, 0x00934df0);
    int32_t& pointZ = GP4_GLOBAL(int32_t, 0x00934df4);
    pointX >>= 5;
    pointZ >>= 5;

    int32_t baseX = GP4_FIELD(int32_t, rec, 0xf0) >> 5;
    uint32_t value = static_cast<uint32_t>(static_cast<int32_t>(static_cast<uint32_t>(pointX) - static_cast<uint32_t>(baseX)) >> 3);
    GP4_FIELD(uint16_t, rec, 0xd6) = static_cast<uint16_t>(value);
    r->eax = value;
    r->edx = static_cast<uint32_t>(baseX);
    gp4::call_regs(0x00555454, *r);
    GP4_FIELD(uint16_t, rec, 0xda) = static_cast<uint16_t>(static_cast<int32_t>(r->eax) >> 3);

    int32_t baseZ = GP4_FIELD(int32_t, rec, 0xf8) >> 5;
    value = static_cast<uint32_t>(static_cast<int32_t>(static_cast<uint32_t>(pointZ) - static_cast<uint32_t>(baseZ)) >> 3);
    GP4_FIELD(uint16_t, rec, 0xde) = static_cast<uint16_t>(value);

    GP4_GLOBAL(uint32_t, 0x008754ac) = (static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, rec, 0xba))) << 3)
        + static_cast<uint32_t>(GP4_FIELD(int32_t, rec, 0xf0) >> 5);
    GP4_GLOBAL(uint32_t, 0x008754b4) = (static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, rec, 0xc2))) << 3)
        + static_cast<uint32_t>(GP4_FIELD(int32_t, rec, 0xf8) >> 5);

    const uint32_t dx = GP4_GLOBAL(uint32_t, 0x008754ac) - GP4_GLOBAL(uint32_t, 0x00934df0);
    const uint32_t dz = GP4_GLOBAL(uint32_t, 0x008754b4) - GP4_GLOBAL(uint32_t, 0x00934df4);
    uint32_t far = static_cast<int32_t>(dx) < 0 ? 0u - dx : dx;
    const uint32_t farZ = static_cast<int32_t>(dz) < 0 ? 0u - dz : dz;
    if (static_cast<int32_t>(far) < static_cast<int32_t>(farZ)) {
        far = farZ;
    }
    r->ebp = far;
    r->eax = dx;
    r->edx = dz;

    uint32_t fl;
    if (static_cast<int32_t>(far) < 0x400) {
        fl = gp4::flags::sub32(far, 0x400u);
        r->eax = (dx & 0xffff0000u) | GP4_FIELD(uint16_t, rec, 0xa);
    } else {
        gp4::call_regs(0x00401315, *r);
        fl = gp4::flags::sub32(r->eax, 0x4000u);
        r->eax -= 0x4000u;
    }
    GP4_FIELD(uint16_t, rec, 0xd2) = static_cast<uint16_t>(r->eax);
    r->eflags = gp4::flags::merge(r->eflags, fl);
}
GP4_IMPL(0x00555555, FUN_00555555, "regs(edi:ptr[0x158]:i16[-200..200]) -> (eax, edx, ebp, cf, pf, af, zf, sf, of)")

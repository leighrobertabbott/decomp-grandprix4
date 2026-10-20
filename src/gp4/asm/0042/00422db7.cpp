#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x00422db7
// ebx = point (three i32), edi = result block (six i32). The point relative to the origin
// [0x6097b4/b8/bc] is rotated by the two matrix columns at 0x00622e98 + (0x58, 0x70, 0x88) and
// (0x68, 0x80, 0x98), each sum kept on the x87 stack (80-bit) until its fistp, and stored to
// edi[0] and edi[2]. edi[3..5] are the cross product of the vector ([0x607f44], [0x607f48] ...)
// scaled by qword [0x6f2bcc] with edi[0..2], plus [0x607f34/38/3c]. Every value goes through the
// scratch dword 0x006f2bf4; eax holds the last one and edx the matrix base 0x00622e98.
void __cdecl FUN_00422db7(gp4::Regs* r) {
    using namespace gp4::x87;

    uint8_t* const point = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* const out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint8_t* const matrix = GP4_ARRAY(uint8_t, 0x00622e98);
    const int32_t px = GP4_FIELD(int32_t, point, 0);
    const int32_t py = GP4_FIELD(int32_t, point, 4);
    const int32_t pz = GP4_FIELD(int32_t, point, 8);

    Ext sum = (ext_i32(px) - ext_i32(GP4_GLOBAL(int32_t, 0x006097b4))) * ext(GP4_FIELD(double, matrix, 0x58));
    sum = sum + (ext_i32(py) - ext_i32(GP4_GLOBAL(int32_t, 0x006097b8))) * ext(GP4_FIELD(double, matrix, 0x70));
    sum = sum + (ext_i32(pz) - ext_i32(GP4_GLOBAL(int32_t, 0x006097bc))) * ext(GP4_FIELD(double, matrix, 0x88));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(sum);
    GP4_FIELD(int32_t, out, 0) = GP4_GLOBAL(int32_t, 0x006f2bf4);

    Ext sum2 = (ext_i32(px) - ext_i32(GP4_GLOBAL(int32_t, 0x006097b4))) * ext(GP4_FIELD(double, matrix, 0x68));
    sum2 = sum2 + (ext_i32(py) - ext_i32(GP4_GLOBAL(int32_t, 0x006097b8))) * ext(GP4_FIELD(double, matrix, 0x80));
    sum2 = sum2 + (ext_i32(pz) - ext_i32(GP4_GLOBAL(int32_t, 0x006097bc))) * ext(GP4_FIELD(double, matrix, 0x98));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(sum2);
    GP4_FIELD(int32_t, out, 8) = GP4_GLOBAL(int32_t, 0x006f2bf4);

    const Ext scale = ext(GP4_GLOBAL(double, 0x006f2bcc));

    Ext lhs = ext_i32(GP4_GLOBAL(int32_t, 0x00607f44)) * scale * ext_i32(GP4_FIELD(int32_t, out, 8));
    Ext rhs = ext_i32(GP4_GLOBAL(int32_t, 0x00607f48)) * scale * ext_i32(GP4_FIELD(int32_t, out, 4));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32((rhs - lhs) + ext_i32(GP4_GLOBAL(int32_t, 0x00607f34)));
    GP4_FIELD(int32_t, out, 0xc) = GP4_GLOBAL(int32_t, 0x006f2bf4);

    lhs = ext_i32(GP4_GLOBAL(int32_t, 0x00607f48)) * scale * ext_i32(GP4_FIELD(int32_t, out, 0));
    rhs = ext_i32(GP4_GLOBAL(int32_t, 0x00607f40)) * scale * ext_i32(GP4_FIELD(int32_t, out, 8));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32((rhs - lhs) + ext_i32(GP4_GLOBAL(int32_t, 0x00607f38)));
    GP4_FIELD(int32_t, out, 0x10) = GP4_GLOBAL(int32_t, 0x006f2bf4);

    lhs = ext_i32(GP4_GLOBAL(int32_t, 0x00607f40)) * scale * ext_i32(GP4_FIELD(int32_t, out, 4));
    rhs = ext_i32(GP4_GLOBAL(int32_t, 0x00607f44)) * scale * ext_i32(GP4_FIELD(int32_t, out, 0));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32((rhs - lhs) + ext_i32(GP4_GLOBAL(int32_t, 0x00607f3c)));
    GP4_FIELD(int32_t, out, 0x14) = GP4_GLOBAL(int32_t, 0x006f2bf4);

    r->eax = GP4_GLOBAL(uint32_t, 0x006f2bf4);
    r->edx = 0x00622e98u;
}
GP4_IMPL(0x00422db7, FUN_00422db7, "regs(ebx:ptr[0x10]:i32[-1000000..1000000], edi:ptr[0x1c]:i32[-1000000..1000000]) -> (eax, edx); globals=fuzz:i32+layout=view_00609860")

#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x00422ece
// ebx = point (three i32), esi = record: i32 +0x2c/+0x30/+0x34 and a pointer at +0x44 to the
// orientation body whose 3x3 matrix qwords start at +0x58 (rows of three, stride 0x18).
// The point relative to the origin [0x6097b4/b8/bc] is rotated by the first two columns,
// each sum kept on the x87 stack (80-bit) until its fistp:
//   [0x6082ac] = col(0x58, 0x70, 0x88), [0x6082b0] = col(0x60, 0x78, 0x90)
// and the result is fistp32((i34 * k * [0x6082ac] - i30 * k * [0x6082b0]) + i2c) with
// k = qword [0x6f2bcc]; it is stored through the scratch dword 0x6f2bf4 and returned in eax.
// edx is left holding the body pointer.
void __cdecl FUN_00422ece(gp4::Regs* r) {
    using namespace gp4::x87;

    const uint8_t* const point = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, rec, 0x44)));
    const int32_t px = GP4_FIELD(int32_t, const_cast<uint8_t*>(point), 0);
    const int32_t py = GP4_FIELD(int32_t, const_cast<uint8_t*>(point), 4);
    const int32_t pz = GP4_FIELD(int32_t, const_cast<uint8_t*>(point), 8);

    Ext sum = (ext_i32(px) - ext_i32(GP4_GLOBAL(int32_t, 0x006097b4))) * ext(GP4_FIELD(double, body, 0x58));
    sum = sum + (ext_i32(py) - ext_i32(GP4_GLOBAL(int32_t, 0x006097b8))) * ext(GP4_FIELD(double, body, 0x70));
    sum = sum + (ext_i32(pz) - ext_i32(GP4_GLOBAL(int32_t, 0x006097bc))) * ext(GP4_FIELD(double, body, 0x88));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(sum);
    GP4_GLOBAL(int32_t, 0x006082ac) = GP4_GLOBAL(int32_t, 0x006f2bf4);

    Ext sum2 = (ext_i32(px) - ext_i32(GP4_GLOBAL(int32_t, 0x006097b4))) * ext(GP4_FIELD(double, body, 0x60));
    sum2 = sum2 + (ext_i32(py) - ext_i32(GP4_GLOBAL(int32_t, 0x006097b8))) * ext(GP4_FIELD(double, body, 0x78));
    sum2 = sum2 + (ext_i32(pz) - ext_i32(GP4_GLOBAL(int32_t, 0x006097bc))) * ext(GP4_FIELD(double, body, 0x90));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(sum2);
    GP4_GLOBAL(int32_t, 0x006082b0) = GP4_GLOBAL(int32_t, 0x006f2bf4);

    const Ext scale = ext(GP4_GLOBAL(double, 0x006f2bcc));
    const Ext lhs = ext_i32(GP4_FIELD(int32_t, rec, 0x30)) * scale * ext_i32(GP4_GLOBAL(int32_t, 0x006082b0));
    const Ext rhs = ext_i32(GP4_FIELD(int32_t, rec, 0x34)) * scale * ext_i32(GP4_GLOBAL(int32_t, 0x006082ac));
    const Ext total = (rhs - lhs) + ext_i32(GP4_FIELD(int32_t, rec, 0x2c));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(total);

    r->eax = GP4_GLOBAL(uint32_t, 0x006f2bf4);
    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
}
GP4_IMPL(0x00422ece, FUN_00422ece, "regs(ebx:ptr[0x40]:bytes, esi:ptr[0x48]:layout=obj_00609860) -> (eax, edx); globals=fuzz:i32")

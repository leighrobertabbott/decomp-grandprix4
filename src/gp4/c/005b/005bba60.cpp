#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005bcd00 { uint8_t Unknown00[0x10]; int32_t Slots10[1]; };
#pragma pack(pop)

// Looks up the object with index DAT_00b4765c in the table of the object DAT_00b471c4 (FUN_005bcd00) and,
// if there is one, calls its virtual method at vtable +0x10 with (0,1,0) when the byte at 0x009ed5b0 is set,
// else (1,1,0). The method returns nothing: the eax callers read afterwards is what the original leaves in
// it at the call - the lookup result with its low byte replaced by the flag byte in the first case, the
// vtable pointer in the second, 0 when the lookup yields no object.
// FUNCTION: GP4 0x005bba60
uint32_t __cdecl FUN_005bba60(void) {
    const uint32_t object = gp4::thiscall_<0x005bcd00, uint32_t>(
        GP4_GLOBAL(Unknown_005bcd00*, 0x00b471c4), GP4_GLOBAL(int32_t, 0x00b4765c));
    if (object == 0) {
        return 0;
    }
    const uint32_t vtable = *reinterpret_cast<const uint32_t*>(object);
    const uint32_t method = reinterpret_cast<const uint32_t*>(vtable)[4];
    gp4::Regs regs = {};
    regs.ecx = object;
    regs.edx = vtable;
    const uint8_t flag = GP4_GLOBAL(uint8_t, 0x009ed5b0);
    if (flag != 0) {
        const uint32_t args[3] = {0, 1, 0};
        regs.eax = (object & 0xffffff00u) | flag;
        gp4::call_regs_stdcall(method, regs, args);
        return regs.eax;
    }
    const uint32_t args[3] = {1, 1, 0};
    regs.eax = vtable;
    gp4::call_regs_stdcall(method, regs, args);
    return regs.eax;
}
GP4_IMPL(0x005bba60, FUN_005bba60, "cdecl() -> u32; globals=fuzz:i32; gfields={0x00b471c4:ptr[0x100]:{0x10*8/4:nptr[0x40]:{0:ptr[0x20]:{0x10:u32{=0x005bc1f0}}}}, 0x00b4765c:i32[0..7], 0x009ed5b0:u8{0,1,0x7f}}")

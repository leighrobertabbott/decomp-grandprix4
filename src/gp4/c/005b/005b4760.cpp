#include <gp4/gp4.h>

// When the object pointer at 0x00b2dd0c is set, forwards the argument to 0x005b5b80 on it.
// FUNCTION: GP4 0x005b4760
void __cdecl FUN_005b4760(int32_t value) {
    void* object = GP4_GLOBAL(void*, 0x00b2dd0c);
    if (object != nullptr) {
        gp4::thiscall_<0x005b5b80, void>(object, value);
    }
}
GP4_IMPL(0x005b4760, FUN_005b4760, "cdecl(i32) -> void; gfields={0x00b2dd0c:nptr[0x58]:bytes}")

"""Small, real-Unicorn counterexamples for the acceptance oracle.

Synthetic images contain no game assets and require neither MSVC nor analysis
state. Each failure exercises executable x86 rather than mocking the oracle.
"""
from __future__ import annotations

import struct
import random
import unittest
from pathlib import Path
from unittest.mock import patch

from gp4re.binary import Binary, Section
from gp4re.emu import CaseGen, Impl, Machine, PAGE, _float_equal, compare_pages, differential
from gp4re.spec import parse


BASE, TARGET, INIT, DLL, IMPL = 0x400000, 0x401000, 0x527453, 0x10000000, 0x10001000


def images(original: bytes, reconstruction: bytes, data: dict[int, bytes] | None = None):
    game = bytearray(0x128000)
    game[TARGET - BASE: TARGET - BASE + len(original)] = original
    game[INIT - BASE] = 0xC3
    for addr, raw in (data or {}).items():
        game[addr - BASE: addr - BASE + len(raw)] = raw
    recon = bytearray(0x3000)
    recon[IMPL - DLL: IMPL - DLL + len(reconstruction)] = reconstruction
    code = Section(".text", TARGET, 0x127000, 0x127000, 0x60000020, 0, True)
    dcode = Section(".text", IMPL, 0x2000, 0x2000, 0x60000020, 0, True)
    return (Binary(Path("synthetic-gp4.exe"), b"", BASE, TARGET, game, [code]),
            Binary(Path("synthetic-recon.dll"), b"", DLL, IMPL, recon, [dcode]))


class OracleSoundnessTests(unittest.TestCase):
    def run_pair(self, original, reconstruction, spec, *, data=None, settings=None):
        cfg = {"init_calls": [INIT], "max_cases": 4, "time_budget_s": 2,
               "min_block_coverage": 0.90, "min_branch_coverage": 0.70, "ni_cases": 1}
        cfg.update(settings or {})
        gp4, dll = images(original, reconstruction, data)
        rec = Impl(TARGET, IMPL, "synthetic", spec)
        with patch("gp4re.config.oracle", side_effect=cfg.__getitem__):
            return differential(gp4, dll, rec, parse(spec), fpucw=0x037F, cases=2,
                                function_starts={TARGET, INIT})

    def test_omitted_register_output_cannot_accept_noop(self):
        r = self.run_pair(bytes.fromhex("b82a000000c3"), b"\xc3", "regs() -> ()")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("eax:", r["mismatch"])

    def test_reconstruction_cannot_clobber_undeclared_preserved_register(self):
        # mov eax,[esp+4]; mov dword ptr[eax+4],42; ret -- overwrite r.ebx
        r = self.run_pair(b"\xc3", bytes.fromhex("8b442404c740042a000000c3"), "regs() -> ()")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("ebx:", r["mismatch"])

    def test_missing_input_cannot_be_replaced_with_canary_constant(self):
        r = self.run_pair(bytes.fromhex("89f8c3"), bytes.fromhex("b80005dec0c3"), "cdecl() -> u32")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("hidden input edi", r["mismatch"])

    def test_hidden_carry_input_is_detected(self):
        # setb al; movzx eax,al; ret
        code = bytes.fromhex("0f92c00fb6c0c3")
        r = self.run_pair(code, code, "cdecl() -> u32")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("hidden input cf", r["mismatch"])

    def test_declared_carry_input_is_generated(self):
        # Original mov eax,0; adc eax,0 has a declared incoming carry. The
        # reconstruction implements it in r.eax and computes arithmetic flags.
        original = bytes.fromhex("b80000000083d000c3")
        # mov edx,[esp+4]; push [edx+0x1c]; popfd; mov eax,0; adc eax,0;
        # mov [edx],eax; pushfd; pop eax; mov [edx+0x1c],eax; ret
        recon = bytes.fromhex("8b542404ff721c9db80000000083d00089029c5889421cc3")
        # edx is restored from the input block, so the native local register is
        # irrelevant; original does not modify its incoming edx.
        r = self.run_pair(original, recon, "regs(cf:u32[0..1]) -> (eax,cf,pf,af,zf,sf,of)")
        self.assertEqual(r["status"], "pass", r)

    def test_signed_zero_is_bit_exact(self):
        r = self.run_pair(bytes.fromhex("d9eec3"), bytes.fromhex("d9eed9e0c3"), "cdecl() -> f64")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("return st0", r["mismatch"])

    def test_distinct_nan_payloads_are_bit_exact(self):
        data = {0x401300: struct.pack("<Q", 0x7FF8000000000001),
                0x401308: struct.pack("<Q", 0x7FF8000000000002)}
        r = self.run_pair(b"\xdd\x05" + struct.pack("<I", 0x401300) + b"\xc3",
                          b"\xdd\x05" + struct.pack("<I", 0x401308) + b"\xc3",
                          "cdecl() -> f64", data=data)
        self.assertEqual(r["status"], "fail", r)

    def test_compiled_abi_preserves_ebx(self):
        r = self.run_pair(b"\xc3", bytes.fromhex("bb2a000000c3"), "cdecl() -> void")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("callee-saved ebx", r["mismatch"])

    def test_compiled_stack_balance_matches(self):
        r = self.run_pair(b"\xc3", bytes.fromhex("c20400"), "cdecl() -> void")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("stack balance", r["mismatch"])

    def test_live_caller_argument_write_is_compared(self):
        r = self.run_pair(bytes.fromhex("c74424042a000000c3"), b"\xc3", "cdecl(i32) -> void")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("caller-owned stack", r["mismatch"])

    def test_undeclared_stack_argument_is_detected(self):
        code = bytes.fromhex("8b442404c3")
        r = self.run_pair(code, code, "cdecl() -> u32")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("hidden input undeclared stack", r["mismatch"])

    def test_uncovered_branch_cannot_pass(self):
        # Declared input is constrained to zero, so only one of the branch's
        # two outcomes is reachable by this spec. Extra search cannot help.
        code = bytes.fromhex("8b44240485c07406b82a000000c3c3")
        r = self.run_pair(code, code, "cdecl(i32[0..0]) -> u32")
        self.assertEqual(r["status"], "inconclusive", r)
        self.assertEqual(r["coverage"]["branch_fraction"], 0.5)
        self.assertEqual(r["cases"], 4)

    def test_branch_to_next_instruction_counts_as_both_directions(self):
        # mov eax,[esp+4]; test eax,eax; je +0; ret -- taken and fallthrough are
        # the same address, so neither direction can be told apart or missed.
        code = bytes.fromhex("8b44240485c07400c3")
        r = self.run_pair(code, code, "cdecl(i32[0..0]) -> u32")
        self.assertEqual(r["status"], "pass", r)
        self.assertEqual(r["coverage"]["branch_fraction"], 1.0)

    def test_coverage_search_uses_branch_boundary_constants(self):
        code = bytes.fromhex("8b4424043d785634127406b82a000000c3c3")
        r = self.run_pair(code, code, "cdecl(u32) -> u32", settings={"max_cases": 80})
        self.assertEqual(r["status"], "pass", r)
        self.assertEqual(r["coverage"]["branch_fraction"], 1.0)
        self.assertGreater(r["cases"], 2)

    def test_callee_bodies_are_not_target_coverage(self):
        target = b"\xe8" + struct.pack("<i", 0x401100 - (TARGET + 5)) + b"\xc3"
        recon = b"\xe8" + struct.pack("<i", 0x401100 - (IMPL + 5)) + b"\xc3"
        # Callee has an unexercised branch; only its call site belongs to target.
        callee = bytes.fromhex("31c085c07406b82a000000c3c3")
        r = self.run_pair(target, recon, "cdecl() -> u32", data={0x401100: callee})
        self.assertEqual(r["status"], "pass", r)
        self.assertEqual(r["coverage"]["branch_directions_total"], 0)

    def test_initialization_failure_is_closed(self):
        r = self.run_pair(b"\xc3", b"\xc3", "cdecl() -> void", settings={"init_calls": [0xDEADBEEF]})
        self.assertEqual(r["status"], "fail", r)
        self.assertEqual(r["reason"], "initialization failed")

    def test_unrepresentable_extended_regs_output_is_inconclusive(self):
        # Exact x87 value 1 + 2^-63 rounds to binary64 1.0.
        data = {0x401300: struct.pack("<QH", 0x8000000000000001, 0x3FFF)}
        original = b"\xdb\x2d" + struct.pack("<I", 0x401300) + b"\xc3"
        r = self.run_pair(original, b"\xc3", "regs() -> (st0)", data=data)
        self.assertEqual(r["status"], "inconclusive", r)
        self.assertIn("cannot be represented exactly", r["reason"])

    def test_compiled_extended_difference_is_compared_before_rounding(self):
        data = {0x401300: struct.pack("<QH", 0x8000000000000001, 0x3FFF)}
        original = b"\xdb\x2d" + struct.pack("<I", 0x401300) + b"\xc3"
        r = self.run_pair(original, bytes.fromhex("d9e8c3"), "cdecl() -> f64", data=data)
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("return st0", r["mismatch"])

    def test_st_inputs_are_loaded_in_register_order(self):
        original = bytes.fromhex("dee9c3")  # fsubp st1,st0
        recon = bytes.fromhex("8b442404c7402000000000c7402400000040c3")
        r = self.run_pair(original, recon, "regs(st1:f64[5..5],st0:f64[3..3]) -> (st0)")
        self.assertEqual(r["status"], "pass", r)

    def test_any_original_fault_keeps_verification_inconclusive(self):
        # One declared scalar value faults; valid cases cannot hide it.
        code = bytes.fromhex("8b44240485c07406a1efbeaddec3c3")
        r = self.run_pair(code, code, "cdecl(i32[0..1]) -> u32", settings={"max_cases": 20})
        self.assertEqual(r["status"], "inconclusive", r)
        self.assertGreater(r["invalid"], 0)

    def test_out_of_scope_code_cannot_execute(self):
        target = b"\xe9" + struct.pack("<i", 0x401100 - (TARGET + 5))
        gp4, dll = images(target, b"\xc3", {0x401100: b"\xc3"})
        gp4.sections.append(Section("wrapper", 0x401100, 1, 1, 0x60000020, 0, False))
        cfg = {"init_calls": [INIT], "max_cases": 2, "time_budget_s": 2,
               "min_block_coverage": 0.9, "min_branch_coverage": 0.7, "ni_cases": 1}
        rec = Impl(TARGET, IMPL, "synthetic", "cdecl() -> void")
        with patch("gp4re.config.oracle", side_effect=cfg.__getitem__):
            r = differential(gp4, dll, rec, parse(rec.spec), fpucw=0x037F, cases=2,
                             function_starts={TARGET, INIT, 0x401100})
        self.assertEqual(r["status"], "inconclusive", r)
        self.assertEqual(r["invalid_reasons"], {"fault": 2})

    def test_undeclared_x87_output_is_rejected(self):
        r = self.run_pair(bytes.fromhex("d9eec3"), b"\xc3", "cdecl() -> void")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("x87 output contract", r["mismatch"])

    def test_memory_tolerance_cannot_hide_integer_changes(self):
        a, b = bytes(PAGE), b"\x01" + bytes(PAGE - 1)
        self.assertTrue(compare_pages({0: a}, {0: b}, lambda _: a, 1e-3))

    def test_tolerance_preserves_signed_zero_and_nonfinite_bits(self):
        for left, right in ((0.0, -0.0), (float("inf"), 1.0),
                            (float("inf"), float("-inf"))):
            self.assertFalse(_float_equal(struct.pack("<d", left), struct.pack("<d", right), 1e-3))

    def test_wide_integer_ranges_include_zero_and_sign_boundaries(self):
        argument = parse("cdecl(i32[-32767..32767]) -> void").args[0].t
        generator = CaseGen(parse("cdecl() -> void"), 7)
        values = {generator.scalar(argument) for _ in range(200)}
        self.assertTrue({-32767, -1, 0, 1, 32767}.issubset(values))
        self.assertTrue(all(-32767 <= value <= 32767 for value in values))

    def test_unicorn_shrd12_flags_follow_penultimate_shift(self):
        # Architecture leaves OF undefined for count>1. This checks the pinned
        # oracle's model, not equality of undefined bits on every native CPU.
        game, _ = images(bytes.fromhex("0facd00cc3"), b"\xc3")
        machine = Machine(game, None, fpucw=0x037F)
        rng = random.Random(52)
        pairs = [(0, 0), (0xFFFFFFFF, 0xFFFFFFFF), (0, 0x80000000),
                 (0x80000000, 0), (0, 0x1000), (0, 0x800)]
        pairs += [(rng.getrandbits(32), rng.getrandbits(32)) for _ in range(128)]
        for low, high in pairs:
            machine.reset()
            result = machine.call(TARGET, regs={"eax": low, "edx": high}, stack=b"", st_in=[],
                                  n_st_out=0, capture_flags=True, maxinsn=100)
            self.assertEqual(result.status, "ok")
            combined = (high << 32) | low
            output = (combined >> 12) & 0xFFFFFFFF
            previous = (combined >> 11) & 0xFFFFFFFF
            overflow = ((previous ^ output) >> 31) & 1
            self.assertEqual(result.regs["eax"], output)
            self.assertEqual((result.flags >> 11) & 1, overflow)

    def test_asymmetric_dirty_page_uses_loaded_input_baseline(self):
        loaded = b"\x05" + bytes(PAGE - 1)
        self.assertEqual(compare_pages({0: loaded}, {}, lambda _: loaded, 0), [])


if __name__ == "__main__":
    unittest.main()

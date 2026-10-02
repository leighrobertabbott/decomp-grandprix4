import json
import random
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

from gp4re import input_layouts
from gp4re.binary import Binary, Section


class GlobalInputLayoutTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.layout_dir = Path(self.directory.name)
        image = bytearray(b"\xaa" * 0x4000)
        sections = [Section(".data", 0x402000, 0x1000, 0x1000, 0xC0000040, 0, True),
                    Section(".rdata", 0x403000, 0x1000, 0x1000, 0x40000040, 0, True)]
        self.binary = Binary(Path("synthetic.exe"), b"", 0x400000, 0x401000, image, sections)
        self.generator = SimpleNamespace(scalar=lambda t: int(t.lo or 1))

    def generate(self, fields):
        (self.layout_dir / "example.json").write_text(json.dumps({"globals": fields}))
        with patch.object(input_layouts, "LAYOUT_DIR", self.layout_dir):
            return input_layouts.global_buffers(self.generator, "example", self.binary)

    def test_repeated_byte_fields_preserve_surrounding_bytes(self):
        before = bytes(self.binary.image)
        values = self.generate([{"addr": "0x402010", "type": "u8", "lo": 7, "hi": 7,
                                 "count": 3, "stride": "0x4"}])
        self.assertEqual(values, [(0x402010, b"\x07"), (0x402014, b"\x07"), (0x402018, b"\x07")])
        self.assertEqual(bytes(self.binary.image), before)

    def test_fields_must_fit_writable_in_scope_data(self):
        for address in ("0x403000", "0x402fff", "0xdeadbeef"):
            with self.assertRaises(ValueError):
                self.generate([{"addr": address, "type": "u32"}])

    def test_global_field_range_is_validated(self):
        with self.assertRaises(ValueError):
            self.generate([{"addr": "0x402000", "type": "u8", "lo": 0, "hi": 256}])

    def test_bit_scan_reaches_clear_and_each_hit_position(self):
        positions = set()
        for seed in range(400):
            self.generator = SimpleNamespace(rng=random.Random(seed), scalar=lambda t: 0xff)
            values = self.generate([{"addr": "0x402010", "type": "u8", "count": 5,
                                     "stride": 4, "pattern": "bit_scan", "bit": "0x80"}])
            bits = [data[0] for _, data in values]
            positions.add(next((i for i, value in enumerate(bits) if value & 0x80), -1))
            self.assertTrue(all(value & 0x7f == 0x7f for value in bits))
        self.assertEqual(positions, {-1, 0, 1, 2, 3, 4})

    def test_bit_scan_rejects_invalid_patterns(self):
        for changes in ({"type": "i8"}, {"count": 1}, {"bit": 3}, {"bit": 256},
                        {"lo": 0, "hi": 127}, {"pattern": "unknown"}):
            field = {"addr": "0x402000", "type": "u8", "count": 3,
                     "pattern": "bit_scan", "bit": 128, **changes}
            with self.assertRaises(ValueError):
                self.generate([field])

    def test_points_into_selects_record_aligned_addresses(self):
        seen = set()
        for seed in range(100):
            self.generator = SimpleNamespace(rng=random.Random(seed), scalar=lambda t: 0)
            values = self.generate([{"addr": "0x402010", "type": "u32",
                                     "points_into": {"base": "0x402100", "stride": "0x20", "lo": 2, "hi": 5}}])
            seen.add(int.from_bytes(values[0][1], "little"))
        self.assertEqual(seen, {0x402100 + 0x20 * k for k in range(2, 6)})

    def test_points_into_rejects_invalid_fields(self):
        base = {"base": 0x402100, "stride": 0x20, "lo": 2, "hi": 5}
        for field in ({"type": "u16"}, {"count": 2}, {"lo": 0, "hi": 1},
                      {"points_into": {**base, "hi": 1}}, {"points_into": {**base, "stride": 0}}):
            full = {"addr": "0x402010", "type": "u32", "points_into": base, **field}
            with self.assertRaises(ValueError):
                self.generate([full])

    def test_alias_is_equal_in_some_cases_and_independent_in_others(self):
        equal = differ = 0
        for seed in range(100):
            self.generator = SimpleNamespace(rng=random.Random(seed),
                                             scalar=lambda t, r=random.Random(seed): r.randrange(1 << 32))
            values = dict(self.generate([{"addr": "0x402010", "type": "u32"},
                                         {"addr": "0x402020", "type": "u32", "alias": "0x402010"}]))
            equal += values[0x402010] == values[0x402020]
            differ += values[0x402010] != values[0x402020]
        self.assertGreater(equal, 20)
        self.assertGreater(differ, 20)

    def test_values_pick_from_the_list_and_validate_fit(self):
        self.generator = SimpleNamespace(rng=random.Random(1), scalar=lambda t: 0)
        seen = {int.from_bytes(d, "little") for _, d in self.generate(
            [{"addr": "0x402010", "type": "u16", "count": 64, "stride": 2, "values": [0xffff, "0x41c", 0]}])}
        self.assertEqual(seen, {0xffff, 0x41c, 0})
        for field in ({"type": "u8", "values": [256]}, {"type": "u8", "values": []},
                      {"type": "u8", "values": [1], "lo": 0, "hi": 1}, {"type": "f32", "values": [1]}):
            with self.assertRaises(ValueError):
                self.generate([{"addr": "0x402010", **field}])

    def test_ptr_bias_and_nullable(self):
        allocated = []
        def alloc(data):
            allocated.append(len(data))
            return 0x500000 + 0x10000 * len(allocated)
        seen = set()
        for seed in range(60):
            allocated.clear()
            self.generator = SimpleNamespace(rng=random.Random(seed), alloc=alloc, fill=lambda n, f: bytes(n),
                                             scalar=lambda t: 0)
            data = self.generate([{"addr": "0x402010", "type": "ptr", "size": 0x2c4, "fill": "bytes",
                                   "bias": "0x15e", "nullable": True}])[0][1]
            seen.add(int.from_bytes(data, "little"))
        self.assertEqual(seen, {0, 0x510000 + 0x15e})
        for field in ({"type": "u32", "bias": 4}, {"type": "u32", "nullable": True},
                      {"type": "ptr", "size": 16, "fill": "bytes", "bias": 16}):
            with self.assertRaises(ValueError):
                self.generate([{"addr": "0x402010", **field}])

    def test_alias_needs_an_earlier_field_of_the_same_width(self):
        for fields in ([{"addr": "0x402020", "type": "u32", "alias": "0x402010"}],
                       [{"addr": "0x402010", "type": "u16"},
                        {"addr": "0x402020", "type": "u32", "alias": "0x402010"}]):
            with self.assertRaises(ValueError):
                self.generate(fields)


if __name__ == "__main__":
    unittest.main()

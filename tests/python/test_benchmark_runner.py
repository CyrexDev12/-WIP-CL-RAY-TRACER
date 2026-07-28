from __future__ import annotations

import unittest

from tools.run_benchmark import BENCHMARK_PROMPTS


class BenchmarkRunnerTests(unittest.TestCase):
    def test_controlled_suite_has_six_named_unique_prompts(self) -> None:
        self.assertEqual(len(BENCHMARK_PROMPTS), 6)
        self.assertEqual([item[0] for item in BENCHMARK_PROMPTS], ["One", "Two", "Three", "Four", "Five", "Six"])
        self.assertEqual(len({item[1] for item in BENCHMARK_PROMPTS}), 6)
        self.assertTrue(all(len(item[2]) > 250 for item in BENCHMARK_PROMPTS))

    def test_prompts_include_required_control_features(self) -> None:
        joined = "\n".join(prompt for _, _, prompt in BENCHMARK_PROMPTS)
        for phrase in (
            "black-and-white checkered floor",
            "five planets",
            "five spheres",
            "two rows of tall closed cylinders",
            "three nested groups",
            "three enormous glossy spheres",
        ):
            self.assertIn(phrase, joined)


if __name__ == "__main__":
    unittest.main()

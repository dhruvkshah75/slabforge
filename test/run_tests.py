#!/usr/bin/env python3
"""Build and run selected Slabforge test suites."""

import argparse
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parent.parent
TESTS = {"basic": "build/test_basic"}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--test", choices=["all", *TESTS], default="all",
        help="test suite to run (default: all)",
    )
    args = parser.parse_args()
    selected = TESTS if args.test == "all" else {args.test: TESTS[args.test]}

    for name, target in selected.items():
        print(f"Building and running {name} tests...", flush=True)
        try:
            subprocess.run(["make", target], cwd=ROOT, check=True)
            subprocess.run([str(ROOT / target)], cwd=ROOT, check=True)
        except subprocess.CalledProcessError as error:
            print(f"FAIL: {name} (exit code {error.returncode})", file=sys.stderr)
            return 1
        print(f"PASS: {name}", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())

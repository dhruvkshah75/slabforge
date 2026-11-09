# Running tests

Run these commands from the project root. You need Python 3, Make, a C11 compiler, and support for AddressSanitizer and UndefinedBehaviorSanitizer.

```sh
make test                  # Build and run all available suites
make test TEST_NAME=basic  # Build and run only the basic suite
```

`basic` is currently the only suite. It checks bump allocation, alignment, data preservation, zero-size requests, oversized requests, and the current no-op `my_free` behavior. The runner builds the test before executing it and stops if the build or test fails.

You can also invoke the runner directly:

```sh
python3 test/run_tests.py --test all
python3 test/run_tests.py --test basic
python3 test/run_tests.py --help
```

To remove generated build files, run `make clean` from the project root. The next test run will rebuild them.

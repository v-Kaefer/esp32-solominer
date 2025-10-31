# Test Automation Scripts

This directory contains scripts for automated test detection and generation.

## Scripts

### detect_features.py

Analyzes the codebase to detect functions and identify which ones lack unit tests.

**Usage:**
```bash
python3 scripts/detect_features.py .
```

**Output:**
- Lists all functions found in source files
- Identifies functions with and without tests
- Generates test stubs for untested functions
- Returns exit code 1 if untested functions are found (useful for CI/CD)

**Example:**
```bash
$ python3 scripts/detect_features.py .
================================================================================
Feature Implementation Detector
================================================================================
Analyzing project: .
Source directory: ./main
Test directory: ./test

Total functions found: 11
Functions with tests: 7
Functions without tests: 4

⚠️  Functions without tests:
--------------------------------------------------------------------------------
  • ssd1306_clear_screen           (void)
    Location: main/ssd1306.c:142
...
```

### auto_generate_tests.py

Automatically generates comprehensive test files for functions that don't have tests.

**Usage:**
```bash
python3 scripts/auto_generate_tests.py .
```

**Output:**
- Creates `test_<module>_auto.c` files in the `test/` directory
- Generates multiple test cases per function:
  - Valid input tests
  - Edge case tests
  - NULL parameter tests (for pointer parameters)
  - Return value tests (for non-void functions)

**Example:**
```bash
$ python3 scripts/auto_generate_tests.py .
================================================================================
Auto Test Generator
================================================================================
Analyzing project: .

Found 4 untested functions

Generating ./test/test_ssd1306_auto.c with 4 test functions...
  ✅ Generated ./test/test_ssd1306_auto.c

================================================================================
Test generation complete!

Next steps:
1. Review generated test files and implement the TODO items
2. Add necessary #include directives
3. Update test/CMakeLists.txt to include new test files
4. Build and run tests with: idf.py build
```

## Workflow Integration

These scripts are integrated into the CI/CD pipeline via `.github/workflows/test-coverage.yml`:

1. On every PR with C file changes, `detect_features.py` runs automatically
2. A test coverage report is generated and posted as a PR comment
3. If functions lack tests, the workflow provides test stubs and guidance

## Adding New Functions

When adding new functions to the codebase:

1. Write the function in the appropriate source file
2. Run `detect_features.py` to identify the new function
3. Either:
   - Manually write tests following the existing patterns, or
   - Run `auto_generate_tests.py` to generate test stubs
4. Implement the test logic in the generated stubs
5. Update `test/CMakeLists.txt` if new test files were created
6. Run `detect_features.py` again to verify coverage

## Best Practices

- **Test naming:** Use the pattern `test_<function_name>_<scenario>`
- **Test structure:** Follow the Arrange-Act-Assert pattern
- **Edge cases:** Always test boundary conditions and error cases
- **NULL safety:** Test behavior with NULL pointers where applicable
- **Return values:** Verify return values for non-void functions

## CI/CD Integration

The `test-coverage.yml` workflow will:
- Run on all PRs that modify C/H files
- Analyze test coverage using `detect_features.py`
- Post a report as a PR comment
- Provide test stubs for missing tests
- Guide contributors on adding tests

This ensures that all new code contributions include appropriate unit tests.

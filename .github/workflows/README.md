# GitHub Actions Workflows

This directory contains the CI/CD workflows for the ESP32 Solo Miner project.

## Workflows Overview

### 1. Build and Security Analysis (`build-and-analyze.yml`)
**Triggers:** Push to main, Pull requests to main, Weekly schedule (Mondays)
**Purpose:** Compiles the ESP32-S3 project and performs CodeQL security analysis

**What it does:**
- Sets up ESP-IDF environment with caching for faster runs
- Initializes CodeQL for security scanning
- Creates config.h from example template
- Builds the project for ESP32-S3 target
- Archives build artifacts (binaries, ELF, map files)
- Performs security-focused code analysis
- Detects common vulnerabilities (buffer overflows, etc.)
- Creates security alerts in GitHub Security tab

### 2. Auto Tag Firmware Releases (`auto-tag.yml`)
**Triggers:** Push to main/develop/release, Pull requests, Manual dispatch
**Purpose:** Automatically creates version tags for firmware releases

**What it does:**
- Auto-creates tags from VERSION file on push
- Allows manual tag creation via workflow dispatch
- Validates tag uniqueness
- Pushes tags to repository

### 3. PR Labeler (`labeler.yml`)
**Triggers:** Pull requests (opened, synchronized, reopened)
**Purpose:** Automatically labels pull requests based on changed files and branch names

**What it does:**
- Labels PRs based on file changes (firmware, mining, display, networking, etc.)
- Labels PRs based on branch prefixes (feature/, fix/, hotfix/, etc.)
- Syncs label colors with .github/labels.json
- Provides labeling summary

### 4. Static Analysis (`static-analysis.yml`)
**Triggers:** Push to main, Pull requests to main
**Purpose:** Runs cppcheck to detect potential code issues

**What it does:**
- Analyzes C source code for common bugs
- Checks for memory leaks, undefined behavior
- Identifies unused functions and variables
- Generates detailed analysis report

### 5. Code Quality Checks (`code-quality.yml`)
**Triggers:** Push to main, Pull requests to main
**Purpose:** Enforces code quality standards

**What it does:**
- Checks for trailing whitespace
- Validates file encodings (UTF-8/ASCII)
- Detects hardcoded credentials (excluding example files)
- Identifies unsafe C functions (gets, strcpy, etc.)
- Finds TODO/FIXME comments
- Checks for overly long functions (>200 lines)

### 6. Test Coverage Check (`test-coverage.yml`)
**Triggers:** Pull requests that modify C/H files
**Purpose:** Monitors test coverage and suggests tests for new features

**What it does:**
- Identifies changed C/H files
- Extracts function definitions from changes
- Checks if test directory exists
- Generates test coverage report
- Comments on PR with recommendations
- Suggests unit tests for mining functions, initialization routines

### 7. Documentation Check (`documentation.yml`)
**Triggers:** Pull requests to main
**Purpose:** Ensures documentation stays current

**What it does:**
- Checks if documentation was updated alongside code changes
- Validates README.md completeness (Configuration, Build, Usage, License sections)
- Analyzes comment density in source files
- Provides warnings if docs are missing

## Best Practices

### For Contributors:
1. Ensure all workflows pass before requesting review
2. Add tests for new features when possible
3. Update documentation when making user-facing changes
4. Fix any security issues identified by CodeQL
5. Address cppcheck warnings for new code

### For Maintainers:
1. Review CodeQL alerts in the Security tab weekly
2. Keep ESP-IDF version updated in workflows
3. Monitor workflow run times and optimize if needed
4. Adjust cppcheck suppressions as needed

## Local Testing

Before pushing, you can run some checks locally:

```bash
# Build the project
idf.py set-target esp32s3
idf.py build

# Run cppcheck
cppcheck --enable=all --suppress=missingIncludeSystem main/*.c

# Check for trailing whitespace
grep -rn --include="*.c" --include="*.h" '[[:space:]]$' main/
```

## Workflow Artifacts

Workflows generate artifacts that can be downloaded from the Actions tab:
- **build-artifacts**: Compiled binaries and map files (30 days retention)
- **cppcheck-report**: Static analysis results (30 days retention)
- **test-coverage-report**: Test suggestions (30 days retention)

## Troubleshooting

### Build and Security Analysis Workflow Fails
- Check if config.h.example exists and is valid
- Verify ESP-IDF version compatibility
- Check CMakeLists.txt configuration
- Review CodeQL logs for compilation errors
- Check CodeQL alerts in Security tab

### Static Analysis False Positives
- Add inline suppressions: `// cppcheck-suppress <warning-id>`
- Update suppressions in static-analysis.yml if needed

## Future Enhancements

Potential improvements to consider:
- Add hardware-in-the-loop testing with actual ESP32 devices
- Implement unit testing framework (Unity/CMock)
- Add code coverage reporting (gcov/lcov)
- Create assembly optimization validation
- Add performance benchmarking workflow

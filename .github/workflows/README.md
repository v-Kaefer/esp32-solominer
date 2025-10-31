# GitHub Actions Workflows

This directory contains the CI/CD workflows for the ESP32 Solo Miner project.

## Workflow Architecture

The repository uses a modular, reusable workflow architecture to reduce duplication and ensure consistency.

## Workflows Overview

### Reusable Workflows (Modular Components)

These workflows are designed to be called by other workflows:

#### `reusable-build.yml`
**Purpose:** Builds the ESP32 project using ESP-IDF
**Inputs:**
- `esp_idf_version`: ESP-IDF version (default: v5.1.2)
- `target`: ESP32 target (default: esp32s3)
- `artifact_retention_days`: Artifact retention period (default: 30)

**What it does:**
- Sets up ESP-IDF environment
- Creates config.h from example template
- Builds the project for ESP32-S3 target
- Archives build artifacts (binaries, ELF, map files)

#### `reusable-static-analysis.yml`
**Purpose:** Runs cppcheck static analysis on C code
**Inputs:**
- `artifact_retention_days`: Artifact retention period (default: 30)

**What it does:**
- Analyzes C source code for common bugs
- Checks for memory leaks, undefined behavior
- Identifies unused functions and variables
- Generates detailed analysis report

#### `reusable-code-quality.yml`
**Purpose:** Performs comprehensive code quality checks
**Inputs:**
- `GITHUB_TOKEN`: Required secret for creating issues

**What it does:**
- Checks for trailing whitespace
- Validates file encodings (UTF-8/ASCII)
- Detects hardcoded credentials (excluding example files)
- Identifies unsafe C functions (gets, strcpy, etc.)
- Finds TODO/FIXME comments
- Checks for overly long functions (>200 lines)
- **Automatically creates GitHub issues on PR failures with @github/copilot assistance**

#### `reusable-test-coverage.yml`
**Purpose:** Analyzes test coverage for changed files
**Inputs:**
- `artifact_retention_days`: Artifact retention period (default: 30)

**What it does:**
- Identifies changed C/H files
- Extracts function definitions from changes
- Checks if test directory exists
- Generates test coverage report
- Comments on PR with recommendations

### Caller Workflows (Orchestrators)

These workflows coordinate multiple checks by calling reusable workflows:

#### `pr-checks.yml`
**Triggers:** Pull requests to main/release branches
**Purpose:** Orchestrates all checks for pull requests

**Jobs:**
- Build verification (calls `reusable-build.yml`)
- Static code analysis (calls `reusable-static-analysis.yml`)
- Code quality checks (calls `reusable-code-quality.yml`)
- Test coverage analysis (calls `reusable-test-coverage.yml`)

#### `main-release-checks.yml`
**Triggers:** Push to main/release branches
**Purpose:** Runs essential checks on direct pushes

**Jobs:**
- Build verification (calls `reusable-build.yml`)
- Static code analysis (calls `reusable-static-analysis.yml`)

### Independent Workflows

These workflows have specific requirements and run independently:

#### `codeql.yml`
**Triggers:** Push to main, Pull requests to main, Weekly schedule (Mondays)
**Purpose:** GitHub CodeQL security analysis with custom build process

**What it does:**
- Performs security-focused code analysis
- Detects common vulnerabilities (buffer overflows, injection patterns, etc.)
- Runs security and quality queries
- Creates security alerts in GitHub Security tab

#### `documentation.yml`
**Triggers:** Pull requests to main/develop
**Purpose:** Ensures documentation stays current

**What it does:**
- Checks if documentation was updated alongside code changes
- Validates README.md completeness (Configuration, Build, Usage, License sections)
- Analyzes comment density in source files
- Provides warnings if docs are missing

#### `auto-tag.yml`
**Triggers:** Push to main/release/develop branches, Manual workflow dispatch
**Purpose:** Automatically creates version tags for firmware releases

**What it does:**
- Reads VERSION file
- Creates tags with appropriate suffix based on branch (alpha/beta/release)
- Prevents duplicate tags
- Supports manual version tagging

#### `labeler.yml`
**Triggers:** PR opened, synchronized, reopened
**Purpose:** Automatically labels PRs based on changed files and branch patterns

**What it does:**
- Labels PRs based on file paths changed
- Syncs label colors from configuration
- Provides PR summary

### Deprecated Workflows

These workflows are kept for backward compatibility and only trigger on `develop` branch or manual dispatch:

- `build.yml` - Replaced by `pr-checks.yml` and `main-release-checks.yml`
- `static-analysis.yml` - Replaced by `pr-checks.yml` and `main-release-checks.yml`
- `code-quality.yml` - Replaced by `pr-checks.yml`
- `test-coverage.yml` - Replaced by `pr-checks.yml`

## Benefits of the Modular Architecture

✅ **Consistency**: Same checks run regardless of branch or trigger
✅ **Maintainability**: Update once, apply everywhere
✅ **Flexibility**: Easy to add/remove checks or customize per branch
✅ **Reusability**: Workflows can be called from different contexts
✅ **Reduced Duplication**: Single source of truth for each check

## Usage Examples

### Calling a Reusable Workflow

```yaml
jobs:
  build:
    uses: ./.github/workflows/reusable-build.yml
    with:
      esp_idf_version: v5.1.2
      target: esp32s3
      artifact_retention_days: 30
```

### Adding a New Check to PRs

1. Create a reusable workflow in `.github/workflows/reusable-<name>.yml`
2. Add a job to `pr-checks.yml` that calls your reusable workflow
3. Test with a PR to ensure it works correctly

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

### Build Workflow Fails
- Check if config.h.example exists and is valid
- Verify ESP-IDF version compatibility
- Check CMakeLists.txt configuration

### CodeQL Analysis Fails
- May need to adjust build commands if project structure changes
- Check CodeQL logs for compilation errors

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

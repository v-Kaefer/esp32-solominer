# GitHub Actions Workflows

This directory contains the CI/CD workflows for the ESP32 Solo Miner project.

## Workflows Overview

### 1. Branch Protection (`branch-protection.yml`)
**Triggers:** Pull requests to main/release (opened, synchronize, reopened, labeled, unlabeled)
**Purpose:** Enforces branch protection rules to maintain code quality and stability

**What it does:**
- Validates that PRs to `release` branch only come from `main` branch
- Validates that PRs to `main` branch only come from `develop` branch OR branches with `🚑 hotfix` label
- Posts comments on PRs indicating pass/fail status with helpful guidance
- Fails the workflow if branch protection rules are violated

**Branch Rules:**
- `release` ← `main` only
- `main` ← `develop` OR hotfix-labeled branches

### 2. Build Workflow (`build.yml`)
**Triggers:** Push to main/release, Pull requests to main/release, workflow_call
**Purpose:** Compiles the ESP32-S3 project using ESP-IDF v5.1.2

**What it does:**
- Sets up ESP-IDF environment
- Creates config.h from example template
- Builds the project for ESP32-S3 target
- Archives build artifacts (binaries, ELF, map files)

**Reusable:** Can be called by other workflows with configurable ESP-IDF version, target, and artifact retention.

### 3. Static Analysis (`static-analysis.yml`)
**Triggers:** Push to main/release, Pull requests to main/release, workflow_call
**Purpose:** Runs cppcheck to detect potential code issues

**What it does:**
- Analyzes C source code for common bugs
- Checks for memory leaks, undefined behavior
- Identifies unused functions and variables
- Generates detailed analysis report

**Reusable:** Can be called by other workflows with configurable artifact retention.

### 4. CodeQL Security Analysis (`codeql.yml`)
**Triggers:** Push to main, Pull requests to main, Weekly schedule (Mondays)
**Purpose:** Scans for security vulnerabilities

**What it does:**
- Performs security-focused code analysis
- Detects common vulnerabilities (buffer overflows, SQL injection patterns, etc.)
- Runs security and quality queries
- Creates security alerts in GitHub Security tab

### 5. Code Quality Checks (`code-quality.yml`)
**Triggers:** Push to main/release, Pull requests to main/release, workflow_call
**Purpose:** Enforces code quality standards

**What it does:**
- Checks for trailing whitespace
- Validates file encodings (UTF-8/ASCII)
- Detects hardcoded credentials (excluding example files)
- Identifies unsafe C functions (gets, strcpy, etc.)
- Finds TODO/FIXME comments
- Checks for overly long functions (>200 lines)
- Automatically creates GitHub issues on PR failures with assistance prompts

**Reusable:** Can be called by other workflows (requires GITHUB_TOKEN secret).

### 6. Test Coverage Check (`test-coverage.yml`)
**Triggers:** Pull requests that modify C/H files, workflow_call
**Purpose:** Monitors test coverage and suggests tests for new features

**What it does:**
- Identifies changed C/H files
- Extracts function definitions from changes
- Checks if test directory exists
- Generates test coverage report
- Comments on PR with recommendations
- Suggests unit tests for mining functions, initialization routines

**Reusable:** Can be called by other workflows with configurable artifact retention.

### 7. Documentation Check (`documentation.yml`)
**Triggers:** Pull requests to main
**Purpose:** Ensures documentation stays current

**What it does:**
- Checks if documentation was updated alongside code changes
- Validates README.md completeness (Configuration, Build, Usage, License sections)
- Analyzes comment density in source files
- Provides warnings if docs are missing

### 8. Sync Copilot Instructions (`sync-copilot-instructions.yml`)
**Triggers:** Push to main (when instruction files change), Pull requests (when instruction files change), Manual dispatch
**Purpose:** Keeps Copilot instructions synchronized across different contexts

**What it does:**
- Monitors changes to `.github/copilot-instructions.md` (for IDE) and `.github/agents/CONTEXT.md` (for Workspace)
- Automatically syncs content from the most recently modified file to the other
- Commits changes to main branch automatically (with `[skip ci]` to avoid loops)
- Comments on PRs to warn about automatic syncing after merge
- Ensures both IDE and Workspace agents have consistent project context

**Why it's needed:**
- GitHub Copilot IDE extension reads from `.github/copilot-instructions.md`
- GitHub Copilot Workspace agents read from `.github/agents/CONTEXT.md`
- This workflow ensures both files stay in sync for consistent AI assistance

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

## Reusable Workflows

Several workflows support `workflow_call` trigger, allowing them to be called by other workflows for consistency and reusability:

- **build.yml** - Accepts parameters for ESP-IDF version, target, and artifact retention
- **static-analysis.yml** - Accepts parameter for artifact retention
- **code-quality.yml** - Requires GITHUB_TOKEN secret
- **test-coverage.yml** - Accepts parameter for artifact retention

### Example: Calling a workflow

```yaml
jobs:
  build:
    uses: ./.github/workflows/build.yml
    with:
      esp_idf_version: v5.1.2
      target: esp32s3
      artifact_retention_days: 30
```

This design allows workflows like `pr-checks.yml` and `main-release-checks.yml` to orchestrate multiple checks while keeping workflow definitions DRY (Don't Repeat Yourself).

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

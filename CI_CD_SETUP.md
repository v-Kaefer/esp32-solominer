# CI/CD Setup

This repository uses GitHub Actions for continuous integration and continuous deployment (CI/CD). All workflows are configured to ensure code quality, security, and maintainability.

## 🚀 Quick Start

When you create a pull request, the following checks will run automatically:
1. ✅ **Build Check** - Compiles your code for ESP32-S3
2. 🔍 **Static Analysis** - Checks for common bugs and code issues
3. 🔒 **Security Scan** - Analyzes code for vulnerabilities
4. 📝 **Code Quality** - Validates formatting and best practices (auto-creates issues with fixes if failures occur)
5. 🧪 **Test Coverage** - Suggests tests for new features
6. 📚 **Documentation** - Checks if docs need updating

💡 **New:** Code quality failures automatically create GitHub issues with detailed fix suggestions and @github/copilot assistance!

## 📊 Workflow Status

All workflow statuses can be viewed in the [Actions tab](../../actions).

## 🛠️ Workflows Overview

### Modular Workflow Architecture

The CI/CD system now uses a modular, reusable workflow architecture that reduces duplication and ensures consistency across different branches:

#### **PR Checks Workflow** (`pr-checks.yml`)
Orchestrates all checks for pull requests to `main` and `release` branches:
- ✅ Build verification (ESP32-S3)
- 🔍 Static code analysis (cppcheck)
- 📝 Code quality checks
- 🧪 Test coverage analysis

#### **Main/Release Branch Checks** (`main-release-checks.yml`)
Runs on direct pushes to `main` and `release` branches:
- ✅ Build verification
- 🔍 Static code analysis

#### **Reusable Workflow Components**
The system uses modular, reusable workflows that can be called by different workflows:
- `reusable-build.yml` - ESP-IDF build process
- `reusable-static-analysis.yml` - cppcheck analysis
- `reusable-code-quality.yml` - Code quality checks with issue creation
- `reusable-test-coverage.yml` - Test coverage analysis

### Individual Workflow Details

### Build Workflow
Compiles the project using ESP-IDF v5.1.2 for ESP32-S3 target.
- Runs on: PRs to main/release, Push to main/release
- Duration: ~5-10 minutes
- Artifacts: Build binaries (30 days)

### Static Analysis
Uses cppcheck to find potential issues in C code.
- Runs on: PRs to main/release, Push to main/release
- Duration: ~1-2 minutes
- Artifacts: Analysis report (30 days)

### CodeQL Security
GitHub's security analysis tool for vulnerability detection.
- Runs on: Push to main, Pull requests, Weekly (Monday)
- Duration: ~10-15 minutes
- Results: GitHub Security tab
- Note: This workflow has specific build requirements and runs independently

### Code Quality
Checks for code standards and best practices.
- Runs on: PRs to main/release
- Duration: ~1 minute
- Checks: Whitespace, encodings, unsafe functions
- **Auto-Fix:** Creates GitHub issues with fix suggestions when checks fail on PRs
- **Copilot Integration:** Automatically mentions @github/copilot for AI-assisted fixes

### Test Coverage
Monitors code changes and suggests tests.
- Runs on: PRs to main/release
- Duration: ~1 minute
- Artifacts: Coverage report (30 days)

### Documentation
Ensures documentation stays current.
- Runs on: Pull requests to main/develop
- Duration: ~1 minute
- Checks: README completeness, comment density
- Note: This workflow runs independently for documentation-specific checks

### Auto Tag
Automatically creates version tags for firmware releases.
- Runs on: Push to main/release/develop branches
- Note: This workflow runs independently for tagging purposes

### PR Labeler
Automatically labels pull requests based on changed files and branch patterns.
- Runs on: Pull request opened, synchronized, reopened
- Note: This workflow runs independently for PR management

## 📋 Contributing Guidelines

1. **Before Creating PR:**
   - Build locally: `idf.py build`
   - Run cppcheck if available
   - Update documentation if needed

2. **During PR Review:**
   - Fix all workflow failures
   - Address reviewer comments
   - Keep commits clean and focused

3. **Merging:**
   - All workflows must pass ✅
   - At least one approval required
   - No merge conflicts

## 🔧 Troubleshooting

### Build Fails
- Ensure config.h.example is present
- Check CMakeLists.txt syntax
- Verify ESP-IDF compatibility

### Static Analysis Issues
- Review cppcheck report artifact
- Add suppressions for false positives
- Fix legitimate issues

### CodeQL Alerts
- Check Security tab for details
- Prioritize high-severity issues
- Add fixes in separate commits

### Code Quality Issues
- **Automated Issue Creation:** When code quality checks fail on a PR, an issue is automatically created
- **Issue Contents:** Detailed explanation of what needs fixing with file/line references
- **Copilot Assistance:** Tagged issues include @github/copilot mention for AI-assisted fixes
- **Duplicate Prevention:** Only one issue per PR is created; subsequent failures add comments
- **Resolution:** Fix the issues listed, then close the automated issue once checks pass

## 🏗️ Workflow Architecture

### Reusable Workflows
The repository uses modular, reusable workflows to reduce duplication and improve maintainability:

- **`reusable-build.yml`**: Handles ESP-IDF build process with configurable parameters
- **`reusable-static-analysis.yml`**: Runs cppcheck with consistent settings
- **`reusable-code-quality.yml`**: Performs code quality checks and creates issues on failures
- **`reusable-test-coverage.yml`**: Analyzes test coverage for changed files

### Caller Workflows
Main workflows that orchestrate the checks:

- **`pr-checks.yml`**: Runs all checks for PRs to main/release branches
- **`main-release-checks.yml`**: Runs essential checks on direct pushes to main/release

### Benefits of Modular Architecture
- ✅ **Consistency**: Same checks run regardless of branch or trigger
- ✅ **Maintainability**: Update once, apply everywhere
- ✅ **Flexibility**: Easy to add/remove checks or customize per branch
- ✅ **Reusability**: Workflows can be called from different contexts

## 📖 Documentation

For detailed information about each workflow, see [.github/workflows/README.md](.github/workflows/README.md).

## 🎯 Goals

The CI/CD setup aims to:
- Catch bugs early in development
- Ensure code compiles on all commits
- Maintain security standards
- Encourage test-driven development
- Keep documentation current
- Maintain code quality consistency

## 🔮 Future Enhancements

Planned improvements:
- Hardware-in-the-loop testing
- Unit test framework (Unity)
- Code coverage reporting (gcov)
- Assembly optimization validation
- Performance benchmarking

---

**Note:** This is a learning project focused on C and Assembly for ESP32. All workflows are designed to support educational goals while maintaining production-quality standards.

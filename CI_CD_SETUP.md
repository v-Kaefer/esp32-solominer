# CI/CD Setup

This repository uses GitHub Actions for continuous integration and continuous deployment (CI/CD). All workflows are configured to ensure code quality, security, and maintainability.

## 🚀 Quick Start

When you create a pull request, the following checks will run automatically:
1. ✅ **Build Check** - Compiles your code for ESP32-S3
2. 🔍 **Static Analysis** - Checks for common bugs and code issues
3. 🔒 **Security Scan** - Analyzes code for vulnerabilities
4. 📝 **Code Quality** - Validates formatting and best practices
5. 🧪 **Test Coverage** - Suggests tests for new features
6. 📚 **Documentation** - Checks if docs need updating

## 📊 Workflow Status

All workflow statuses can be viewed in the [Actions tab](../../actions).

## 🛠️ Workflows Overview

### Build Workflow
Compiles the project using ESP-IDF v5.1.2 for ESP32-S3 target.
- Runs on: Push to main, Pull requests
- Duration: ~5-10 minutes
- Artifacts: Build binaries (30 days)

### Static Analysis
Uses cppcheck to find potential issues in C code.
- Runs on: Push to main, Pull requests
- Duration: ~1-2 minutes
- Artifacts: Analysis report (30 days)

### CodeQL Security
GitHub's security analysis tool for vulnerability detection.
- Runs on: Push to main, Pull requests, Weekly (Monday)
- Duration: ~10-15 minutes
- Results: GitHub Security tab

### Code Quality
Checks for code standards and best practices.
- Runs on: Push to main, Pull requests
- Duration: ~1 minute
- Checks: Whitespace, encodings, unsafe functions

### Test Coverage
Monitors code changes and suggests tests.
- Runs on: Pull requests with C/H changes
- Duration: ~1 minute
- Artifacts: Coverage report (30 days)

### Documentation
Ensures documentation stays current.
- Runs on: Pull requests
- Duration: ~1 minute
- Checks: README completeness, comment density

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

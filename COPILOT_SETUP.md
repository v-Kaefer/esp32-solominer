# GitHub Copilot Instructions Setup

## Problem Identified

The repository had comprehensive context documentation in `.github/agents/CONTEXT.md`, which is excellent for other AI agents (like GitHub Copilot Workspace agents), but **GitHub Copilot (the IDE extension) does NOT read from this location**.

## Solution Implemented

Created `.github/copilot-instructions.md` - the file that GitHub Copilot specifically looks for when providing code suggestions in your IDE.

## File Location

```
.github/
├── agents/
│   └── CONTEXT.md              # For GitHub Copilot Workspace and other AI agents
└── copilot-instructions.md     # For GitHub Copilot IDE extension ✅ NEW
```

## What's Included in the Instructions

The `.github/copilot-instructions.md` file includes:

1. **Project Context** - ESP32-S3 Bitcoin mining, educational focus
2. **Security Guidelines** - WiFi credentials handling, config.h pattern
3. **Thermal Management** - Cooling requirements, temperature guidelines
4. **Performance Optimization** - SHA-256 tips, IRAM usage, dual-core patterns
5. **Hardware Configuration** - Default pin assignments
6. **Code Style Patterns** - ESP-IDF conventions, error handling
7. **Common Pitfalls** - What to avoid when suggesting code
8. **Reference Documentation** - Links to comprehensive guides

## How to Verify It's Working

### Method 1: Check File Exists
```bash
ls -la .github/copilot-instructions.md
```

### Method 2: Test in VSCode with Copilot

1. Open a C file in the repository (e.g., `main/main.c`)
2. Start typing a comment or code related to mining
3. GitHub Copilot should now:
   - Understand the ESP32-S3 context
   - Suggest code that follows the config.h pattern for WiFi
   - Consider thermal implications
   - Use ESP-IDF logging patterns
   - Reference correct pin numbers

### Method 3: Ask Copilot Chat

In VSCode with Copilot Chat:
```
@workspace What are the thermal requirements for dual-core mining?
```

Copilot should now reference the cooling requirements from the instructions.

## Key Differences

| Feature | .github/agents/CONTEXT.md | .github/copilot-instructions.md |
|---------|---------------------------|----------------------------------|
| **Purpose** | For GitHub Copilot Workspace agents | For GitHub Copilot IDE extension |
| **Read by** | Workspace AI agents | Copilot in VSCode/IDE |
| **Format** | Comprehensive agent context | Concise coding guidelines |
| **Usage** | PR generation, issue analysis | Code suggestions, completions |

## Both Files Are Important!

- **Keep both files** - they serve different purposes
- `.github/agents/CONTEXT.md` - For Copilot Workspace and issue/PR agents
- `.github/copilot-instructions.md` - For Copilot in your IDE while coding

## Automatic Synchronization

A GitHub Action (`.github/workflows/sync-copilot-instructions.yml`) automatically keeps both files synchronized:

- **When you update either file**, the workflow detects the changes
- **On main branch**: A PR is automatically created with the synced changes for review
- **In pull requests**: You'll receive a comment warning about sync needed after merge
- **Sync direction**: Most recently modified file → other file

This ensures both IDE and Workspace contexts stay consistent while maintaining proper review process.

## Testing Your Setup

Try asking Copilot to generate code for:

1. **WiFi Configuration** - Should suggest using config.h pattern, not hardcoding
2. **Dual-Core Mining** - Should mention cooling requirements
3. **SHA-256 Optimization** - Should suggest IRAM_ATTR and proper techniques
4. **Display Code** - Should use correct I2C pins (15, 9)

## References

- [GitHub Copilot Instructions Documentation](https://docs.github.com/en/copilot/customizing-copilot/adding-custom-instructions-for-github-copilot)
- Repository docs: `ESP32_MINING_STRATEGIES.md`, `MINING_QUICKSTART.md`
- Agent context: `.github/agents/CONTEXT.md`

## Notes

This fix addresses the issue where Copilot seemed to "not have or use contexts" because it wasn't reading from the correct file location. Now that instructions are in `.github/copilot-instructions.md`, Copilot will have full project context when making suggestions.

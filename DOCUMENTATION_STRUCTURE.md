# Documentation Structure

This document explains the organization of documentation in this repository.

## User-Facing Documentation (Root Directory)

These files are for users and contributors:

- **[README.md](README.md)** - Project overview, getting started, build instructions
- **[CHANGELOG.md](CHANGELOG.md)** - User-facing changelog with notable changes
- **[MINING_QUICKSTART.md](MINING_QUICKSTART.md)** - Quick start guide for ESP32 mining (5 minutes)
- **[ESP32_MINING_STRATEGIES.md](ESP32_MINING_STRATEGIES.md)** - Comprehensive mining strategies, hardware specs, and optimization
- **[CI_CD_SETUP.md](CI_CD_SETUP.md)** - CI/CD workflows and contribution guidelines

### Subdirectories
- **[driver/README.md](driver/README.md)** - I2C driver documentation
- **[scripts/README.md](scripts/README.md)** - Build and test scripts documentation
- **[GPIO_Pin_Test/README.md](GPIO_Pin_Test/README.md)** - GPIO pin testing tool

## AI Agent Context (.github/agents/)

These files are for GitHub Copilot and other AI agents:

- **[.github/copilot-instructions.md](.github/copilot-instructions.md)** - Instructions for GitHub Copilot IDE extension
- **[.github/agents/CONTEXT.md](.github/agents/CONTEXT.md)** - Context for GitHub Copilot Workspace agents

### Implementation Notes (.github/agents/implementation-notes/)

Detailed technical implementation notes archived for reference:

- **[IMPLEMENTATION_COMPLETE.md](.github/agents/implementation-notes/IMPLEMENTATION_COMPLETE.md)** - I2C driver update completion summary
- **[IMPLEMENTATION_SUMMARY.md](.github/agents/implementation-notes/IMPLEMENTATION_SUMMARY.md)** - Test infrastructure implementation
- **[I2C_DRIVER_UPDATE.md](.github/agents/implementation-notes/I2C_DRIVER_UPDATE.md)** - Technical I2C driver details
- **[CHANGELOG_I2C_DIAGNOSTIC.md](.github/agents/implementation-notes/CHANGELOG_I2C_DIAGNOSTIC.md)** - Detailed I2C diagnostic process (Portuguese)
- **[CHANGELOG2.md](.github/agents/implementation-notes/CHANGELOG2.md)** - I2C diagnostic summary (Portuguese)

## Documentation Principles

### User-Facing Docs Should:
- ✅ Be clear and concise
- ✅ Focus on what users need to know
- ✅ Include practical examples
- ✅ Be written in English (primary)
- ✅ Avoid excessive technical implementation details

### AI Agent Context Should:
- ✅ Include detailed technical context
- ✅ Explain design decisions and rationale
- ✅ Provide historical implementation notes
- ✅ Reference coding patterns and best practices
- ✅ Be kept in `.github/agents/` directory

### Implementation Notes Should:
- ✅ Document detailed problem-solving approaches
- ✅ Include step-by-step technical processes
- ✅ Be archived in `.github/agents/implementation-notes/`
- ✅ Be referenced by AI agents when working on related features

## Quick Reference

| Need | Look Here |
|------|-----------|
| **Get started** | [README.md](README.md) |
| **Quick mining setup** | [MINING_QUICKSTART.md](MINING_QUICKSTART.md) |
| **Detailed mining info** | [ESP32_MINING_STRATEGIES.md](ESP32_MINING_STRATEGIES.md) |
| **Contribute code** | [CI_CD_SETUP.md](CI_CD_SETUP.md) |
| **See what changed** | [CHANGELOG.md](CHANGELOG.md) |
| **Configure I2C** | [driver/README.md](driver/README.md) |
| **Test GPIO pins** | [GPIO_Pin_Test/README.md](GPIO_Pin_Test/README.md) |

---

**Last Updated:** 2025-10-31

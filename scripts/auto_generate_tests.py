#!/usr/bin/env python3
"""
Auto Test Generator
Automatically generates unit tests for functions that don't have tests.
"""

import os
import sys
import re
from pathlib import Path
from typing import List, Set, Tuple

# Import the feature detector module
import importlib.util
spec = importlib.util.spec_from_file_location("detect_features", 
    os.path.join(os.path.dirname(__file__), "detect_features.py"))
detect_features = importlib.util.module_from_spec(spec)
spec.loader.exec_module(detect_features)


def parse_function_signature(file_path: str, func_name: str, line_number: int) -> dict:
    """Parse full function signature including parameters"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
            
            # Get the function declaration (may span multiple lines)
            signature_lines = []
            for i in range(line_number - 1, min(line_number + 5, len(lines))):
                line = lines[i].strip()
                signature_lines.append(line)
                if '{' in line or ';' in line:
                    break
            
            signature = ' '.join(signature_lines)
            
            # Extract parameters
            param_match = re.search(r'\((.*?)\)', signature)
            if param_match:
                params_str = param_match.group(1)
                params = [p.strip() for p in params_str.split(',') if p.strip() and p.strip() != 'void']
            else:
                params = []
            
            return {
                'signature': signature,
                'parameters': params
            }
    except Exception as e:
        return {'signature': '', 'parameters': []}


def generate_comprehensive_test(func: detect_features.FunctionInfo, file_path: str) -> str:
    """Generate a comprehensive test with multiple test cases"""
    
    sig_info = parse_function_signature(func.file_path, func.name, func.line_number)
    params = sig_info['parameters']
    
    test_cases = []
    
    # Generate basic test
    test_cases.append(f"""
// Test {func.name} with valid inputs
void test_{func.name}_valid(void)
{{
    // TODO: Setup valid test data
    // TODO: Call {func.name}
    // TODO: Assert expected behavior
    TEST_ASSERT(true); // Placeholder - implement actual test
}}""")
    
    # If function has parameters, generate edge case tests
    if params:
        test_cases.append(f"""
// Test {func.name} with edge cases
void test_{func.name}_edge_cases(void)
{{
    // TODO: Test boundary conditions
    // TODO: Test with minimum/maximum values
    // TODO: Assert correct handling
    TEST_ASSERT(true); // Placeholder - implement actual test
}}""")
        
        # Check if any parameter might be a pointer
        if any('*' in p for p in params):
            test_cases.append(f"""
// Test {func.name} with NULL parameters
void test_{func.name}_null_params(void)
{{
    // TODO: Test behavior with NULL pointers
    // TODO: Ensure function handles NULL gracefully
    TEST_ASSERT(true); // Placeholder - implement actual test
}}""")
    
    # If return type is not void, add return value test
    if func.return_type != 'void':
        test_cases.append(f"""
// Test {func.name} return value
void test_{func.name}_return_value(void)
{{
    // TODO: Call {func.name}
    // TODO: Verify return value is correct
    TEST_ASSERT(true); // Placeholder - implement actual test
}}""")
    
    header = f"""
// ==============================================================================
// Tests for {func.name}
// Location: {func.file_path}:{func.line_number}
// Signature: {sig_info['signature'][:80]}{'...' if len(sig_info['signature']) > 80 else ''}
// ==============================================================================
"""
    
    return header + '\n'.join(test_cases)


def generate_test_file(functions: List[detect_features.FunctionInfo], module_name: str) -> str:
    """Generate a complete test file for a module"""
    
    # Determine which headers are needed based on the module
    module_headers = set()
    for func in functions:
        # Extract module name from file path to include appropriate header
        file_path = Path(func.file_path)
        if file_path.stem != 'main':  # Don't include main.h if it exists
            module_headers.add(f'"{file_path.stem}.h"')
    
    header_includes = '\n'.join(f'#include {h}' for h in sorted(module_headers))
    
    header = f"""#include <string.h>
#include <stdint.h>
#include "unity.h"
{header_includes if header_includes else ''}

void setUp(void)
{{
    // Setup before each test
}}

void tearDown(void)
{{
    // Cleanup after each test
}}
"""
    
    tests = []
    for func in functions:
        tests.append(generate_comprehensive_test(func, func.file_path))
    
    test_runner = f"""
// ==============================================================================
// Test Runner
// ==============================================================================
void test_{module_name}_functions(void)
{{
"""
    
    for func in functions:
        test_runner += f"    RUN_TEST(test_{func.name}_valid);\n"
        sig_info = parse_function_signature(func.file_path, func.name, func.line_number)
        if sig_info['parameters']:
            test_runner += f"    RUN_TEST(test_{func.name}_edge_cases);\n"
            if any('*' in p for p in sig_info['parameters']):
                test_runner += f"    RUN_TEST(test_{func.name}_null_params);\n"
        if func.return_type != 'void':
            test_runner += f"    RUN_TEST(test_{func.name}_return_value);\n"
    
    test_runner += "}\n"
    
    return header + '\n'.join(tests) + test_runner


def main():
    """Main function"""
    if len(sys.argv) > 1:
        project_root = sys.argv[1]
    else:
        project_root = os.getcwd()
    
    src_dir = os.path.join(project_root, 'main')
    test_dir = os.path.join(project_root, 'test')
    
    print("=" * 80)
    print("Auto Test Generator")
    print("=" * 80)
    print(f"Analyzing project: {project_root}")
    print()
    
    # Analyze the project using the feature detector
    all_functions, tested_functions = detect_features.analyze_project(src_dir, test_dir)
    
    # Find untested functions
    untested_functions = []
    for func in all_functions:
        has_test = any(func.name in tested_name or tested_name in func.name 
                      for tested_name in tested_functions)
        if not has_test and not func.is_static:
            untested_functions.append(func)
    
    if not untested_functions:
        print("✅ All functions have tests!")
        return 0
    
    print(f"Found {len(untested_functions)} untested functions")
    print()
    
    # Group functions by source file
    functions_by_file = {}
    for func in untested_functions:
        module = Path(func.file_path).stem
        if module not in functions_by_file:
            functions_by_file[module] = []
        functions_by_file[module].append(func)
    
    # Generate test files
    for module, functions in functions_by_file.items():
        output_file = os.path.join(test_dir, f'test_{module}_auto.c')
        
        print(f"Generating {output_file} with {len(functions)} test functions...")
        
        test_content = generate_test_file(functions, module)
        
        # Write to file
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(test_content)
        
        print(f"  ✅ Generated {output_file}")
    
    print()
    print("=" * 80)
    print("Test generation complete!")
    print()
    print("Next steps:")
    print("1. Review generated test files and implement the TODO items")
    print("2. Add necessary #include directives")
    print("3. Update test/CMakeLists.txt to include new test files")
    print("4. Build and run tests with: idf.py build")
    print()
    
    return 0


if __name__ == '__main__':
    sys.exit(main())

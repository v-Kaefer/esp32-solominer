#!/usr/bin/env python3
"""
Feature Implementation Detector
Analyzes C source files to detect functions and checks if they have corresponding unit tests.
"""

import os
import re
import sys
from pathlib import Path
from typing import List, Dict, Set, Tuple

# Patterns to detect function definitions in C
FUNCTION_PATTERN = re.compile(
    r'^(?:static\s+)?(?:inline\s+)?'  # Optional static/inline
    r'(?:const\s+)?'  # Optional const
    r'(\w+(?:\s*\*)?)\s+'  # Return type (including pointer)
    r'(\w+)\s*'  # Function name
    r'\([^)]*\)\s*'  # Parameters
    r'(?:{|;)',  # Opening brace or semicolon (for declarations)
    re.MULTILINE
)

# Patterns for test functions
TEST_FUNCTION_PATTERN = re.compile(r'test_(\w+)', re.IGNORECASE)

# Functions to exclude from testing requirements
EXCLUDED_FUNCTIONS = {
    'main', 'app_main', 'setUp', 'tearDown', 
    'vTaskDelay', 'printf', 'ESP_LOGI', 'ESP_LOGW', 'ESP_LOGE'
}

# Return types that indicate non-testable functions
NON_TESTABLE_RETURN_TYPES = {'void'}


class FunctionInfo:
    """Information about a detected function"""
    def __init__(self, name: str, return_type: str, file_path: str, line_number: int):
        self.name = name
        self.return_type = return_type
        self.file_path = file_path
        self.line_number = line_number
        self.is_static = False
        self.is_testable = True

    def __repr__(self):
        return f"Function({self.name}, {self.return_type}, {self.file_path}:{self.line_number})"


def find_functions_in_file(file_path: str) -> List[FunctionInfo]:
    """Extract all function definitions from a C file"""
    functions = []
    
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            lines = content.split('\n')
            
            for i, line in enumerate(lines, 1):
                # Skip comments and preprocessor directives
                if line.strip().startswith('//') or line.strip().startswith('#'):
                    continue
                
                match = FUNCTION_PATTERN.match(line)
                if match:
                    return_type = match.group(1).strip()
                    func_name = match.group(2).strip()
                    
                    # Skip if it's a common macro or excluded function
                    if func_name in EXCLUDED_FUNCTIONS:
                        continue
                    
                    # Skip if it looks like a type definition
                    if func_name[0].isupper() and func_name.endswith('_t'):
                        continue
                    
                    func_info = FunctionInfo(func_name, return_type, file_path, i)
                    
                    # Check if static
                    if 'static' in line:
                        func_info.is_static = True
                    
                    functions.append(func_info)
    
    except Exception as e:
        print(f"Error reading {file_path}: {e}", file=sys.stderr)
    
    return functions


def find_test_functions(test_dir: str) -> Set[str]:
    """Find all test functions in test files"""
    tested_functions = set()
    
    if not os.path.exists(test_dir):
        return tested_functions
    
    for file_path in Path(test_dir).rglob('*.c'):
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                
                # Find test functions like test_function_name
                for match in TEST_FUNCTION_PATTERN.finditer(content):
                    tested_name = match.group(1)
                    tested_functions.add(tested_name)
                    
                    # Also check for the full function name in the test
                    # e.g., test_double_sha256 tests double_sha256
                    if '_' in tested_name:
                        parts = tested_name.split('_')
                        # Try various combinations
                        for i in range(len(parts)):
                            tested_functions.add('_'.join(parts[i:]))
        
        except Exception as e:
            print(f"Error reading test file {file_path}: {e}", file=sys.stderr)
    
    return tested_functions


def generate_test_stub(func: FunctionInfo) -> str:
    """Generate a test stub for a function"""
    test_name = f"test_{func.name}"
    
    stub = f"""
// Test for {func.name}
void {test_name}(void)
{{
    // TODO: Implement test for {func.name}
    // Function signature: {func.return_type} {func.name}(...)
    // Location: {func.file_path}:{func.line_number}
    
    // Example test structure:
    // 1. Setup test data
    // 2. Call the function
    // 3. Assert expected results
    
    TEST_ASSERT(false); // Remove this and implement actual test
}}
"""
    return stub


def analyze_project(src_dir: str, test_dir: str) -> Tuple[List[FunctionInfo], Set[str]]:
    """Analyze project to find functions and their test coverage"""
    all_functions = []
    
    # Find all C source files in src_dir
    for file_path in Path(src_dir).rglob('*.c'):
        # Skip test files
        if 'test' in str(file_path).lower():
            continue
        
        functions = find_functions_in_file(str(file_path))
        all_functions.extend(functions)
    
    # Find all existing tests
    tested_functions = find_test_functions(test_dir)
    
    return all_functions, tested_functions


def main():
    """Main function"""
    if len(sys.argv) > 1:
        project_root = sys.argv[1]
    else:
        project_root = os.getcwd()
    
    src_dir = os.path.join(project_root, 'main')
    test_dir = os.path.join(project_root, 'test')
    
    print("=" * 80)
    print("Feature Implementation Detector")
    print("=" * 80)
    print(f"Analyzing project: {project_root}")
    print(f"Source directory: {src_dir}")
    print(f"Test directory: {test_dir}")
    print()
    
    # Analyze the project
    all_functions, tested_functions = analyze_project(src_dir, test_dir)
    
    # Categorize functions
    untested_functions = []
    tested_function_list = []
    
    for func in all_functions:
        # Check if this function has a test
        has_test = False
        for tested_name in tested_functions:
            if func.name in tested_name or tested_name in func.name:
                has_test = True
                break
        
        if has_test:
            tested_function_list.append(func)
        else:
            # Only report public (non-static) functions as untested
            if not func.is_static:
                untested_functions.append(func)
    
    # Print results
    print(f"Total functions found: {len(all_functions)}")
    print(f"Functions with tests: {len(tested_function_list)}")
    print(f"Functions without tests: {len(untested_functions)}")
    print()
    
    if untested_functions:
        print("⚠️  Functions without tests:")
        print("-" * 80)
        for func in untested_functions:
            print(f"  • {func.name:<30} ({func.return_type})")
            print(f"    Location: {func.file_path}:{func.line_number}")
        print()
        
        # Generate test stubs
        print("=" * 80)
        print("Generated Test Stubs (copy to test file):")
        print("=" * 80)
        for func in untested_functions[:5]:  # Limit to first 5 for brevity
            print(generate_test_stub(func))
        
        if len(untested_functions) > 5:
            print(f"... and {len(untested_functions) - 5} more functions need tests.")
    else:
        print("✅ All public functions have tests!")
    
    # Exit with error code if there are untested functions
    sys.exit(1 if untested_functions else 0)


if __name__ == '__main__':
    main()

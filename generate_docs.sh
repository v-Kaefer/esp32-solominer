#!/bin/bash
# Documentation generation and verification script

set -e

echo "=== ESP32 SoloMiner Documentation Generator ==="
echo ""

# Check if doxygen is installed
if ! command -v doxygen &> /dev/null; then
    echo "Error: Doxygen is not installed."
    echo "Please install it using:"
    echo "  Ubuntu/Debian: sudo apt-get install doxygen graphviz"
    echo "  macOS: brew install doxygen graphviz"
    exit 1
fi

# Check if graphviz is installed
if ! command -v dot &> /dev/null; then
    echo "Warning: Graphviz (dot) is not installed."
    echo "Call graphs will not be generated."
    echo "Install with:"
    echo "  Ubuntu/Debian: sudo apt-get install graphviz"
    echo "  macOS: brew install graphviz"
fi

echo "Generating documentation..."
doxygen Doxyfile

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Documentation generated successfully!"
    echo ""
    echo "Output location: docs/html/"
    echo "Open docs/html/index.html in your browser to view the documentation."
    
    # Try to open the documentation in the default browser
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v xdg-open &> /dev/null; then
            echo ""
            read -p "Open documentation in browser? (y/n) " -n 1 -r
            echo
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                xdg-open docs/html/index.html
            fi
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo ""
        read -p "Open documentation in browser? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            open docs/html/index.html
        fi
    fi
else
    echo ""
    echo "✗ Error generating documentation"
    exit 1
fi

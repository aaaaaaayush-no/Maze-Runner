#!/bin/bash

# Maze Runner - PowerPoint Conversion Script
# This script converts the markdown presentation to PowerPoint format

echo "================================================"
echo "Maze Runner - PowerPoint Conversion Script"
echo "================================================"
echo ""

# Get the script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Check if pandoc is installed
if ! command -v pandoc &> /dev/null; then
    echo "ERROR: Pandoc is not installed!"
    echo ""
    echo "Please install Pandoc:"
    echo "  Ubuntu/Debian: sudo apt-get install pandoc"
    echo "  macOS: brew install pandoc"
    echo "  Windows: Download from https://pandoc.org/installing.html"
    echo ""
    exit 1
fi

echo "Pandoc found: $(pandoc --version | head -n 1)"
echo ""

# Convert Presentation to PowerPoint
echo "Converting MAZE_RUNNER_PRESENTATION.md to PowerPoint..."
echo "------------------------------------------------------"

pandoc MAZE_RUNNER_PRESENTATION.md \
    -o MAZE_RUNNER_PRESENTATION.pptx \
    --slide-level=2 \
    2>&1

if [ $? -eq 0 ]; then
    echo "✓ PowerPoint presentation generated successfully!"
    echo ""
    echo "Output file: MAZE_RUNNER_PRESENTATION.pptx"
    echo ""
    echo "Next steps:"
    echo "  1. Open the file in Microsoft PowerPoint or LibreOffice Impress"
    echo "  2. Apply your preferred theme/template"
    echo "  3. Add any additional images or diagrams"
    echo "  4. Customize colors and fonts"
    echo "  5. Add transitions and animations if desired"
else
    echo "✗ Failed to generate PowerPoint presentation"
    echo ""
    echo "Troubleshooting:"
    echo "  1. Make sure Pandoc is properly installed"
    echo "  2. Check that the markdown file exists"
    echo "  3. Try running pandoc directly: pandoc MAZE_RUNNER_PRESENTATION.md -o output.pptx"
fi

echo ""
echo "================================================"
echo "Conversion Complete!"
echo "================================================"
echo ""

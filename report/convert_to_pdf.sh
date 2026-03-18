#!/bin/bash

# Maze Runner - PDF Conversion Script
# This script converts the markdown documentation to PDF format

echo "================================================"
echo "Maze Runner - PDF Conversion Script"
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
    echo "  Ubuntu/Debian: sudo apt-get install pandoc texlive-latex-base texlive-latex-extra"
    echo "  macOS: brew install pandoc && brew install --cask mactex"
    echo "  Windows: Download from https://pandoc.org/installing.html"
    echo ""
    exit 1
fi

echo "Pandoc found: $(pandoc --version | head -n 1)"
echo ""

# Check if pdflatex is installed
if ! command -v pdflatex &> /dev/null; then
    echo "WARNING: pdflatex is not installed!"
    echo "PDF generation may fail. Install a LaTeX distribution:"
    echo "  Ubuntu/Debian: sudo apt-get install texlive-latex-base texlive-latex-extra"
    echo "  macOS: brew install --cask mactex"
    echo "  Windows: Install MiKTeX from https://miktex.org/"
    echo ""
fi

# Convert Report to PDF
echo "Converting MAZE_RUNNER_REPORT.md to PDF..."
echo "----------------------------------------"

pandoc MAZE_RUNNER_REPORT.md \
    -o MAZE_RUNNER_REPORT.pdf \
    --pdf-engine=pdflatex \
    --toc \
    --toc-depth=3 \
    --number-sections \
    -V geometry:margin=1in \
    -V documentclass=report \
    -V fontsize=11pt \
    -V colorlinks=true \
    -V linkcolor=blue \
    -V urlcolor=blue \
    -V toccolor=black \
    --highlight-style=tango \
    2>&1

if [ $? -eq 0 ]; then
    echo "✓ Report PDF generated successfully: MAZE_RUNNER_REPORT.pdf"
else
    echo "✗ Failed to generate report PDF"
    echo ""
    echo "Try using xelatex instead:"
    echo "  pandoc MAZE_RUNNER_REPORT.md -o MAZE_RUNNER_REPORT.pdf --pdf-engine=xelatex --toc --number-sections"
fi

echo ""

# Convert Presentation to PDF (Beamer slides)
echo "Converting MAZE_RUNNER_PRESENTATION.md to PDF (Beamer)..."
echo "--------------------------------------------------------"

pandoc MAZE_RUNNER_PRESENTATION.md \
    -o MAZE_RUNNER_PRESENTATION.pdf \
    -t beamer \
    --pdf-engine=pdflatex \
    -V theme:Madrid \
    -V colortheme:dolphin \
    -V navigation:horizontal \
    --highlight-style=tango \
    2>&1

if [ $? -eq 0 ]; then
    echo "✓ Presentation PDF generated successfully: MAZE_RUNNER_PRESENTATION.pdf"
else
    echo "✗ Failed to generate presentation PDF"
    echo ""
    echo "Try a simpler theme:"
    echo "  pandoc MAZE_RUNNER_PRESENTATION.md -o MAZE_RUNNER_PRESENTATION.pdf -t beamer -V theme:default"
fi

echo ""
echo "================================================"
echo "Conversion Complete!"
echo "================================================"
echo ""
echo "Generated files:"
[ -f "MAZE_RUNNER_REPORT.pdf" ] && echo "  ✓ MAZE_RUNNER_REPORT.pdf"
[ -f "MAZE_RUNNER_PRESENTATION.pdf" ] && echo "  ✓ MAZE_RUNNER_PRESENTATION.pdf"
echo ""
echo "You can now:"
echo "  1. View the PDFs with any PDF reader"
echo "  2. Print or share the documents"
echo "  3. Edit the markdown files and regenerate"
echo ""

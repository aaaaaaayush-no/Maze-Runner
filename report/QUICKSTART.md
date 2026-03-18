# Quick Start Guide - Generating Report Documents

This guide will help you quickly generate PowerPoint and PDF versions of the Maze Runner documentation.

## What You'll Get

After following this guide, you'll have:
- **MAZE_RUNNER_PRESENTATION.pptx** - Professional PowerPoint presentation (40+ slides)
- **MAZE_RUNNER_REPORT.pdf** - Comprehensive technical report (40+ pages)
- **MAZE_RUNNER_PRESENTATION.pdf** - Presentation in PDF format (alternative to PPTX)

## Prerequisites

You need to install Pandoc (and optionally LaTeX for PDF generation).

### Installation

Choose your operating system:

#### Linux (Ubuntu/Debian)
```bash
# Install Pandoc
sudo apt-get update
sudo apt-get install -y pandoc

# Install LaTeX (for PDF generation)
sudo apt-get install -y texlive-latex-base texlive-latex-extra texlive-fonts-recommended
```

#### macOS
```bash
# Install Pandoc
brew install pandoc

# Install LaTeX (for PDF generation)
brew install --cask mactex
```

#### Windows
1. Download Pandoc from: https://pandoc.org/installing.html
2. Run the installer
3. (Optional) Install MiKTeX for PDF support: https://miktex.org/

## Quick Generation

### Method 1: Using the Provided Scripts (Recommended)

**On Linux/macOS:**
```bash
cd report

# Generate PowerPoint
./convert_to_pptx.sh

# Generate PDFs
./convert_to_pdf.sh
```

**On Windows (Git Bash or WSL):**
```bash
cd report
bash convert_to_pptx.sh
bash convert_to_pdf.sh
```

### Method 2: Manual Pandoc Commands

**Generate PowerPoint:**
```bash
cd report
pandoc MAZE_RUNNER_PRESENTATION.md -o MAZE_RUNNER_PRESENTATION.pptx
```

**Generate Report PDF:**
```bash
cd report
pandoc MAZE_RUNNER_REPORT.md -o MAZE_RUNNER_REPORT.pdf --pdf-engine=pdflatex --toc --number-sections
```

**Generate Presentation PDF (Beamer slides):**
```bash
cd report
pandoc MAZE_RUNNER_PRESENTATION.md -o MAZE_RUNNER_PRESENTATION.pdf -t beamer -V theme:Madrid
```

## Troubleshooting

### "pandoc: command not found"
- Install Pandoc following the instructions above
- Make sure Pandoc is in your PATH
- Restart your terminal after installation

### "pdflatex not found" or LaTeX errors
- Install a LaTeX distribution (TeXLive on Linux, MacTeX on macOS, MiKTeX on Windows)
- Try using XeLaTeX instead: `--pdf-engine=xelatex`
- Or skip PDF generation and just create PowerPoint

### Permission Denied (Linux/macOS)
```bash
chmod +x convert_to_pdf.sh convert_to_pptx.sh
```

### PowerPoint looks plain
- Open the generated PPTX in PowerPoint or LibreOffice
- Apply a theme: Design → Themes → Choose your preferred theme
- Customize colors, fonts, and layouts as needed

## Alternative Methods

### Without Installing Anything (Online)

If you don't want to install software, you can use online converters:

1. **For PowerPoint:**
   - Go to https://cloudconvert.com/md-to-pptx
   - Upload `MAZE_RUNNER_PRESENTATION.md`
   - Download the generated PPTX

2. **For PDF:**
   - Go to https://cloudconvert.com/md-to-pdf
   - Upload `MAZE_RUNNER_REPORT.md`
   - Download the generated PDF

### Using Visual Studio Code

1. Install VS Code: https://code.visualstudio.com/
2. Install "Markdown PDF" extension
3. Open the markdown file
4. Right-click → "Markdown PDF: Export (pdf)"

### Using Python

```bash
# Install converter
pip install markdown2pptx

# Convert to PowerPoint
markdown2pptx MAZE_RUNNER_PRESENTATION.md MAZE_RUNNER_PRESENTATION.pptx
```

## Customization Tips

### Change PowerPoint Theme
```bash
# Use a custom template
pandoc MAZE_RUNNER_PRESENTATION.md -o output.pptx --reference-doc=your-template.pptx
```

### Change PDF Report Style
```bash
# Custom margins and fonts
pandoc MAZE_RUNNER_REPORT.md -o output.pdf \
    --pdf-engine=xelatex \
    --toc \
    --number-sections \
    -V geometry:margin=0.75in \
    -V mainfont="Times New Roman" \
    -V fontsize=12pt \
    -V colorlinks=true
```

### Change Presentation Theme
```bash
# Use different Beamer theme
pandoc MAZE_RUNNER_PRESENTATION.md -o output.pdf \
    -t beamer \
    -V theme:Warsaw \
    -V colortheme:beaver
```

Available Beamer themes:
- Madrid (recommended), Warsaw, Copenhagen, Berlin
- Singapore, Malmoe, AnnArbor, Bergen, and more

## Next Steps

After generating the files:

1. **For PowerPoint (PPTX):**
   - Open in Microsoft PowerPoint or LibreOffice Impress
   - Apply your school/company theme
   - Add institution logo to master slides
   - Review and adjust font sizes if needed
   - Add any additional images or diagrams

2. **For PDF Report:**
   - Review the document
   - Print or share digitally
   - Use as project submission

3. **For PDF Presentation:**
   - Use for handouts
   - Share with those who don't have PowerPoint
   - Print multiple slides per page

## What's Included

### Technical Report Content
- Project abstract and introduction
- System requirements and architecture
- Detailed technical implementation
- Graphics pipeline explanation
- Code examples and algorithms
- Testing results and metrics
- Future enhancements
- Complete references

### Presentation Content
- Project overview (5 slides)
- Features and architecture (8 slides)
- Graphics pipeline details (6 slides)
- Implementation highlights (8 slides)
- Code examples (4 slides)
- Testing and results (3 slides)
- Demo and conclusion (6 slides)

## File Sizes

Approximate file sizes:
- MAZE_RUNNER_PRESENTATION.pptx: ~500 KB - 2 MB
- MAZE_RUNNER_REPORT.pdf: ~200 KB - 1 MB
- MAZE_RUNNER_PRESENTATION.pdf: ~300 KB - 1 MB

## Need Help?

If you encounter issues:

1. Check the detailed README.md in this directory
2. Verify Pandoc is installed: `pandoc --version`
3. Try the online conversion methods
4. Check Pandoc documentation: https://pandoc.org/MANUAL.html

## Summary

**Fastest method:**
```bash
cd report
./convert_to_pptx.sh    # Generates PowerPoint
./convert_to_pdf.sh      # Generates PDFs
```

That's it! You should now have professional-looking documentation for your Maze Runner project.

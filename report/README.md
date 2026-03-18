# Maze Runner - Report Documentation

This directory contains comprehensive documentation for the Maze Runner project, including a detailed technical report and a presentation suitable for academic or professional purposes.

## Contents

1. **MAZE_RUNNER_REPORT.md** - Comprehensive technical report (PDF-ready)
2. **MAZE_RUNNER_PRESENTATION.md** - Presentation slides (PowerPoint-ready)
3. **README.md** - This file (conversion instructions)
4. **convert_to_pdf.sh** - Shell script for PDF generation
5. **convert_to_pptx.sh** - Shell script for PowerPoint generation

## Document Overview

### Technical Report (MAZE_RUNNER_REPORT.md)

A comprehensive 40+ page technical report covering:
- Project abstract and introduction
- System requirements and architecture
- Core features and implementation details
- Graphics pipeline explanation
- Testing results and performance metrics
- Future enhancements and conclusion
- Complete references and appendices

**Suitable for:** Project submissions, technical documentation, academic reports

### Presentation (MAZE_RUNNER_PRESENTATION.md)

A professional 40+ slide presentation covering:
- Project overview and objectives
- Technical architecture and features
- Graphics pipeline and algorithms
- Code examples and implementation
- Testing results and demonstrations
- Future work and conclusions

**Suitable for:** Class presentations, project demos, conference talks

## Converting to PowerPoint (PPTX)

The presentation markdown file can be converted to PowerPoint using [Pandoc](https://pandoc.org/).

### Prerequisites

Install Pandoc:

**Ubuntu/Debian:**
```bash
sudo apt-get install pandoc
```

**macOS:**
```bash
brew install pandoc
```

**Windows:**
Download from https://pandoc.org/installing.html

### Conversion Command

**Basic conversion:**
```bash
pandoc MAZE_RUNNER_PRESENTATION.md -o MAZE_RUNNER_PRESENTATION.pptx
```

**With custom theme:**
```bash
pandoc MAZE_RUNNER_PRESENTATION.md \
    -o MAZE_RUNNER_PRESENTATION.pptx \
    --reference-doc=custom-template.pptx
```

**Using the provided script:**
```bash
chmod +x convert_to_pptx.sh
./convert_to_pptx.sh
```

### Customization

After generating the PPTX file, you can:
1. Open in Microsoft PowerPoint or LibreOffice Impress
2. Apply your preferred theme/template
3. Add images, charts, or diagrams
4. Adjust font sizes and colors
5. Add transitions and animations

## Converting to PDF

Both documents can be converted to professional PDF files using Pandoc.

### Prerequisites

Install Pandoc and LaTeX:

**Ubuntu/Debian:**
```bash
sudo apt-get install pandoc texlive-latex-base texlive-latex-extra texlive-fonts-recommended
```

**macOS:**
```bash
brew install pandoc
brew install --cask mactex
```

**Windows:**
- Install Pandoc from https://pandoc.org/installing.html
- Install MiKTeX from https://miktex.org/

### Conversion Commands

**Convert report to PDF:**
```bash
pandoc MAZE_RUNNER_REPORT.md \
    -o MAZE_RUNNER_REPORT.pdf \
    --pdf-engine=pdflatex \
    --toc \
    --number-sections \
    -V geometry:margin=1in \
    -V documentclass=report \
    -V fontsize=11pt
```

**Convert presentation to PDF (slides):**
```bash
pandoc MAZE_RUNNER_PRESENTATION.md \
    -o MAZE_RUNNER_PRESENTATION.pdf \
    -t beamer \
    --pdf-engine=pdflatex \
    -V theme:Madrid \
    -V colortheme:default
```

**Using the provided script:**
```bash
chmod +x convert_to_pdf.sh
./convert_to_pdf.sh
```

### Advanced PDF Options

**For better looking report:**
```bash
pandoc MAZE_RUNNER_REPORT.md \
    -o MAZE_RUNNER_REPORT.pdf \
    --pdf-engine=xelatex \
    --toc \
    --number-sections \
    -V geometry:margin=1in \
    -V documentclass=report \
    -V fontsize=11pt \
    -V colorlinks=true \
    -V linkcolor=blue \
    -V urlcolor=blue \
    -V toccolor=black \
    --highlight-style=tango
```

## Alternative Conversion Methods

### Using Online Tools

1. **Markdown to PowerPoint:**
   - Upload to https://cloudconvert.com/md-to-pptx
   - Or use https://convertio.co/md-pptx/

2. **Markdown to PDF:**
   - Upload to https://cloudconvert.com/md-to-pdf
   - Or use https://www.markdowntopdf.com/

### Using Visual Studio Code

Install extensions:
1. "Markdown PDF" by yzane
2. "Marp for VS Code" for presentations

### Using Python (markdown2pptx)

```bash
pip install markdown2pptx
markdown2pptx MAZE_RUNNER_PRESENTATION.md MAZE_RUNNER_PRESENTATION.pptx
```

### Using LibreOffice

1. Install `pandoc` and LibreOffice
2. Convert to ODT first: `pandoc MAZE_RUNNER_REPORT.md -o report.odt`
3. Open in LibreOffice Writer and export as PDF
4. Or convert to ODP: `pandoc MAZE_RUNNER_PRESENTATION.md -o presentation.odp -t odt`

## Viewing Markdown

You can view the markdown files directly in:

- **GitHub:** Automatic rendering in repository
- **VS Code:** Built-in markdown preview (Ctrl+Shift+V)
- **Typora:** Dedicated markdown editor with live preview
- **Obsidian:** Knowledge base with markdown support
- **Marked 2 (macOS):** Professional markdown preview

## Customizing the Documents

### Modifying the Report

The report is structured with:
- Markdown headers for sections (# ## ###)
- Code blocks with syntax highlighting
- Tables for data presentation
- Lists for feature enumeration

To customize:
1. Edit the markdown file directly
2. Add images: `![Caption](path/to/image.png)`
3. Add code: Use triple backticks with language
4. Add tables: Use pipe syntax

### Modifying the Presentation

The presentation uses:
- `---` to separate slides
- `#` for slide titles
- `##` for slide subtitles
- YAML frontmatter for metadata

To customize:
1. Edit slide content between `---` markers
2. Adjust YAML frontmatter for theme/author
3. Add speaker notes with `Notes:` section
4. Use `:::` for special blocks

## PDF Generation Themes

### Beamer Themes (for presentations)

Available themes:
- AnnArbor, Antibes, Bergen, Berkeley, Berlin
- Boadilla, CambridgeUS, Copenhagen, Darmstadt
- Dresden, Frankfurt, Goettingen, Hannover
- Ilmenau, JuanLesPins, Luebeck, Madrid, Malmoe
- Marburg, Montpellier, PaloAlto, Pittsburgh
- Rochester, Singapore, Szeged, Warsaw

Usage:
```bash
pandoc MAZE_RUNNER_PRESENTATION.md -o output.pdf -t beamer -V theme:Madrid
```

### Color Themes

Available color themes:
- default, albatross, beaver, beetle, crane
- dolphin, dove, fly, lily, orchid
- rose, seagull, seahorse, whale, wolverine

Usage:
```bash
pandoc MAZE_RUNNER_PRESENTATION.md -o output.pdf -t beamer -V theme:Madrid -V colortheme:dolphin
```

## Troubleshooting

### Pandoc Not Found

Make sure Pandoc is installed and in your PATH:
```bash
pandoc --version
```

### LaTeX Errors

If you get LaTeX errors during PDF generation:
1. Make sure you have a complete LaTeX distribution installed
2. Try using `--pdf-engine=xelatex` instead of `pdflatex`
3. Install missing packages: `tlmgr install <package-name>`

### Missing Fonts

If fonts are not rendering correctly:
1. Use `--pdf-engine=xelatex` for better Unicode support
2. Install Microsoft fonts or Liberation fonts
3. Specify fonts: `-V mainfont="Liberation Serif"`

### Images Not Showing

Make sure image paths are:
1. Relative to the markdown file location
2. Using forward slashes (even on Windows)
3. Properly encoded (no special characters)

## Conversion Script Usage

### convert_to_pdf.sh

Generates both report PDF and presentation PDF:
```bash
chmod +x convert_to_pdf.sh
./convert_to_pdf.sh
```

Output files:
- `MAZE_RUNNER_REPORT.pdf`
- `MAZE_RUNNER_PRESENTATION.pdf` (Beamer slides)

### convert_to_pptx.sh

Generates PowerPoint presentation:
```bash
chmod +x convert_to_pptx.sh
./convert_to_pptx.sh
```

Output file:
- `MAZE_RUNNER_PRESENTATION.pptx`

## Tips for Best Results

### For PowerPoint:

1. **Use a reference template:**
   ```bash
   pandoc MAZE_RUNNER_PRESENTATION.md \
       -o output.pptx \
       --reference-doc=template.pptx
   ```

2. **Keep slides concise:** Each slide should have 3-5 bullet points

3. **Use consistent formatting:** Maintain heading hierarchy

4. **Add speaker notes:** Use note blocks for presentation notes

### For PDF:

1. **Enable table of contents:** Use `--toc` flag

2. **Add page numbers:** Use `-V pagestyle=plain`

3. **Syntax highlighting:** Use `--highlight-style=tango`

4. **Custom margins:** Use `-V geometry:margin=0.75in`

5. **Font selection:** Use `-V mainfont="Times New Roman"`

## Examples

### Professional Report PDF

```bash
pandoc MAZE_RUNNER_REPORT.md \
    -o MAZE_RUNNER_REPORT.pdf \
    --pdf-engine=xelatex \
    --toc \
    --toc-depth=3 \
    --number-sections \
    -V geometry:margin=1in \
    -V documentclass=report \
    -V fontsize=11pt \
    -V mainfont="Times New Roman" \
    -V colorlinks=true \
    -V linkcolor=blue \
    -V urlcolor=blue \
    --highlight-style=breezedark
```

### Academic Presentation PDF

```bash
pandoc MAZE_RUNNER_PRESENTATION.md \
    -o MAZE_RUNNER_PRESENTATION.pdf \
    -t beamer \
    --pdf-engine=pdflatex \
    -V theme:Madrid \
    -V colortheme:dolphin \
    -V navigation:horizontal \
    --highlight-style=tango
```

### Modern PowerPoint

```bash
pandoc MAZE_RUNNER_PRESENTATION.md \
    -o MAZE_RUNNER_PRESENTATION.pptx \
    --slide-level=2
```

## Additional Resources

- **Pandoc Manual:** https://pandoc.org/MANUAL.html
- **Markdown Guide:** https://www.markdownguide.org/
- **Beamer User Guide:** http://mirrors.ctan.org/macros/latex/contrib/beamer/doc/beameruserguide.pdf
- **LaTeX Documentation:** https://www.latex-project.org/help/documentation/

## License

These documentation files are part of the Maze Runner project and are provided for educational purposes.

---

*Generated: March 2026*
*Project: Maze Runner - 3D Maze Exploration Game*

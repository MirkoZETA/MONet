# Documentation Guide

This document provides guidelines for modifying the documentation. The documentation is generated using **Doxygen** and deployed via **Sphinx** on **Read the Docs**.

## Documentation Setup

### 1. Doxygen
- The Doxygen configuration file is located at: **`./docs/Doxyfile`**
- This configuration extracts relevant documentation from the source code.
- The documentation uses the **doxygen-awesome** submodule for improved styling.

### 2. Doxygen-Awesome (Enhanced CSS Styling)
- The **doxygen-awesome** submodule improves the visual presentation of Doxygen-generated content.
- The CSS styling is applied automatically during Doxygen generation.

### 3. Sphinx (Deployment via Read the Docs)
- **Sphinx** generates the Doxygen output through config.py.
- Configuration is defined in: **`./docs/sphinx/source/conf.py`**
- Sphinx is set to include all Doxygen-generated HTML files.

### 4. Read the Docs Configuration
- Documentation is hosted on **Read the Docs**.
- The build process is configured in: **`./readthedocs.yml`**
- Read the Docs automatically triggers Sphinx to generate and update the documentation upon repository updates.

### 5. Building the Documentation Locally

Before building the documentation locally, ensure the following tools are installed:

**Doxygen**:
```bash
sudo apt install doxygen
```
**Sphinx**:
```bash
python3 -m venv .venv
source ./.venv/bin/activate
pip install -U sphinx
```
**Graphviz** is required for generating diagrams via the dot tool (used by Doxygen).
```bash
sudo apt install graphviz
```
To build the documentation, Navigate to the Sphinx folder: **`docs/sphinx`** and run the build command:
```bash
make html
```
Open **`docs/sphinx/build/html/index.html`** in your browser to preview the documentation.

## Adding Custom Pages

When adding custom documentation pages to the project:

1. Create your new documentation page with appropriate Doxygen markup
2. Update the navigation tree structure in **`./resources/DoxygenLayout.xml`** to include your new page
3. The DoxygenLayout.xml file controls the structure and organization of pages in the documentation sidebar
4. Without updating this file, new pages will be generated but won't appear in the navigation menu

## Deployment Workflow
- Each commit to the repository triggers an automatic rebuild of the documentation on **Read the Docs**.
- The latest deployed version can be accessed at: **[Read the Docs Deployment](https://flex-net-sim-fork.readthedocs.io/en/stable/)**

## Modifying the Meta Tag for Google Search Verification
To associate the documentation with **Google Search Console** for verification, you need to modify the custom HTML header at `./docs/Doxygen/header.html`. It is currently linked to [Mirko Zitkovich's](https://gitlab.com/mirkozeta/) google account.

## Guidelines
- Ensure that all documentation updates are consistently reflected in **Doxygen**.
- Perform a local build before committing changes to verify formatting and correctness, see section 5.
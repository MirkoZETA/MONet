import os
import subprocess

read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'

if read_the_docs_build:
    # Get submodules
    subprocess.call('git submodule update --init --recursive', shell=True)

    # Run Doxygen to generate HTML docs before Sphinx builds
    subprocess.call('cd .. ; cd .. ;doxygen Doxyfile', shell=True)
    
# Tell ReadTheDocs to serve Doxygen HTML files
html_extra_path = ['../../doxygen-output/html']
templates_path = ["../../doxygen-output/html"]
html_additional_pages = {"index": "index.html"}
#!/bin/bash

# This script generates resource file mloader_gtk_resources.c during cmake configuration
glib-compile-resources --target=../src/resource/mloader_gtk_resources.c --generate-source mloader-gtk-resources.xml

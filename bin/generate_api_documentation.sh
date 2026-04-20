#!/usr/bin/env bash
#
# Copyright (C) Sven Rosvall (sven@rosvall.ie)
# This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
# Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
# The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/
#
# Generate documentation from source code.
# The documentation is generated for two target groups: sketch developers and library developers.
#

doxygen Doxygen.Sketch.conf
doxygen Doxygen.Library.conf
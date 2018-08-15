#!/bin/bash
##
# @file This file is part of MeshGen.
#
# @section LICENSE
# MIT License
# 
# Copyright (c) 2018 Rajdeep Konwar
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# @section DESCRIPTION
# Mesh generator shell script.
##

# Usage info
function show_help() {
cat << EOF
Usage: ${0##*/} [-h] [-c CONF -m MODEL -o MSHDIR] [-x]
Generates Eureka-format mesh file using Gmsh as meshing-tool.
    -h        This help message.
    -c CONF   Input config file.
    -m MODEL  Model name (used for mesh filenames).
    -o MSHDIR Mesh directory.
    -x        Do not delete intermediate mat file (optional).
EOF
}

# Parse command line arguments
OPTIND=1
MAT=1

while getopts "hc:m:o:x" opt; do
  case "$opt" in
    h)
      show_help       # Show help
      exit 0
      ;;
    c)
      CONF=$OPTARG    # GeoGen input config file
      ;;
    m)
      MODEL=$OPTARG     # Gmsh output msh file
      ;;
    o)
      MSHDIR=$OPTARG     # EurekGen output mesh file
      ;;
    x)
      MAT=0           # Do not delete intermediate mat file (optional)
      ;;
    \?)
      show_help >&2
      exit 1
      ;;
    esac
done
shift "$((OPTIND-1))" # Shift off the options and optional --.

# Check command line arguments
if [[ ( -z "$CONF" || -z "$MODEL" || -z "$MSHDIR" ) ]]
then
  show_help >&2
  exit 1
fi

# Check if gmsh binary is present
command -v ./gmsh >/dev/null 2>&1 || { echo >&2 "Gmsh binary needs to be in MeshGen directory! Exiting.."; exit 1; }

# Check if GeoGen and EurekaGen are built
command -v ./GeoGen/GeoGen >/dev/null 2>&1 || { echo >&2 "GeoGen needs to be built first! Type 'make'.."; exit 1; }
command -v ./EurekaGen/EurekaGen >/dev/null 2>&1 || { echo >&2 "EurekaGen needs to be built first! Type 'make'.."; exit 1; }

# Add trailing slash to mesh directory (if missing)
length=${#MSHDIR}
last_char=${MSHDIR:length-1:1}
[[ $last_char != "/" ]] && MSHDIR="$MSHDIR/";

# Create mesh dir (if missing)
mkdir -p $MSHDIR

printf -v GEO '%s%s.geo' "$MSHDIR" "$MODEL"
printf -v MSH '%s%s.msh' "$MSHDIR" "$MODEL"
printf -v DAT '%s%s.dat' "$MSHDIR" "$MODEL"

# Generate geo using GeoGen program
printf "\n-----------------------------"
printf "\nGeoGen: Generating geo file.."
printf "\n-----------------------------\n"
./GeoGen/GeoGen -f $CONF -o $GEO

# Exit if GeoGen failed
if [ $? -ne 0 ]
then
  exit 1
fi

# Generate msh using Gmsh program
printf "\n---------------------------"
printf "\nGmsh: Generating msh file.."
printf "\n---------------------------\n"
SECONDS=0
./gmsh $GEO -3 -o $MSH -optimize_threshold 0.6
duration=$SECONDS
printf "Time taken = $duration s\n"

# Exit if Gmsh failed
if [ $? -ne 0 ]
then
  exit 1
fi

# Generate dat using EurekaGen program
printf "\n--------------------------------"
printf "\nEurekaGen: Generating dat file.."
printf "\n--------------------------------\n"
./EurekaGen/EurekaGen -f $CONF -i $MSH -o $DAT

# Exit if EurekaGen failed
if [ $? -ne 0 ]
then
  exit 1
fi

# Remove intermediate material file
if [ $MAT -eq 1 ]
then
  rm -f GeoGen.mat
fi
#!/bin/bash

# Usage info
function show_help() {
cat << EOF
Usage: ${0##*/} [-h] [-c CONF -g GEO -m MSH -d DAT] [-x]
Generates Eureka format mesh file (.dat) using Gmsh as meshing-tool.
    -h      This help message.
    -c CONF GeoGen input config file.
    -g GEO  GeoGen output geo file.
    -m MSH  Gmsh output msh file.
    -d DAT  EurekGen output mesh file.
    -x      Do not delete intermediate mat file (optional).
EOF
}

# Parse command line arguments
OPTIND=1
MAT=1

while getopts "hc:g:m:d:x" opt; do
  case "$opt" in
    h)
      show_help       # Show help
      exit 0
      ;;
    c)
      CONF=$OPTARG    # GeoGen input config file
      ;;
    g)
      GEO=$OPTARG     # GeoGen output geo file
      ;;
    m)
      MSH=$OPTARG     # Gmsh output msh file
      ;;
    d)
      DAT=$OPTARG     # EurekGen output mesh file
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
if [[ ( -z "$CONF" || -z "$GEO" || -z "$MSH" || -z "$DAT" ) ]]
then
  show_help >&2
  exit 1
fi

# Check if gmsh binary is present
command -v ./gmsh >/dev/null 2>&1 || { echo >&2 "Gmsh binary needs to be in MeshGen directory! Exiting.."; exit 1; }

# Check if GeoGen and EurekaGen are built
command -v ./GeoGen/GeoGen >/dev/null 2>&1 || { echo >&2 "GeoGen needs to be built first! Type 'make'.."; exit 1; }
command -v ./EurekaGen/EurekaGen >/dev/null 2>&1 || { echo >&2 "EurekaGen needs to be built first! Type 'make'.."; exit 1; }

# Create mesh dir (if missing)
mkdir -p mesh

# Generate geo using GeoGen program
printf "\n-----------------------------"
printf "\nGeoGen: Generating geo file.."
printf "\n-----------------------------\n"
./GeoGen/GeoGen -f $CONF -o $GEO
if [ $? -ne 0 ]
then
  exit 1
fi

SECONDS=0
# Generate msh using Gmsh program
printf "\n---------------------------"
printf "\nGmsh: Generating msh file.."
printf "\n---------------------------\n"
./gmsh $GEO -3 -o $MSH
duration=$SECONDS
printf "Time taken = $duration s\n"
if [ $? -ne 0 ]
then
  exit 1
fi

# Generate dat using EurekaGen program
printf "\n--------------------------------"
printf "\nEurekaGen: Generating dat file.."
printf "\n--------------------------------\n"
./EurekaGen/EurekaGen -f $CONF -i $MSH -o $DAT
if [ $? -ne 0 ]
then
  exit 1
fi

# Remove intermediate material file
if [ $MAT -eq 1 ]
then
  rm -f GeoGen.mat
fi
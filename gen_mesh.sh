#!/bin/bash

# Usage info
function show_help() {
cat << EOF
Usage: ${0##*/} [-h] [-d] [-f GEOCONF -g OUTGEO -m OUTMSH -o EUREKADAT]
Generates Eureka format mesh file (.dat) using Gmsh as meshing-tool.
    -h           This help message.
    -d           Do not delete intermediate mat file (optional).
    -f GEOCONF   GeoGen input config file.
    -g OUTGEO    GeoGen output geo file.
    -m OUTMSH    Gmsh output msh file.
    -o EUREKADAT EurekGen output mesh file.
EOF
}

# Parse command line arguments
OPTIND=1
DELETEMAT=1
GEOCONF=""
OUTGEO=""
OUTMSH=""
EUREKADAT=""

while getopts "hdf:g:m:o:" opt; do
  case "$opt" in
    h)
      show_help                         # Show help
      return
      ;;
    d)
      DELETEMAT=0                       # Do not delete intermediate mat file
      ;;
    f)
      GEOCONF=$OPTARG                   # GeoGen input config file
      ;;
    g)
      OUTGEO=$OPTARG                    # GeoGen output geo file
      ;;
    m)
      OUTMSH=$OPTARG                    # Gmsh output msh file
      ;;
    o)
      EUREKADAT=$OPTARG                 # EurekGen output mesh file
      ;;
    \?)
      show_help >&2
      return
      ;;
    esac
done
shift "$((OPTIND-1))" # Shift off the options and optional --.

# Check command line arguments
if [[ ( -z "$GEOCONF" || -z "$OUTGEO" || -z "$OUTMSH" || -z "$EUREKADAT" ) ]]
then
  show_help >&2
  return
fi


# Create mesh dir (if missing)
mkdir -p mesh

# Generate geo using GeoGen program
printf "\n---------------------"
printf "\nGenerating geo file.."
printf "\n---------------------\n"
./GeoGen/GeoGen -f $GEOCONF -o $OUTGEO

SECONDS=0
# Generate msh using Gmsh program
printf "\n---------------------"
printf "\nGenerating msh file.."
printf "\n---------------------\n"
gmsh $OUTGEO -3 -o $OUTMSH
duration=$SECONDS
printf "Time taken = $duration s\n"

# Generate dat using EurekaGen program
printf "\n---------------------"
printf "\nGenerating dat file.."
printf "\n---------------------\n"
./EurekaGen/EurekaGen -f $GEOCONF -i $OUTMSH -o $EUREKADAT

# Remove intermediate material file
if [ $DELETEMAT -eq 1 ]
then
  rm -f GeoGen.mat
fi
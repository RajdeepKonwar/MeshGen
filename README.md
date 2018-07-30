# MeshGen
Generates Eureka format mesh file using [Gmsh](http://gmsh.info/) as meshing-tool. Gmsh is distributed under the terms of the GNU General Public License (GPL) which means that everyone is free to use Gmsh and to redistribute it on a free basis.

## Compilation instructions
```sh
$ make clean
$ make
```
This generates two programs: `GeoGen` and `EurekaGen`
* `GeoGen`: Takes in input config file and outputs geometry script (.geo) file
* `EurekaGen`: Takes in input Gmsh mesh (.msh) file and outputs Eureka format mesh (.dat) file

## Run instructions
```sh
$ . gen_mesh.sh -c ./conf/BrakePad.conf -g ./mesh/BrakePad.geo -m ./mesh/BrakePad.msh -d ./mesh/BrakePad.dat
```
The mesh generation script `gen_mesh.sh` takes in the following command-line arguments:

| Argument | Description                                         |
| -------- | --------------------------------------------------- |
| -h       | Help message                                        |
| -x       | Do not delete intermediate material file (optional) |
| -c CONF  | Input config file to both programs                  |
| -g GEO   | Output geometry script file from `GeoGen`           |
| -m MSH   | Output mesh file from `Gmsh`                        |
| -d DAT   | Output Eureka format mesh file from `EurekaGen`     |

The `GeoGen` program also generates an intermediate material (`GeoGen.mat`) file which is later used by the `EurekaGen` program to compute element groups for different materials. By default, the shell script removes this file after successful execution (i.e. after the .dat file is generated). One can choose to keep this material file for debugging purposes by including the command-line argument (-x) as shown below:
```sh
$ . gen_mesh.sh -x -c ./conf/BrakePad.conf -g ./mesh/BrakePad.geo -m ./mesh/BrakePad.msh -d ./mesh/BrakePad.dat
```

## Directory structure
`MeshGen` comprises of a rigid directory structure which is as follows:

| Directory | Description |
| --------- | ----------- |
| conf | Place all config files here |
| EurekaGen | Contains the `EurekaGen` program executable and source files |
| GeoGen | Contains the `GeoGen` program executable and source files |
| mesh | Stores all the mesh-related output files (.dat, .geo, .msh) |

Always try and use `mesh/` directory to store all output files, namely the (`-g`, `-m`, `-d`) options from the command-line arguments. The shell script will automatically create the `mesh/` directory if missing. If you wish to use your own directory, make sure to create it first and set the path correctly in order to avoid dire consequences!

## Input config file (.conf) format
NOTE: All units are in microns (μm). Lines starting with `#` in .conf file are comments.
The input config file (for example, `./conf/BrakePad.conf`) comprises of the following format:
##### Break-pad bounding box dimensions
Internally, we follow a cartesian coordinate system, where z points upward. Length (by default 10000.0) is measured in x-direction, width (by default 5000.0) in y-direction and height (by default 5500.0) in z-direction.
```
# Box dimension
length=10000.0
width=5000.0
height=5500.0
```
##### Global mesh-size
One can specify the global matrix mesh-size (by default 200.0) for Gmsh to follow while meshing. This will propagate to the material particles if their individual mesh-sizes are not specified (see `Material block` below).
```
# Global mesh-size
global_mesh_size=200.0
```
##### Tolerance between particles
Once can specify the tolerance-value between particles (by default 50.0), i.e. the minimum inter-particle distance while inserting each particle into the matrix.
```
# Tolerance between particles
tol_particles=50.0
```
##### Tolerance between particles and boundaries
Once can specify the tolerance-value between particles and the boundaries (by default 100.0) of the bounding box.
```
# Tolerance between particles and boundaries
tol_particles_boundaries=100.0
```
##### Piston thickness
There is a steel piston on the top of the bounding box whose thickness (by default 500.0) needs to be specified.
```
# Piston thickness
piston_thicc=500.0
```
##### Random seed
The output `.geo` file writes out the seed value used to initialize the randomizer on line 4.
```
/** Gmsh geometry script generated by GeoGen (author: Rajdeep Konwar)
 *  Copyright (c) 2018, Robert Bosch LLC
 *  Timestamp: Mon Jul 30 10:34:56 2018
 *  Rand seed: 1532972096
 **/
```
One can use that seed value for a different run by placing it in the config file (as shown below) to achieve the same placement and orientation of all the particles.
```
# Random seed
rand_seed=1532972096
```
If left blank, the randomizer will use the current system time as seed (default behavior).
##### Material block
The following table describe all aspects of a material block and the possible options and combinations:

| key-phrase | Description |
| ---------- | ----------- |
| material | Name of the material, used for element group names |
| vol_frac | Volume fraction in percentage (wrt the total volume) of the material |
| mesh_size | Individual mesh-size for the material. If left blank, will use the `global_mesh_size` value |
| morph | Morphology of the material particles. Available options are `cylinder` and `sphere` |
| rad_distrib | Probability distribution for radius (base-radius for cylinder whereas actual radius for sphere). Available options are `gaussian`/`gauss` and `uniform`/`flat` |
| rad_mean | Mean-value of radius. Only applicable for Gaussian distribution. `GeoGen` will output error if tried to use with Uniform distribution |
| rad_min rad_max | Minimum and maximum value of radius. Used only if `rad_mean` isn't specified (i.e. `rad_mean` has higher priority). Applicable for both types of distribution |
| rad_std_dev | Standard deviation for radius. Must be specified when using `rad_mean` (otherwise `GeoGen` will output error) whereas automatically computed from `rad_min` & `rad_max` |
| len_distrib | Probability distribution for length of cylinder. The following key-phrases are only applicable for `cylinder` morphology. Available options are `gaussian`/`gauss` and `uniform`/`flat` |
| len_mean | Mean-value of cylinder length. Only applicable for Gaussian distribution. `GeoGen` will output error if tried to use with Uniform distribution |
| len_min len_max | Minimum and maximum value of cylinder length. Used only if `len_mean` isn't specified (i.e. `len_mean` has higher priority). Applicable for both types of distribution |
| len_std_dev | Standard deviation for cylinder length. Must be specified when using `len_mean` (otherwise `GeoGen` will output error) whereas automatically computed from `len_min` & `len_max` |

One can add as many materials according to their use-case by placing each material in its own block as described by the above table (for reference, see `./conf/BrakePad.conf`). For example:
```
# Material 3
material=Friction Dust
vol_frac=16.0
mesh_size=
morph=sphere
rad_distrib=gaussian
rad_min=1500.0
rad_max=2000.0
rad_std_dev=100.0
```

## Material file (GeoGen.mat) format
As described above, the `GeoGen` program outputs an intermediate material file (`GeoGen.mat`) in `MeshGen`'s root directory which is used later by `EurekaGen` to help identify the material element groups. For debugging purposes, the material file format is as follows:
```
Name of the material
Morphology
Number of particles
```
Available options for `Morphology` are `cyl` (for cylinder) and `sph` (for sphere).
The following lines (corresponding to the number of particles) for `cylinder` morphology:
```
base-radius center1.x center1.y center1.z center2.x center2.y center2.z
```
where `center1` is the center of the left face and `center2` is the center of the right face of the cylinder.
Whereas for `sphere` morphology, we have:
```
radius center.x center.y center.z
```
where `center` is the center of the sphere. x, y, z are the cartesian coordinate values of a point.

## Notes on constants used in code
The code uses certain constants and typedefs which can be changed (although not recommended) according to the need of the user. The file `./GeoGen/GeoConstants.h` contains the iteration limit (by default 1000000) for the part where it tries to insert cylinder and sphere into the matrix while trying to avoid placing it out of the bounding box and avoid all types of collision detection. If the code fails to do so in the number of iterations specified above, it will quit the program by throwing an error message like:
```
Reached limit for iterative cylinder insertion! Exiting..
```

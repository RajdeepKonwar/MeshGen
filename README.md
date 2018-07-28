# MeshGen
Generates Eureka format mesh file using Gmsh as meshing-tool

## Compilation instruction
```
make clean
make
```

## Run instruction
. gen_mesh.sh -f ./conf/BrakePad.conf -g ./mesh/BrakePad.geo -m ./mesh/BrakePad.msh -o ./mesh/BrakePad.dat
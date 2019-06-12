/*____________________________________________________________________
|
| File: ReadOBJFile.h
|___________________________________________________________________*/

// Reads a single-mesh OBJ file, saving the data in a created Object3D
void ReadOBJFile  (
  char      *filename, 
  Object3D **object, 
  bool       load_texcoords,
  bool       smooth_discontinuous_vertices );

// Frees all data in a Object3D
void FreeObject(Object3D *object);

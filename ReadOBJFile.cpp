/*____________________________________________________________________
|
| File: ReadOBJFile.cpp
|
| Description: Function to read in 3D data from an OBJ file.  Assumes 
|   the OBJ file data was created for a RHS.  If data is in LHS then 
|   uncomment the code that performs the conversion as follows:
|     1) negate all Z coords
|     2) ordering polygons in counterclockface order (instead of clockwise)
|
| Functions: ReadOBJFile
|             strNumExists
|             Convert_Data
|             Convert_Data_With_Texcoords
|            FreeObject
|___________________________________________________________________*/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

/*___________________
|
| Include Files
|__________________*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math3d.h"
#include "ReadOBJFile.h"

/*___________________
|
| Type definitions
|__________________*/

struct SrcPolyVertex {
  int v;  // index into source vertex array
  int t;  // index into source textcoords array
};

struct SrcPoly {
  SrcPolyVertex vdata[3];   // a source poly is a triangle so it has 3 items of vertex data
};

/*___________________
|
| Function Prototypes
|__________________*/

static int strNumExists (char *str, char c);
static void Convert_Data (Object3D *object, bool smooth_discontinuous_vertices);
static void Convert_Data_With_Texcoords (Object3D *object, bool smooth_discontinuous_vertices);

/*___________________
|
| Global variables
|__________________*/

static int src_num_vertices;            // # of vertices in the OBJ file
static int src_num_texcoords;           // # of texture coords in the OBJ file (if any)
static int src_num_polys;               // # of polys in OBJ file  
static Vector3D *src_vertices;        // array of vertices read from file
static UVCoordinate *src_texcoords; // array of texcoords read from file
static SrcPoly *src_polys;              // array of polys read from file

/*____________________________________________________________________
|
| Function: ReadOBJFile
|
| Output: Reads a single-mesh OBJ file into a Object3D.
|___________________________________________________________________*/

void ReadOBJFile  (
  char      *filename, 
  Object3D **object, 
  bool       load_texcoords,
  bool       smooth_discontinuous_vertices )
{
  FILE *fp;
  char line[500];
  bool error = false; // set to true on any processing error

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  // Set object pointer to null in case of error reading file
  *object = 0;

  fp = 0;
  src_num_vertices = 0;
  src_num_texcoords = 0;
  src_num_polys = 0;
  src_vertices = 0;
  src_texcoords = 0;
  src_polys = 0;

/*____________________________________________________________________
|
| Open the file
|___________________________________________________________________*/

  // Open the fil
  fp = fopen(filename, "rt");
  if (NOT fp)
    error = true;

/*____________________________________________________________________
|
| Count number of data items in file
|___________________________________________________________________*/

  if (NOT error) {
    // Count # of vertices
    while (NOT feof(fp)) {
      line[0] = 0; // set first 2 characters in line array to 0
      line[1] = 0;
      fgets (line, 500, fp);      // try to read in a entire line from the file
      // Is this line a vertex?
      if (line[0] == 'v' AND line[1] == ' ')
        src_num_vertices++;
      // Is this line a texture coord?
      else if (line[0] == 'v' AND line[1] == 't')
        src_num_texcoords++;
      // Is this line a poly?
      else if (line[0] == 'f' AND line[1] == ' ')
        src_num_polys++;
    }
    // Reset file pointer to start of the file
    fseek (fp, 0, SEEK_SET);

/*____________________________________________________________________
|
| Error checking - Is needed data available in the file?
|___________________________________________________________________*/

    if (src_num_vertices == 0)  // we require vertices
      error = true;
    if (src_num_polys == 0)     // we require polys
      error = true;
    // Do we require texcoords?
    if (load_texcoords)
      // If so, then there should be some in the file
      if (src_num_texcoords == 0)
        error = true;
  }

/*____________________________________________________________________
|
| Allocate temp arrays for source data
|___________________________________________________________________*/
    
  if (NOT error) {
    // Allocate array of vertices
    src_vertices = (Vector3D *) calloc (src_num_vertices, sizeof(Vector3D));
    if (src_vertices == 0)
      error = true;

    // Allocate array of polys
    src_polys = (SrcPoly *) calloc (src_num_polys, sizeof(SrcPoly));
    if (src_polys == 0)
      error = true;

    // Allocate array of texcoords, if needed
    if (load_texcoords) {
      src_texcoords = (UVCoordinate *) calloc (src_num_texcoords, sizeof(UVCoordinate));
      if (src_texcoords == 0)
        error = true;
    }
  }

/*____________________________________________________________________
|
| Read in data from file
|___________________________________________________________________*/

  if (NOT error) {
      // Init indeces into arrays
      int v = 0, p = 0, t = 0;
      int tex0, tex1, tex2; 
      int vn0, vn1, vn2;

      while (NOT feof(fp)) {
        line[0] = 0; // set first 2 characters in line array to 0
        line[1] = 0;
        fgets (line, 500, fp);      // try to read in a entire line from the file
        // Is this line a vertex?
        if (line[0] == 'v' AND line[1] == ' ') {
          sscanf_s (line, "v %f %f %f", &(src_vertices[v].x), &(src_vertices[v].y), &(src_vertices[v].z));
          v++;
        }
        // Is this line a texture coord?
        else if (line[0] == 'v' AND line[1] == 't') {
          // Are we reading in texcoords?
          if (load_texcoords) {
            sscanf_s (line, "vt %f %f", &(src_texcoords[t].u), &(src_texcoords[t].v));
            t++;
          }
        }
        // Is this line a poly?
        else if (line[0] == 'f' AND line[1] == ' ') {
          // Select from 4 different formats
          if (strstr(line,"//")) {
            sscanf_s (line, "f %d//%d %d//%d %d//%d", &(src_polys[p].vdata[0].v), &vn0,  
                                                    &(src_polys[p].vdata[1].v), &vn1,
                                                    &(src_polys[p].vdata[2].v), &vn2);
          }
          else {
            switch (strNumExists(line, '/')) {
              case 0: sscanf_s (line, "f %d %d %d", &(src_polys[p].vdata[0].v), 
                                                  &(src_polys[p].vdata[1].v), 
                                                  &(src_polys[p].vdata[2].v));
                      break;
              case 3: sscanf_s (line, "f %d/%d %d/%d %d/%d", &(src_polys[p].vdata[0].v), &tex0,  
                                                           &(src_polys[p].vdata[1].v), &tex1,
                                                           &(src_polys[p].vdata[2].v), &tex2);
                      if (load_texcoords) {
                        src_polys[p].vdata[0].t = tex0;
                        src_polys[p].vdata[1].t = tex1;
                        src_polys[p].vdata[2].t = tex2;
                      }
                      break;
              case 6: sscanf_s (line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &(src_polys[p].vdata[0].v), &tex0, &vn0,  
                                                                    &(src_polys[p].vdata[1].v), &tex1, &vn1,
                                                                    &(src_polys[p].vdata[2].v), &tex2, &vn2);
                      if (load_texcoords) {
                        src_polys[p].vdata[0].t = tex0;
                        src_polys[p].vdata[1].t = tex1;
                        src_polys[p].vdata[2].t = tex2;
                      }
                      break;
            }
          }
          // Subtract one from all indeces read from the file since they are +1
          src_polys[p].vdata[0].v--;
          src_polys[p].vdata[1].v--;
          src_polys[p].vdata[2].v--;
          if (load_texcoords) {
            src_polys[p].vdata[0].t--;
            src_polys[p].vdata[1].t--;
            src_polys[p].vdata[2].t--;
          }
          p++;
        }
     }
  }

/*____________________________________________________________________
|
|  Convert the data read from the file into Object3D format
|___________________________________________________________________*/

  // Create an empty object
  *object = (Object3D *)calloc(1,sizeof(Object3D));

  // Add data to the object
  if  (load_texcoords)
    Convert_Data_With_Texcoords (*object,smooth_discontinuous_vertices);
  else
    Convert_Data (*object,smooth_discontinuous_vertices);

  /*____________________________________________________________________
  |
  |  Convert the data from LHS into RHS format (needed only if the OBJ
  |   file data was in LHS format)
  |___________________________________________________________________*/

  // Negate all z coords (for opengl RHS formatting)
  //for (int i = 0; i<(*object)->num_vertices; i++) {
  //  (*object)->vertex[i].z        = -(*object)->vertex[i].z;
  //  (*object)->vertex_normal[i].z = -(*object)->vertex_normal[i].z;
  //}

  //// Reorder polygon indeces from clockwise to counterclockwise order
  //unsigned short temp;
  //for(int i = 0; i<(*object)->num_polygons; i++) {
  //  temp = (*object)->polygon[i].index[1];
  //  (*object)->polygon[i].index[1] = (*object)->polygon[i].index[2];
  //  (*object)->polygon[i].index[2] = temp;
  //}

/*____________________________________________________________________
|
| Free resources 
|___________________________________________________________________*/

  // Free temp memory
  if (src_vertices)
    free (src_vertices);
  if (src_texcoords)
    free (src_texcoords);
  if (src_polys)
    free (src_polys);
  // Close the file
  if (fp)
    fclose (fp);
}

/*____________________________________________________________________
|
| Function: strNumExists
|
| Input: Called from ReadOBJFile()
| Output: Returns the number of occurences of a char in the ASCIIZ 
|   string.  Assumes str is not a null pointer.
|___________________________________________________________________*/

static int strNumExists (char *str, char c)
{
  int n;
  for (n=0; *str; str++)
    if (*str == c)
      n++;
  return (n);
}

/*____________________________________________________________________
|
| Function: Convert_Data
|
| Input: Called from ReadOBJFile()
| Output: Adds data to the Object3D.
|___________________________________________________________________*/

static void Convert_Data (Object3D *object, bool smooth_discontinuous_vertices)
{
  int i, j;

/*____________________________________________________________________
|
| Init variables in the object
|___________________________________________________________________*/

  object->num_vertices = src_num_vertices;
  object->num_polygons = src_num_polys;

/*____________________________________________________________________
|
| Allocate memory in the object layer
|___________________________________________________________________*/
  
  object->vertex         = (Vector3D *) malloc (object->num_vertices * sizeof(Vector3D));
  object->vertex_normal  = (Vector3D *) malloc (object->num_vertices * sizeof(Vector3D));
  object->polygon        = (Polygon3D*) malloc (object->num_polygons * sizeof(Polygon3D));
  object->polygon_normal = (Vector3D *) malloc (object->num_polygons * sizeof(Vector3D));

/*____________________________________________________________________
|
| Copy data
|___________________________________________________________________*/

  // Copy vertex data
  for (i=0; i<src_num_vertices; i++) 
    object->vertex[i] = src_vertices[i];

  // Copy polygon data
  for (i=0; i<src_num_polys; i++) 
    for (j=0; j<3; j++)
      object->polygon[i].index[j] = src_polys[i].vdata[j].v;

/*____________________________________________________________________
|
| Calculate normals
|___________________________________________________________________*/

  // Calculate polygon normals
  for (i=0; i<src_num_polys; i++) 
    SurfaceNormal (&(object->vertex[object->polygon[i].index[0]]),
                   &(object->vertex[object->polygon[i].index[1]]),
                   &(object->vertex[object->polygon[i].index[2]]),
                   &(object->polygon_normal[i]));
  
  // Calculate vertex normals
  ComputeVertexNormals (object,smooth_discontinuous_vertices);
}

/*____________________________________________________________________
|
| Function: Convert_Data_With_Texcoords
|
| Input: Called from ReadOBJFile()
| Output: Adds data to the Object3D.
|___________________________________________________________________*/

static void Convert_Data_With_Texcoords (Object3D *object, bool smooth_discontinuous_vertices)
{
   int i, j, k, max_gx_vertices, num_gx_vertices;
   bool found;
   Vector3D *tmp_gx_vertices = 0;
   UVCoordinate *tmp_gx_texcoords = 0;
   SrcPolyVertex *tmp_gx_vertices_sig = 0; // a vertex signature is the combination of vertex index and texcoord index

/*____________________________________________________________________
|
| Init variables in the object
|___________________________________________________________________*/

  //object->has_texcoords = load_texcoords;
  object->num_polygons = src_num_polys;

/*____________________________________________________________________
|
| Allocate memory in the object layer
|___________________________________________________________________*/
  
  object->polygon        = (Polygon3D*) malloc (object->num_polygons * sizeof(Polygon3D));
  object->polygon_normal = (Vector3D *) malloc (object->num_polygons * sizeof(Vector3D));

/*____________________________________________________________________
|
|  Allocate temp array of vertices (not sure yet how many distinct
|  vertices are in the model)
|___________________________________________________________________*/

  // Calculate the max number of distinct vertices in this model
  max_gx_vertices = src_num_polys * 3;
  // Allocate memory for a temp array of vertices
  tmp_gx_vertices = (Vector3D *) calloc (max_gx_vertices, sizeof(Vector3D));
  // Allocate memory for a temp array of texcoords
  tmp_gx_texcoords = (UVCoordinate *) calloc (max_gx_vertices, sizeof(UVCoordinate));
  // Allocate an equal size temp array of vertex 'signatures'
  tmp_gx_vertices_sig = (SrcPolyVertex *) calloc (max_gx_vertices, sizeof(SrcPolyVertex *));

/*____________________________________________________________________
|
|  Build temp arrays of vertices and texcoords
|___________________________________________________________________*/

  num_gx_vertices = 0;  // no distinct vertices identified so far
  // Look at each poly
  for (i=0; i<src_num_polys; i++) {
    // Look at the 3 vertices that make up this poly
    for (j=0; j<3; j++) {
      // See if this vertex has been identified already
      found = false;
      for (k=0; k<num_gx_vertices; k++) 
        if (memcmp((void*)&(src_polys[i].vdata[j]), (void*)&(tmp_gx_vertices_sig[k]), sizeof(SrcPolyVertex)) == 0) {
          found = true;
          break;
        }
      // If found, just instance it - this is not a newly identified distinct vertex
      if (found)
        object->polygon[i].index[j] = k;
      // Otherwise, create a new gx vertex
      else {
        // Trying to create too much?  (should never happen)
        if (num_gx_vertices == max_gx_vertices)
          exit(0);
        else  {
          tmp_gx_vertices    [num_gx_vertices] = src_vertices [src_polys[i].vdata[j].v];
          tmp_gx_texcoords   [num_gx_vertices] = src_texcoords[src_polys[i].vdata[j].t];
          tmp_gx_vertices_sig[num_gx_vertices] = src_polys[i].vdata[j];
          object->polygon[i].index[j] = num_gx_vertices;
          num_gx_vertices++;
        }
      }
    }
  }

/*____________________________________________________________________
|
| Copy the temp data into the object layer
|___________________________________________________________________*/
  
  object->num_vertices  = num_gx_vertices;
  object->vertex        = (Vector3D *)     malloc (num_gx_vertices * sizeof(Vector3D));
  object->vertex_normal = (Vector3D *)     malloc (num_gx_vertices * sizeof(Vector3D));
  object->tex_coords    = (UVCoordinate *) malloc (num_gx_vertices * sizeof(UVCoordinate));
  memcpy ((void *)(object->vertex),     (void *)tmp_gx_vertices,  num_gx_vertices * sizeof(Vector3D));
  memcpy ((void *)(object->tex_coords), (void *)tmp_gx_texcoords, num_gx_vertices * sizeof(UVCoordinate));

/*____________________________________________________________________
|
| Calculate normals
|___________________________________________________________________*/

  // Calculate polygon normals
  for (i=0; i<src_num_polys; i++) 
    SurfaceNormal (&(object->vertex[object->polygon[i].index[0]]),
                   &(object->vertex[object->polygon[i].index[1]]),
                   &(object->vertex[object->polygon[i].index[2]]),
                   &(object->polygon_normal[i]));
  
  // Calculate vertex normals
  ComputeVertexNormals (object,smooth_discontinuous_vertices);

/*____________________________________________________________________
|
| Free resources
|___________________________________________________________________*/

  if (tmp_gx_vertices)
    free (tmp_gx_vertices);
  if (tmp_gx_texcoords)
    free (tmp_gx_texcoords);
  if (tmp_gx_vertices_sig)
    free (tmp_gx_vertices_sig);
}

/*____________________________________________________________________
|
| Function: FreeObject
|
| Output: Frees all memory allocated in an Object3D.
|___________________________________________________________________*/

void FreeObject (Object3D *object)
{
  if (object) {
    if (object->vertex)
      free (object->vertex);
    if (object->vertex_normal)
      free (object->vertex_normal);
    if (object->tex_coords)
      free (object->tex_coords);
    if (object->polygon)
      free (object->polygon);
    if(object->polygon_normal)
      free(object->polygon_normal);
    free (object);
  }
}

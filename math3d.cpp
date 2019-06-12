/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
| math3d.cpp
|
| Description: Math functions needed in typical 3D graphics programs.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "math3d.h"

/*___________________
|
| Macros
|__________________*/

// Needed here since the default library doesn't support float versions of these functions
#define sinf(_expr_) ((float)sin((double)(_expr_)))
#define cosf(_expr_) ((float)cos((double)(_expr_)))
#define tanf(_expr_) ((float)tan((double)(_expr_)))
#define asinf(_expr_) ((float)asin((double)(_expr_)))
#define acosf(_expr_) ((float)acos((double)(_expr_)))
#define atanf(_expr_) ((float)atan((double)(_expr_)))
#define sqrtf(_expr_) ((float)sqrt((double)(_expr_)))
#define absf(_expr_) ((float)fabs((double)(_expr_)))
#define logf(_expr_)  ((float)log((double)(_expr_)))
#define log10f(_expr_) ((float)log10((double)(_expr_)))
#define fabsf(_expr_) ((_expr_) >=0 ? (_expr_) : -(_expr_))

// Assert macro (caller needs to include stdio.h for this to work)
#define DEBUG_ASSERT(_assert_stuff_)                            \
  if (!(_assert_stuff_)) {                                      \
    char str[256];                                              \
    sprintf_s (str, "<ASSERT> line %d, %s", __LINE__, __FILE__); \
    assert (_assert_stuff_);                                    \
  }

#define IDENTITY_MATRIX(_m_)            \
  {                                     \
    memset (_m_, 0, 16*sizeof(float));  \
    _m_->_00 = 1;                       \
    _m_->_11 = 1;                       \
    _m_->_22 = 1;                       \
    _m_->_33 = 1;                       \
  }      

/*************************************************************************************
| Function: MultiplyMatrix
|
| Description: Multiplies m1 * m2, putting result in mresult.
*************************************************************************************/
void MultiplyMatrix(Matrix3D *m1,Matrix3D *m2,Matrix3D *mresult)
{
  int i,j,k;
  Matrix3D mtemp;

  // Verify input params
  DEBUG_ASSERT(m1);
  DEBUG_ASSERT(m2);
  DEBUG_ASSERT(mresult);

  // Multiply
  memset((void *)&mtemp,0,16 * sizeof(float));
  for(i = 0; i<4; i++)
    for(j = 0; j<4; j++)
      for(k = 0; k<4; k++)
        ((float *)&mtemp)[i * 4 + j] += (((float *)m1)[i * 4 + k] * ((float *)m2)[k * 4 + j]);

  // Put result into mresult
  *mresult = mtemp;
}

/*************************************************************************************
| Function: MultiplyVectorMatrix
|
| Description:  Multiplies v * m, putting result in vresult.  v and vresult
|   are 1x3 vectors. Assumes a matrix in Column-major order (used in a Right Handed
|   Coordinate System like OpenGL).
*************************************************************************************/
void MultiplyVectorMatrix(Vector3D *v,Matrix3D *m,Vector3D *vresult)
{
  Vector3D vorig;

  // Verify input params
  DEBUG_ASSERT(v);
  DEBUG_ASSERT(m);
  DEBUG_ASSERT(vresult);

  // Make a copy of v in case v and vresult are the same  
  vorig.x = v->x;
  vorig.y = v->y;
  vorig.z = v->z;

  vresult->x = vorig.x * m->_00 +
    vorig.y * m->_01 +
    vorig.z * m->_02 +
    m->_03;
  vresult->y = vorig.x * m->_10 +
    vorig.y * m->_11 +
    vorig.z * m->_12 +
    m->_13;
  vresult->z = vorig.x * m->_20 +
    vorig.y * m->_21 +
    vorig.z * m->_22 +
    m->_23;
}

/*************************************************************************************
| Function: GetIdentityMatrix
|
| Description:  Sets m to the identity matrix.
*************************************************************************************/
inline void GetIdentityMatrix(Matrix3D *m)
{
  IDENTITY_MATRIX(m)
}

/*************************************************************************************
| Function: GetTranslateMatrix
|
| Description:  Sets m to the translation matrix. Assumes a matrix in Column-major order
|   (used in a Right Handed Coordinate System like OpenGL).
*************************************************************************************/
void GetTranslateMatrix(Matrix3D *m,float tx,float ty,float tz)
{
  // Verify input params
  DEBUG_ASSERT(m);

  IDENTITY_MATRIX(m)
    m->_03 = tx;
  m->_13 = ty;
  m->_23 = tz;
}

/*************************************************************************************
| Function: GetScaleMatrix
|
| Description:  Sets m to the scaling matrix.
*************************************************************************************/
void GetScaleMatrix(Matrix3D *m,float sx,float sy,float sz)
{
  // Verify input params
  DEBUG_ASSERT(m);

  IDENTITY_MATRIX(m)
    m->_00 = sx;
  m->_11 = sy;
  m->_22 = sz;
}

/*************************************************************************************
| Function: GetRotateXMatrix
|
| Description:  Sets m to the x-axis rotation matrix. Assumes a matrix in Column-major
|   order (used in a Right Handed Coordinate System like OpenGL).
*************************************************************************************/
void GetRotateXMatrix(Matrix3D *m,float degrees)
{
  float s,c;

  // Verify input params
  DEBUG_ASSERT(m);

  // Create rotate matrix
  s = sinf(degrees * DEGREES_TO_RADIANS);
  c = cosf(degrees * DEGREES_TO_RADIANS);
  IDENTITY_MATRIX(m)
    m->_11 = c;
  m->_12 = -s;
  m->_21 = s;
  m->_22 = c;
}

/*************************************************************************************
| Function: GetRotateYMatrix
|
| Description:  Sets m to the x-axis rotation matrix. Assumes a matrix in Column-major
|   order (used in a Right Handed Coordinate System like OpenGL).
*************************************************************************************/
void GetRotateYMatrix(Matrix3D *m,float degrees)
{
  float s,c;

  // Verify input params
  DEBUG_ASSERT(m);

  // Create rotate matrix
  s = sinf(degrees * DEGREES_TO_RADIANS);
  c = cosf(degrees * DEGREES_TO_RADIANS);
  IDENTITY_MATRIX(m)
    m->_00 = c;
  m->_02 = s;
  m->_20 = -s;
  m->_22 = c;
}

/*************************************************************************************
| Function: GetRotateZMatrix
|
| Description:  Sets m to the z-axis rotation matrix. Assumes a matrix in Column-major
|   order (used in a Right Handed Coordinate System like OpenGL).
*************************************************************************************/
void GetRotateZMatrix(Matrix3D *m,float degrees)
{
  float s,c;

  // Verify input params
  DEBUG_ASSERT(m);

  // Create rotate matrix
  s = sinf(degrees * DEGREES_TO_RADIANS);
  c = cosf(degrees * DEGREES_TO_RADIANS);
  IDENTITY_MATRIX(m)
    m->_00 = c;
  m->_10 = s;
  m->_01 = -s;
  m->_11 = c;
}

/*************************************************************************************
| Function: SurfaceNormal
|
| Output: Using 3 points, computes the normal vector, if possible.
|   Returns true if normal computed successfully else false if points
|   are collinear (and therefore cannot define a plane).
|
|   Assumes points are given in a counterwise order (for right-handed coordinate system
|   like OpenGL).
*************************************************************************************/
bool SurfaceNormal(Vector3D *p1,Vector3D *p2,Vector3D *p3,Vector3D *normal)
{
  Vector3D a,b;
  bool success = false;

  // Verify input params
  DEBUG_ASSERT(p1);
  DEBUG_ASSERT(p2);
  DEBUG_ASSERT(p3);
  DEBUG_ASSERT(normal);

  // Compute 2 vectors from 3 points
  SubtractVector(p2,p1,&a);
  SubtractVector(p3,p1,&b);

  // Compute cross product of 2 vectors
  VectorCrossProduct(&a,&b,normal);

  // Are 3 points non-collinear?
  if((normal->x != 0) OR(normal->y != 0) OR(normal->z != 0)) {
    // Normalize the result
    NormalizeVector(normal,normal);
    success = true;
  }

  return (success);
}

/*************************************************************************************
| Function: AddVector
|
| Output: Computes vresult = v1 + v2.
*************************************************************************************/
void AddVector(Vector3D *v1,Vector3D *v2,Vector3D *vresult)
{
  // Verify input params
  DEBUG_ASSERT(v1);
  DEBUG_ASSERT(v2);
  DEBUG_ASSERT(vresult);

  vresult->x = v1->x + v2->x;
  vresult->y = v1->y + v2->y;
  vresult->z = v1->z + v2->z;
}

/*************************************************************************************
| Function: SubtractVector
|
| Output: Computes vresult = v1 - v2.  This is useful to compute the
|   displacement from a to b, then compute b - a.
*************************************************************************************/
inline void SubtractVector(Vector3D *v1,Vector3D *v2,Vector3D *vresult)
{
  // Verify input params
  DEBUG_ASSERT(v1);
  DEBUG_ASSERT(v2);
  DEBUG_ASSERT(vresult);

  vresult->x = v1->x - v2->x;
  vresult->y = v1->y - v2->y;
  vresult->z = v1->z - v2->z;
}

/*************************************************************************************
| Function: NormalizeVector
|
| Output: Normalizes a vector, returning result in vnormal vector.
*************************************************************************************/
inline void NormalizeVector(Vector3D *v,Vector3D *normal)
{
  float m,magnitude;

  // Verify input params
  DEBUG_ASSERT(v);
  DEBUG_ASSERT(normal);

  // Get the magnitude of the vector 
  magnitude = VectorMagnitude(v);

  // Compute the normal vector
  if(magnitude == 0)
    *normal = *v;
  else {
    m = 1 / magnitude;
    MultiplyScalarVector(m,v,normal);
  }
}

/*************************************************************************************
| Function: VectorMagnitude
|
| Output: Returns the magnitude of the vector (the length).
*************************************************************************************/
inline float VectorMagnitude(Vector3D *v)
{
  // Verify input params
  DEBUG_ASSERT(v);

  return (sqrtf(v->x*v->x + v->y*v->y + v->z*v->z));
}

/*************************************************************************************
| Function: MultiplyScalarVector
|
| Output: Multiplies a vector by a scalar, returning result in vresult
|   vector.  Has the effect of scaling the length of the vector by s.
|   Also, if s < 0, the direction of the vector is flipped.
*************************************************************************************/
inline void MultiplyScalarVector(float s,Vector3D *v,Vector3D *vresult)
{
  // Verify input params
  DEBUG_ASSERT(v);
  DEBUG_ASSERT(vresult);

  vresult->x = v->x * s;
  vresult->y = v->y * s;
  vresult->z = v->z * s;
}

/*************************************************************************************
| Function: VectorCrossProdcut
|
| Output: Computes the cross product of two vectors.  If the two input
|   vectors are parallel the result is undefined.  The result vector
|   is a vector perpendicular to the two input vectors.  A vector
|   pointing in the opposite direction can be obtained by passing the
|   two input vectors switched - v2, v1 instead of v1, v2.
|
|   In a RHS, a x b points toward you if the vectors a,b make a counterclockwise
|   turn from your viewpoint.  If clockwise, a x b points away
|   from you.
|
|   Put tail of b at head of a to judge clockwise or counterclockwise
|   orientation.
*************************************************************************************/
inline void VectorCrossProduct(Vector3D *v1,Vector3D *v2,Vector3D *vresult)
{
  Vector3D v;

  // Verify input params
  DEBUG_ASSERT(v1);
  DEBUG_ASSERT(v2);
  DEBUG_ASSERT(vresult);

  v.x = v1->y * v2->z - v1->z * v2->y;
  v.y = v1->z * v2->x - v1->x * v2->z;
  v.z = v1->x * v2->y - v1->y * v2->x;

  *vresult = v;
}

/*************************************************************************************
| Function: ComputeVertexNormals
|
| Output: Computes and sets all vertex normals for a 3D object. Returns true on success, 
|   else false on any error.
|
| Description:
|   Flags that can be used:
|     smooth_discontinuous_vertices
|         Polygons that aren't attached get averaged in when computing
|         the vertex normals if vertices have the same position.
|
|       The default is any polygon that is adjacent to a vertex gets
|       averaged in when computing a vertex normal.  In some cases
|       polygons have separate vertices but the vertices are located
|       at the same coordinate so the polygon is counted for smoothing
|       purposes.
|
| Notes: Allocates memory for the vertex_normal array if needed.
|
*************************************************************************************/
bool ComputeVertexNormals (Object3D *object,bool smooth_discontinuous_vertices)
{
  int i,j,k,poly_count;
  float f;
  bool error = false;

  // Verify input params
  DEBUG_ASSERT(object);

  // Allocate memory for vertex normal array?
  if(object->vertex_normal == NULL) {
    object->vertex_normal = (Vector3D *)malloc(object->num_vertices * sizeof(Vector3D));
    if(object->vertex_normal == NULL)
      error = true;
  }

  // Compute a vertex normal for each vertex
  for(i = 0; (i<object->num_vertices) AND(NOT error); i++) {
    // Init variables;
    object->vertex_normal[i].x = 0;
    object->vertex_normal[i].y = 0;
    object->vertex_normal[i].z = 0;
    poly_count = 0;

    if(smooth_discontinuous_vertices) {
      // Search each polygon to see if adjacent to this vertex (has a vertex with the same value as the vertex)
      for(j = 0; j<object->num_polygons; j++)
        for(k = 0; k<3; k++)
          if((!memcmp(&(object->vertex[object->polygon[j].index[k]]),&(object->vertex[i]),sizeof(Vector3D))) OR
             (!memcmp(&(object->vertex[object->polygon[j].index[k]]),&(object->vertex[i]),sizeof(Vector3D))) OR
             (!memcmp(&(object->vertex[object->polygon[j].index[k]]),&(object->vertex[i]),sizeof(Vector3D)))) {
            object->vertex_normal[i].x += object->polygon_normal[j].x;
            object->vertex_normal[i].y += object->polygon_normal[j].y;
            object->vertex_normal[i].z += object->polygon_normal[j].z;
            poly_count++;
            break;
          }
    }
    else {
      // Search each polygon to see if directly connected to this vertex (search each polygon for vertex i)
      for(j = 0; j<object->num_polygons; j++)
        for(k = 0; k<3; k++)
          if(object->polygon[j].index[k] == i) {
            object->vertex_normal[i].x += object->polygon_normal[j].x;
            object->vertex_normal[i].y += object->polygon_normal[j].y;
            object->vertex_normal[i].z += object->polygon_normal[j].z;
            poly_count++;
            break;
          }
    }
    // Compute the normal   
    if(poly_count) {
      f = (float)1 / (float)poly_count;
      object->vertex_normal[i].x *= f;
      object->vertex_normal[i].y *= f;
      object->vertex_normal[i].z *= f;
    }
    // Normalize to get the vertex normal
    NormalizeVector(&(object->vertex_normal[i]),&(object->vertex_normal[i]));
  }

  // Verify output params
  DEBUG_ASSERT(NOT error);

  return (NOT error);
}

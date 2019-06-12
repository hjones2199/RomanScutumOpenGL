/*____________________________________________________________________
|
| File: math3d.h
|___________________________________________________________________*/

/*___________________
|
| Constants
|__________________*/

#define PI                  ((float) 3.141592654)
#define RADIANS_TO_DEGREES  ((float) ((float)180/PI))
#define DEGREES_TO_RADIANS  ((float) (PI/(float)180))
#define AND &&
#define OR ||
#define NOT !

/*___________________
|
| Type definitions
|__________________*/

struct Vector3D { float x,y,z; };

struct Matrix3D { // 3D graphics requires 4x4 matrices (2D only requires 3x3)
  float _00,_01,_02,_03;
  float _10,_11,_12,_13;
  float _20,_21,_22,_23;
  float _30,_31,_32,_33;
};

// A triangle consists of 3 vertices (points)
struct Triangle3D {
  Vector3D v0,n0; // Vertex x,y,z followed by Normal x,y,z
  Vector3D v1,n1;
  Vector3D v2,n2;
};

struct Polygon3D {
  unsigned short index[3];  // each of these 3 is an index into the vertex array
};

struct UVCoordinate {
  float u,v;
};

// 3D object data structure - for one 3D object
struct Object3D {
  int num_vertices;
  int num_polygons;

  Vector3D     *vertex;
  Vector3D     *vertex_normal;
  UVCoordinate *tex_coords;
  
  Polygon3D *polygon;
  Vector3D  *polygon_normal;
};

/*___________________
|
| Function prototypes
|__________________*/

void MultiplyMatrix(Matrix3D *m1,Matrix3D *m2,Matrix3D *mresult);
void MultiplyVectorMatrix(Vector3D *v,Matrix3D *m,Vector3D *vresult);
inline void GetIdentityMatrix(Matrix3D *m);
void GetTranslateMatrix(Matrix3D *m,float tx,float ty,float tz);
void GetScaleMatrix(Matrix3D *m,float sx,float sy,float sz);
void GetRotateXMatrix(Matrix3D *m,float degrees);
void GetRotateYMatrix(Matrix3D *m,float degrees);
void GetRotateZMatrix(Matrix3D *m,float degrees);

bool SurfaceNormal(Vector3D *p1,Vector3D *p2,Vector3D *p3,Vector3D *normal);
void AddVector(Vector3D *v1,Vector3D *v2,Vector3D *vresult);
inline void SubtractVector(Vector3D *v1,Vector3D *v2,Vector3D *vresult);
inline void NormalizeVector(Vector3D *v,Vector3D *normal);
inline float VectorMagnitude(Vector3D *v);
inline void MultiplyScalarVector(float s,Vector3D *v,Vector3D *vresult);
inline void VectorCrossProduct(Vector3D *v1,Vector3D *v2,Vector3D *vresult);
bool ComputeVertexNormals(Object3D *object,bool smooth_discontinuous_vertices);

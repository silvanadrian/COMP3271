//////////////////////////////////////////////////////////////////////
// Code.h
// declare functions called by main program

//////////////////////////////////////////////////
//Global variables defined in other files


//////////////////////////////////////////////////////////////////////
//The struct Triangle
typedef struct tagTriangle{
	GLdouble vertices[3][2];
	GLdouble matrix[3][3];
	GLfloat  color[3];
//public:
//	Triangle(void){};
//	Triangle(GLdouble vtc[][2]);
} Triangle;



extern vector<Triangle> triangles;

extern Triangle triangle_to_draw;
extern int point_count;

extern GLfloat color_array[][3];

extern int recursion_depth;
//////////////////////////////////////////////////


void ConstructiveFractals();

void RecursiveFractal(int k);

void DrawTriangles();

void MouseInteraction(GLdouble mx, GLdouble my);

void AffineMatricesCalculation(GLdouble v_original[][2],GLdouble v_transformed[][2],GLdouble matrix[][3]);

void InverseMatrix(GLdouble original_m[][3],GLdouble inverse_m[][3]);
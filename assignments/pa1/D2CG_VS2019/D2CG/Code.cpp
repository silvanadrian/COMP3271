//////////////////////////////////////////////////////////////////////
// opengl.cpp

#include "stdafx.h"

#include "Code.h"

#include "D2CG.h"


//////////////////////////////////////////////////////
////////////
//The Global Variables To Be Used

//list of triangles, each element is of type Triangle
//to access the kth triangle, just use triangles[k]
//to get the number of triangles in the list, use triangles.size()
vector<Triangle> triangles;

// a triangle object for temporary storage of points
Triangle triangle_to_draw;
// count the number of points specified in this triangle
int point_count = 0;

// depth of recursion for IFS
// inital value is 8
// change the initial value here
int recursion_depth = 8;


// color array for triangles
// size is 11. So color_index should range from 0 to 10 for triangles.
GLfloat color_array[][3] = {
	{0.9,0,0}, //red
	{0,0.5,0.4},
	{0.1,0.2,0.46},
	{0.9,0.9,0},
	{0,1.0,0},
	{0,1.0,1.0},
	{0,0,1.0},
	{1.0,0,1.0},
	{0.9,0.6,0},
	{0.9,1.0,0.6},
	{0.2,0.2,0.2}
};

///////////////////////////////////////////////////////////////////



#ifdef _DEBUG //for debug builds only
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////////////////
//This function is called to draw triangles in the Triangles window.

void DrawTriangles() {

	//uncomment this line to unfill triangles
	//glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

	//sample code to draw vetices of triangle
	glColor3f(1.0, 1.0, 1.0);
	glPointSize(4);
	glBegin(GL_POINTS);
	for (int i = 0; i < point_count; i++)
		glVertex2d(triangle_to_draw.vertices[i][0], triangle_to_draw.vertices[i][1]);
	glEnd();

	//TO DO: Add code to draw triangles here. Use triangles.size() to get number of triangles.
	//Use triangles[i] to get the ith triangle.
	int size = triangles.size();
	if (size > 0) {
		AffineMatricesCalculation(triangles[0].vertices, triangle_to_draw.vertices, triangle_to_draw.matrix);
	}
	for (int count = 0; count < size;count++) {
		AffineMatricesCalculation(triangles[0].vertices, triangles[count].vertices, triangles[count].matrix);
		glColor3f(color_array[count][0], color_array[count][1], color_array[count][2]);
		glBegin(GL_TRIANGLES);
		Triangle t = triangles[count];
		glVertex2d(triangles[count].vertices[0][0], triangles[count].vertices[0][1]);
		glVertex2d(triangles[count].vertices[1][0], triangles[count].vertices[1][1]);
		glVertex2d(triangles[count].vertices[2][0], triangles[count].vertices[2][1]);
		glEnd();
	}
}




/////////////////////////////////////////////////////////////////////////////////////////
// This function invokes RecursiveFractal()

void ConstructiveFractals() {

	if (triangles.size() < 2)
		return;


	glColor3f(1.0, 1.0, 0.0);
	RecursiveFractal(recursion_depth);

}


/////////////////////////////////////////////////////////////////////////////////////////
//This function draws the factal in a recursion manner. The depth of recursion is k.
//When k=0, the function draw the most basic shape, that is, the original triangle, and 
//stops recursion.

void RecursiveFractal(int k) {

	//TO DO: Add code to implement the IFS method to draw fractal.
	//Use triangles.size() to get number of triangles.
	//Use triangles[i] to get the ith triangle.
	//The fields of struct Triangle include:
	//	GLdouble vertices[3][2];
	//	GLdouble matrix[3][3];	
	glMatrixMode(GL_MODELVIEW);
	int size = triangles.size();
	if (k > 0) {
		for (int count = 0; count < size;count++) {
			glPushMatrix();
			Triangle tri = triangles[count];
			GLdouble m[16] = {
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0 };

			m[0] = tri.matrix[0][0];
			m[1] = tri.matrix[1][0];
			m[3] = tri.matrix[2][0];

			m[4] = tri.matrix[0][1];
			m[5] = tri.matrix[1][1];
			m[7] = tri.matrix[2][1];

			m[12] = tri.matrix[0][2];
			m[13] = tri.matrix[1][2];
			m[15] = tri.matrix[2][2];
			glMultMatrixd(m);
			RecursiveFractal(k - 1);
			glPopMatrix();
		}
	}
	else {
		glBegin(GL_TRIANGLES);
		Triangle t = triangles[0];
		glVertex2d(triangles[0].vertices[0][0], triangles[0].vertices[0][1]);
		glVertex2d(triangles[0].vertices[1][0], triangles[0].vertices[1][1]);
		glVertex2d(triangles[0].vertices[2][0], triangles[0].vertices[2][1]);
		glEnd();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//This function is called to handle mouse left click events.
//m_x and m_y are the x and y coordinates of the clicked point in OpenGL coordinate system.

void MouseInteraction(GLdouble m_x, GLdouble m_y) {

	//TO DO: Store the point temporarily into the variable triangle_to_draw.
	// When 3 points are specified, we get a new triangle.
	// Compute the matrix for affine transformation from the first triangle to this one
	//	by invoking AffineMatricesCalculation().
	// Store both the points and matrix of the triangle into a new element of the list 'triangles'
	//
	triangle_to_draw.vertices[point_count][0] = m_x;
	triangle_to_draw.vertices[point_count][1] = m_y;
	point_count += 1;
	if (point_count > 2) {
		triangles.push_back(triangle_to_draw);
		point_count = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//This function is a tool function that computes matrix for the affine transformation from one original
//triangle to another triangle.
//Input: 
//		 v_original[][2]	the pointer to an array containing data of original triangle
//		 v_transformed[][2]	the pointer to an array containing data of triangle obtained by transforming original triangle
//Output:
//		 matrix[][3]		a pointer to 3x3 matrix that is the affine transformation that changes original triangle to the later one.

void AffineMatricesCalculation(GLdouble v_original[][2], GLdouble v_transformed[][2], GLdouble matrix[][3]) {

	//TO DO: Compute the affine transformation matrix that transforms triangle specified in v_original to the one specified in v_transformed.
	//		 Base the computation on the formula M = T'T^(-1), where T' is the 3x3 matrix with each column the homogeneous coordinates of transformed triangle's vertex
	//		 and T is 3x3 matrix orginazed in a similar manner but stores data of the original triangle.
	//		 If you do not want to calculate the inverse of T yourself, we provide a tool function InverseMatrix(). This function could compute the inverse of T.

	GLdouble T[3][3];
	// Assign the vectors to matrix
	T[0][0] = v_original[0][0];
	T[0][1] = v_original[0][2];
	T[0][2] = v_original[0][3];

	T[1][0] = v_original[1][0];
	T[1][1] = v_original[1][1];
	T[1][2] = v_original[1][2];
	T[2][0] = 1;
	T[2][1] = 1;
	T[2][2] = 1;

	GLdouble T2[3][3];
	T2[0][0] = v_transformed[0][0];
	T2[0][1] = v_transformed[0][2];
	T2[0][2] = v_transformed[0][3];

	T2[1][0] = v_transformed[1][0];
	T2[1][1] = v_transformed[1][1];
	T2[1][2] = v_transformed[1][2];

	T2[2][0] = 1;
	T2[2][1] = 1;
	T2[2][2] = 1;

	GLdouble TInv[3][3];
	InverseMatrix(T2, TInv);

	// Matrix calculation
	for (int i = 0; i < 3; i++)
	{
		for (int k = 0; k < 3; k++)
		{
			matrix[i][k] = 0;

			for (int t = 0; t < 3; t++)
				matrix[i][k] += T[i][t] * TInv[t][k];
		}
	}

	// matrix = T * T2

}


////////////////////////////////////////////////////////////////////////////////////////////////////////
// A routine to calculate inverse matrix of a 3x3 matrix which has all its values in the third row being 1.
//	original_m: 3x3 matrix with original_m[2][0]=original_m[2][1]=original_m[2][2]=1.
//  inverse_m:  3x3 matrix, the inverse of original_m.
//
void InverseMatrix(GLdouble original_m[][3], GLdouble inverse_m[][3]) {
	GLdouble determinant;
	determinant = original_m[0][0] * (original_m[1][1] - original_m[1][2]) - original_m[0][1] * (original_m[1][0] - original_m[1][2]) +
		original_m[0][2] * (original_m[1][0] - original_m[1][1]);

	inverse_m[0][0] = (original_m[1][1] - original_m[1][2]) / determinant;
	inverse_m[1][0] = (original_m[1][2] - original_m[1][0]) / determinant;
	inverse_m[2][0] = (original_m[1][0] - original_m[1][1]) / determinant;

	inverse_m[0][1] = (original_m[0][2] - original_m[0][1]) / determinant;
	inverse_m[1][1] = (original_m[0][0] - original_m[0][2]) / determinant;
	inverse_m[2][1] = (original_m[0][1] - original_m[0][0]) / determinant;

	inverse_m[0][2] = (original_m[0][1] * original_m[1][2] - original_m[0][2] * original_m[1][1]) / determinant;
	inverse_m[1][2] = (original_m[0][2] * original_m[1][0] - original_m[0][0] * original_m[1][2]) / determinant;
	inverse_m[2][2] = (original_m[0][0] * original_m[1][1] - original_m[0][1] * original_m[1][0]) / determinant;
}

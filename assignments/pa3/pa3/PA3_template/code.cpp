#include "stdafx.h"
#include "code.h"

int winWidth = 640;		// window width
int winHeight = 480;	// window height

V3 ViewPoint;		// view point
V3 ImageLL;		// coordinates at lower left corner of image
V3 ImageLR;		// coordinates at lower right corner of image
V3 ImageUL;		// coordinates at upper left corner of image
V3 ImageUR;		// coordinates at upper right corner of image

int MaxTraceDepth = 5;			// depth of recursive ray-tracing

// scene objects
vector<CLightSource *> vLightSource;		// array of light sources
vector<CPrimitive *> vObjects;				// array of objects


// Calculate the color of each pixel and save it in colorMap
void RayTracing(V3 * colorMap)
{	
  //add your code here
  // visit each pixel
  for (int i = 0; i < winWidth - 1; i++) {
    for (int j = 0; j < winHeight - 1; j++) {
      // Get the world coordinates of p.
      V3 p(ImageLL + i * (ImageLR - ImageLL) / (winWidth - 1) + j * (ImageUL - ImageLL)
		  /(winHeight - 1));
      V3 rayStart(ViewPoint);
      V3 rayDir(p - rayStart);  // fire a ray from the viewpoint through p
      V3 color;
      Trace(rayStart, rayDir, 0, color);  // Call Trace() function to trace the ray and get the color of p.
      colorMap[j * winWidth + i] = color;  // save color in colorMap
    }
  }
}

// Given a ray, calculate the color at the nearest intersection point.
void Trace(V3& rayStart, V3& rayDir, int depth, V3& color)
{
  //add your code here
  rayDir.normalize();
  CPrimitive* objHit;
  V3 intersection, normal;

  if (Intersect(rayStart, rayDir, objHit, intersection, normal)) {
    Shade(objHit, rayStart, rayDir, intersection, normal, depth, color);
  } else {
	// Return the background color(0,0,0).
    color.x = color.y = color.z = 0;
  }
}

// Given a point, calculate the color of this point.
void Shade(CPrimitive *obj,V3& rayStart, V3& rayDir, V3& intersection, V3& normal, int depth, V3& color)
{
  // add your code here
  V3 color_ambient, color_diffuse, color_specular;
  obj->GetAmbient(intersection, color_ambient);
  color = color_ambient;  // ambient term

  // for each light source
  for (vector<CLightSource*>::iterator itr = vLightSource.begin();
       itr != vLightSource.end(); itr++) {
    // ray to light from intersection point
    V3 sRay((*itr)->position - intersection);
    sRay.normalize();  // normalize
    float cosTheta = normal.dot(sRay);
    if (cosTheta > 0) {
      // temp intersection point and normal vector
      V3 tempInter, tempNvec;
      if (!Intersect(intersection, sRay, obj, tempInter, tempNvec)) {
		// see whether sRay is blocked
        obj->GetDiffuse(intersection, color_diffuse);
        obj->GetSpecular(intersection, color_specular);
        V3 R(2 * cosTheta * normal - sRay);  // direction of reflection
        V3 V(rayStart - intersection);       // direction to viewpoint
        V.normalize();
        float cosPhi = R.dot(V);

        // color + diffuse
        color.x += (*itr)->color.x * (obj->m_Opacity * color_diffuse.x * cosTheta +
                                      obj->m_Reflectance * color_specular.x *
                                          pow(cosPhi, obj->m_Shininess));

        color.y += (*itr)->color.y * (obj->m_Opacity * color_diffuse.y * cosTheta +
                                      obj->m_Reflectance * color_specular.y *
                                          pow(cosPhi, obj->m_Shininess));

        color.z += (*itr)->color.z * (obj->m_Opacity * color_diffuse.z * cosTheta +
                                      obj->m_Reflectance * color_specular.z *
                                          pow(cosPhi, obj->m_Shininess));
      }
    }
  }

  // reflected ray contribution
  if (depth < MaxTraceDepth) {
    if (obj->m_Reflectance > 0) {  // if object is reflective
      V3 rRay(rayDir +
              2 * (-rayDir).dot(normal) *
                  normal);  // ray in reflection direction from intersection
      V3 rColor;
      Trace(intersection, rRay, depth + 1, rColor);
      color += obj->m_Reflectance *
               rColor;  // scale rColor by reflectance and add to color
    }
  }

  // clamp color to 1.0
  if (color.x > 1.0) color.x = 1.0;
  if (color.y > 1.0) color.y = 1.0;
  if (color.z > 1.0) color.z = 1.0;
}

// Perform intersection between a ray and a quadratic surface.
bool IntersectQuadratic(V3 rayStart,V3 rayDir, float * coeffMatrix,float& t, V3& intersection)
{		
  // add your code here
  // R(t) = S + Dt;
  float S[] = {rayStart[0], rayStart[1], rayStart[2], 1};
  float D[] = {rayDir[0], rayDir[1], rayDir[2], 0};

  float a, b, c;  // at^2 + bt + c = 0
  float temp[4];
  VectorMultMatrix(D, coeffMatrix, temp);
  a = VectorMultVector(temp, D);  // a = D^T A D
  VectorMultMatrix(S, coeffMatrix, temp);
  b = 2 * VectorMultVector(temp, D);  // b = S^T A D
  c = VectorMultVector(temp, S);      // c = S^T A S
  float delta = b * b - 4 * a * c;    // determine closest t

  if (delta > 0) {
    float t0 = (-b + sqrt(delta)) / (2 * a);
	float t1 = (-b - sqrt(delta)) / (2 * a);
    t = t0 > t1 ? t1 : t0;
  } else if (delta == 0) {
    t = -b / (2 * a);
  } else {
    return false;
  }

  // set intersection
  intersection = rayStart + t * rayDir;
  return true;
}

// Perform intersection between a ray and a triangle.
bool  IntersectTriangle(V3 rayStart,V3 rayDir, V3 v0, V3 v1,V3 v2, float& t,V3& intersection)
{	
  // add your code here
  // plane is NX = d;
  V3 N((v1 - v0).cross(v2 - v0));
  float d = N.dot(v0); 
  if (N.dot(rayDir) != 0) {
    // N(rayStart + t * rayDir) = d
    float val_t = (d - N.dot(rayStart)) / (N.dot(rayDir));
    V3 C(rayStart + val_t * rayDir);
    V3 cp_0(v0 - C), cp_1(v1 - C), cp_2(v2 - C);
	// Compute cross product in the following sequence.
    // cross(CP0, CP1), cross(CP1, CP2), cross(CP2, CP0)
    V3 cross_0(cp_0.cross(cp_1)), cross_1(cp_1.cross(cp_2)), cross_2(cp_2.cross(cp_0));
    // Check if the intersection point is inside the triangle
    if (cross_0.dot(cross_1) > 0 && cross_1.dot(cross_2) > 0 &&
        cross_2.dot(cross_0) > 0) {
      intersection = C;
      return true;
    }
  }
  return false;
}

void MatrixMultVector(float *m,float *v,float *rv)//rv=m*v
{
	rv[0]=m[0]*v[0]+m[4]*v[1]+m[8]*v[2]+m[12]*v[3];
	rv[1]=m[1]*v[0]+m[5]*v[1]+m[9]*v[2]+m[13]*v[3];
	rv[2]=m[2]*v[0]+m[6]*v[1]+m[10]*v[2]+m[14]*v[3];
	rv[3]=m[3]*v[0]+m[7]*v[1]+m[11]*v[2]+m[15]*v[3];
}
void VectorMultMatrix(float *v,float *m,float *lv)//lv=v^Tm
{
	lv[0]=m[0]*v[0]+m[1]*v[1]+m[2]*v[2]+m[3]*v[3];
	lv[1]=m[4]*v[0]+m[5]*v[1]+m[6]*v[2]+m[7]*v[3];
	lv[2]=m[8]*v[0]+m[9]*v[1]+m[10]*v[2]+m[11]*v[3];
	lv[3]=m[12]*v[0]+m[13]*v[1]+m[14]*v[2]+m[15]*v[3];
}
float VectorMultVector(float *v1,float *v2)//v3=v1^Tv2
{
	return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2]+v1[3]*v2[3];
}




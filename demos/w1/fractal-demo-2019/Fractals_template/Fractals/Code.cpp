//////////////////////////////////////////////////////////////////////
// opengl.cpp
#include <afx.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include "Code.h"

#ifdef _DEBUG //for debug builds only
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// global variables used in render the fractals
#define K 2048		// max number of steps
#define R 1000		// square of radius of divergence,sequence jumping outside
					// the circle are considered as divergent

//////////////////////////////////////////////////////////////////
// compute your Mandelbrot set here, store color indices 0-255 in array 'map'.
//
// parameters:
//		left, right, bottom and top - defines the range of the complex number plane
//		winwidth and winheight - defines the viewport dimension
//		map - is the pointer to the unsigned character array to store the resulting
//				color indices
//
// the space needed by the map is already allocated. the number of bytes
// of map array is (winwidth * winheight * sizeof(unsigned char))
// do not access memory beyond the allocated space
//
void Mandelbrot(double left, double right, double bottom, double top, int winwidth,int winheight,unsigned char *map)
{
	int i,j,n,co;
	double da,ia,a,db,ib,b,x,y,temp_x;
	da = (right - left)/winwidth;
  db = (top - bottom)/winheight;
	x = 0;
	y = 0;
	n = 1;
	for(j=0; j<winheight; j++){
		for (i=0; i<winwidth; i++){
	    /* TODO : Add your codes here to replace it */	    		
	    
			map[j*winwidth+i]= (i/30)%2>0?((j/30)%2>0?31:70):((j/30)%2>0?70:31);
							
	    /* TODO : END */
		}
	}
}

//////////////////////////////////////////////////////////////////
// compute your Julia set here, store color indices 0-255 in array 'map'.
//
// parameters:
//		left, right, bottom and top - defines the range of the complex number plane
//		winwidth and winheight - defines the viewport dimension
//		map - is the pointer to the unsigned character array to store the resulting
//				color indices
//		ca and cb - the complex number c=ca+cbi needed in Julia set
//
// the space needed by the map is already allocated. the number of bytes
// of map array is (winwidth * winheight * sizeof(unsigned char))
// do not access memory beyond the allocated space
//
void Julia(double left, double right, double bottom, double top, double a,double b, int winwidth,int winheight,unsigned char *map)
{
	int i,j,n,co;
  	double df,f,dg,g,x,y,temp_x;
	df = (right - left)/winwidth;
  dg = (top - bottom)/winheight;
	n = 1;
	for(j=0; j<winheight; j++){
		for (i=0; i<winwidth; i++){
        /* TODO : Add your codes here to replace it*/
        
			   map[j*winwidth+i]= (i/30)%2>0?((j/30)%2>0?31:70):((j/30)%2>0?70:31);
			   
				/* TODO : END */
		}
	}
}

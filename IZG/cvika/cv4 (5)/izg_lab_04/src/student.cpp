///////////////////////////////////////////////////////////////////////////////
// File containing student functions
///////////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "base.h"
#include "transform.h"
#include "model.h"

#include <cstdio>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
// name spaces

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// function for drawing projected line, etc.
///////////////////////////////////////////////////////////////////////////////

// draw line defined in 3D
// applies transform matrix "model" and consequently "projection"
// a - first vertex in 3D
// b - second vertex
// color - drawing color
void ProjectLine(const S_Coords& a,
                 const S_Coords& b,
                 const S_RGBA &color)
{
  // line is projected by projection of its end points
  // first pally trsnforms of object and scene
  S_Coords aa, bb;
  trTransformVertex(aa, a);
  trTransformVertex(bb, b);

  // subsequently project to screen
  int u1,v1,u2,v2;
  trProjectVertex(u1, v1, aa);
  trProjectVertex(u2, v2, bb);

  DrawLine(u1, v1, u2, v2, aa.z, bb.z, color);
}

// draw triangle defined in 3D
// a,b,c - vertices
// n - normal
// material - material diffuse parameters of lightning model
void ProjectTriangle(const S_Coords& a,
                     const S_Coords& b,
                     const S_Coords& c,
                     const S_Coords& n,
                     const S_Material &material)
{
  //a test to catch cheaters
  S_Coords nn;
  S_Coords aa, bb, cc;

  // transform vertices with model matrix (object/scene manipulation)
  S_Coords ta, tb, tc;
  trTransformVertex(ta, a);
  trTransformVertex(tb, b);
  trTransformVertex(tc, c);

  // project triangle vertices to display
  int u1, v1, u2, v2, u3, v3;
  trProjectVertex(u1, v1, ta);
  trProjectVertex(u2, v2, tb);
  trProjectVertex(u3, v3, tc);

  // transform normal
  S_Coords no;
  trTransformVector(no, n);
  no.normalize();

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // PLACE FOR STUDENTS WORK - LABORATORIES
  // - complete back-faced triangle test
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  S_Coords kamera(0 - ta.x, 0 - ta.y, -CAMERA_DIST - ta.z);

  
  if (((kamera.x * no.x) + (kamera.y * no.y) + (kamera.z * no.z)) < 0)
  {
	  return;
  }
		  
  // recompute color
  S_RGBA color;
  color.red = ROUND2BYTE(255 * material.red);
  color.green = ROUND2BYTE(255 * material.green);
  color.blue = ROUND2BYTE(255 * material.blue);

  // draw as wire model
  DrawLine(u1, v1, u2, v2, ta.z, tb.z, color);
  DrawLine(u2, v2, u3, v3, tb.z, tc.z, color);
  DrawLine(u3, v3, u1, v1, tc.z, ta.z, color);
  
}

///////////////////////////////////////////////////////////////////////////////
// ProjectObject function
// - draw object transformed with active transform matrix

void ProjectObject(const S_Material &material)
{
  T_Triangles::iterator end = triangles.end();
  for( T_Triangles::iterator it = triangles.begin(); it != end; ++it )
  {
    ProjectTriangle(vertices[it->v[0]],
                    vertices[it->v[1]],
                    vertices[it->v[2]],
                    normals[it->n],
                    material);
  }
}

///////////////////////////////////////////////////////////////////////////////
// function DrawScene() invoked from main.cpp
// - draw whole scene including placed objects

void DrawScene()
{
  // clear frame buffer
  ClearBuffers();

  // set projection matrix
  trProjectionPerspective(CAMERA_DIST, frame_w, frame_h);

  // initialize model matrix
  trLoadIdentity();

  // translate whole scene
  trTranslate(0.0, 0.0, scene_move_z);

  // rotate whole scene - in two axis only - mouse is 2D device only... :(
  trRotateX(scene_rot_x * 0.01);
  trRotateY(scene_rot_y * 0.01);
  Matrix matica = trGetMatrix();
  // draw "mirror" with blue color
  ProjectLine(S_Coords(0,-2,-2), S_Coords(0, 2,-2), COLOR_BLUE);
  ProjectLine(S_Coords(0, 2,-2), S_Coords(0, 2, 2), COLOR_BLUE);
  ProjectLine(S_Coords(0, 2, 2), S_Coords(0,-2, 2), COLOR_BLUE);
  ProjectLine(S_Coords(0,-2, 2), S_Coords(0,-2,-2), COLOR_BLUE);

  // draw object with red color
  // add translation and rotation of object
  trTranslate(obj_move_x * 0.015, obj_move_y * 0.015, 0.0);
  trRotateX(obj_rot_x * 0.01);
  trRotateY(obj_rot_y * 0.01);

  // now project and draw the object
  ProjectObject(MAT_RED);

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // PLACE FOR STUDENTS WORK - LABORATORIES
  // - complete drawing of second object symmetrically by the blue "mirror"
  // - use material MAT_RED2
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  trLoadIdentity();
  trSetMatrix(matica);
  trScale(-1, 1, 1);

  trTranslate(obj_move_x * 0.015, obj_move_y * 0.015, 0.0);
  trRotateX(obj_rot_x * 0.01);
  trRotateY(obj_rot_y * 0.01);
  ProjectObject(MAT_RED2);

}

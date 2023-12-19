///////////////////////////////////////////////////////////////////////
// $Id$
//
// Geometric objects (Points, Vectors, Planes, ...) and operations.
//
// Gary Herron
//
// Copyright © 2007 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////

#if !defined(_GEOMLIB_H)
#define _GEOMLIB_H

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include <algorithm> // transform
#include <cmath> // sqrt
#include <functional> // bind2nd, plus, minus, etc.
#include <iostream>
#include <numeric> // inner_product
#include <sstream> // stringstream
#include <utility> // pair

const float PI = 3.14159f;

struct Unimplemented {};       // Marks code to be implemented by students.

// This may be a bad idea.  Previously, Point?D and Vector?D were
// separate types, each with their own methods.  Due to student
// requests, the distinction has been done away with.  Point?D is now
// a synonym for Vector?D, and the methods of the Point?D classes have
// been moved to the corresponding Vector?D class.

// Forward declarations:
using Color = glm::vec3;
using Point2D = glm::vec2;
using Point3D = glm::vec3;
using Vector2D = glm::vec2;
using Vector3D = glm::vec3;
using Vector4D = glm::vec4;
using Matrix4x4 = glm::mat4;

using glm::normalize;
using glm::length;
using glm::cross;
using glm::dot;

class Line3D;
class Box3D;
class Sphere3D;
class Plane3D;
class Triangle3D;

Color HSVColor(float h=0.0, float s=0.0, float v=0.0, float a=1.0);


////////////////////////////////////////////////////////////////////////
// Line2D
////////////////////////////////////////////////////////////////////////
class Line2D
{
public:
    // Constructors
    Line2D() : point(), vector() {return;}
    Line2D(const Point2D& p, const Vector2D& v): point(p), vector(v) {return;}
    Point2D lerp(const float t) const { return(point+t*vector); }

    Point2D point;
    Vector2D vector;
};

// Functions
bool Intersects(const Line2D& line1, const Line2D& line2, float *rt=NULL);

////////////////////////////////////////////////////////////////////////
// Segment2D
////////////////////////////////////////////////////////////////////////
class Segment2D
{
public:
    // Constructor
    Segment2D() : point1(), point2() {return;}

    Segment2D(const Point2D& p1, const Point2D& p2)
        : point1(p1), point2(p2) {return;}
    Point2D lerp(const float t) const { return((1.0f-t)*point1+t*point2); }


    Point2D point1;
    Point2D point2;
};

// Functions
bool Intersects(const Segment2D& seg1, const Segment2D& seg2, float *rt=NULL);

// Utility functions:
float Distance(const Point3D& point, const Line3D& line); 
float Distance(const Point3D& point, const Plane3D& plane);
bool Coplanar(const Point3D& A,const Point3D& B,
              const Point3D& C, const Point3D& D); 

////////////////////////////////////////////////////////////////////////
// Line3D
////////////////////////////////////////////////////////////////////////
class Line3D
{
public:
    // Constructors
    Line3D() : point(), vector() {return;}
    Line3D(const Point3D& p, const Vector3D& v) : point(p),vector(v) {return;}
    Point3D lerp(const float t) const { return(point+t*vector); }

public:
    Point3D point;
    Vector3D vector;
};

// Functions
float AngleBetween(const Line3D& line1, const Line3D& line2); 
bool Coplanar(const Line3D& line1, const Line3D& line2); 
bool Parallel(const Line3D& line1, const Line3D& line2); 
bool Perpendicular(const Line3D& line1, const Line3D& line2);
 
float AngleBetween(const Line3D& line, const Plane3D& plane); 
bool Parallel(const Line3D& line, const Plane3D& plane); 
bool Perpendicular(const Line3D& line, const Plane3D& plane); 
bool Intersects(const Line3D& line, const Plane3D& plane, float *rt=NULL); 

////////////////////////////////////////////////////////////////////////
// Segment3D
////////////////////////////////////////////////////////////////////////
class Segment3D
{
public:
    // Constructors
    Segment3D() : point1(), point2() {return;}
    Segment3D(const Point3D& p1, const Point3D& p2)
        : point1(p1), point2(p2) {return;}
    Point3D lerp(const float t) const { return((1.0f-t)*point1+t*point2); }

    // Utility methods
    bool contains(const Point3D& point) const;

    Point3D point1;
    Point3D point2;
};

// Functions
bool Intersects(const Segment3D& seg, const Triangle3D& tri, float *rt=NULL);


////////////////////////////////////////////////////////////////////////
// Ray3D
////////////////////////////////////////////////////////////////////////
class Ray3D
{
public:
    // Constructor
    Ray3D() : origin(), direction() {return;}
    Ray3D(const Point3D& o, const Vector3D& d)
        : origin(o), direction(d) {return;} 
    Point3D lerp(const float t) const { return(origin+t*direction); }


    // Containment method
    bool contains(const Point3D& point, float *t=NULL) const;
    // Returns paramter of intersection if containment is true and t != NULL

    Point3D origin;
    Vector3D direction;
};

// Utility functions:
bool Intersects(const Ray3D& ray, const Sphere3D& sphere,   float *rt=NULL); 
bool Intersects(const Ray3D& ray, const Triangle3D& tri,    float *rt=NULL);
bool Intersects(const Ray3D& ray, const Box3D& box,         float *rt=NULL);


////////////////////////////////////////////////////////////////////////
// Box3D
////////////////////////////////////////////////////////////////////////
class Box3D
{
public:
    // Constructor
    Box3D() 
        : owner(nullptr)
    {
        return;
    }
    Box3D(const Point3D& c, const Vector3D& e) : center(c), extents(e), owner(nullptr)
    {
        return;
    }

    Box3D(const Point3D& c, const Vector3D& e, void* _owner) : center(c), extents(e), owner(_owner)
    {
        return;
    }

    // Utility method
    bool contains(const Point3D& point) const;

    Point3D  center;    // Center point
    Vector3D extents;   // Center to corner half extents.

    // if used, points to the object which this AABB belongs to
    void* owner;
};


////////////////////////////////////////////////////////////////////////
// Sphere3D
////////////////////////////////////////////////////////////////////////
class Sphere3D
{
public:
    // Constructors
    Sphere3D() : center(), radius(0) {return;}
    Sphere3D(const Point3D& c, const float r) : center(c), radius(r) {return;}

    Point3D center;
    float radius;
};


////////////////////////////////////////////////////////////////////////
// Plane3D
////////////////////////////////////////////////////////////////////////
class Plane3D
{
public:
    // Constructor
    Plane3D(const float A=0, const float B=0, const float C=0, const float D=0)
        { crds[0]=A; crds[1]=B; crds[2]=C; crds[3]=D; }

    // Indexing operators.
    float& operator[](const unsigned int i) { return crds[i]; }
    const float& operator[](const unsigned int i) const { return crds[i]; } 

    // Utility methods.
    Vector3D normal() const { return Vector3D(crds[0], crds[1], crds[2]); }
    float Evaluate(Point3D p) const { return crds[0]*p[0] + crds[1]*p[1] + crds[2]*p[2] + crds[3]; }

private:
    enum {DIM=4} ;
    float crds[DIM];

};

// Utility functions:
float AngleBetween(const Plane3D& plane1, const Plane3D& plane2);
bool Parallel(const Plane3D& plane1, const Plane3D& plane2);
bool Perpendicular(const Plane3D& plane1, const Plane3D& plane2);
bool Intersects(const Segment3D& seg, const Plane3D& plane, float *rt=NULL);


////////////////////////////////////////////////////////////////////////
// Triangle3D
////////////////////////////////////////////////////////////////////////
class Triangle3D
{
public:
    // Constructor
    Triangle3D() {return;}
    Triangle3D(const Point3D& p1, const Point3D& p2, const Point3D& p3)
        { points[0]=p1; points[1]=p2; points[2]=p3; }

        Point3D& operator[](unsigned int i) { return points[i]; }
        const Point3D& operator[](unsigned int i) const { return points[i]; } 


    bool contains(const Point3D& point) const;

private:
    enum{ DIM=3 };
    Point3D points[DIM];
};

// Utility function:
int Intersects(const Triangle3D& tri1, const Triangle3D& tri2,
           std::pair<Point3D, Point3D> *rpoints=0);


#endif // _GEOMLIB_H

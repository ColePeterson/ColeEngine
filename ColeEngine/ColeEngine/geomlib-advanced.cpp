


#include <vector>
#include <cassert>
#include <float.h> // FLT_EPSILON
#include <iostream>

#include "geomlib.h"

////////////////////////////////////////////////////////////////////////
// Distance methods
////////////////////////////////////////////////////////////////////////

// Return the distance from a point to a line.
float Distance(const Point3D& point, const Line3D& line)
{
    float ll = glm::length(line.vector);
    // Not sure if necessary
    if (ll != 0.0f)
        return glm::length(glm::cross(line.vector, point - line.point)) / ll;
    else
        return 0.0f;
}


// Return the distance from a point to a plane.
float Distance(const Point3D& point, const Plane3D& plane)
{
    float pn = glm::length(plane.normal());
    // Not sure if necessary
    if (pn != 0.0f)
        return glm::abs((glm::dot(point, plane.normal()) + plane[3]) / pn);
    else
        return 0.0f;
}

////////////////////////////////////////////////////////////////////////
// Containment methods
////////////////////////////////////////////////////////////////////////

// Determines if point (known to be on a line) is contained within a segment.
bool Segment3D::contains(const Point3D& point) const
{
    if (((this->point2.x - this->point1.x > 0) && (this->point1.x <= point.x && point.x <= this->point2.x)) ||
        (this->point2.x - this->point1.x < 0) && (this->point1.x >= point.x && point.x >= this->point2.x))
    {
        return true;
    }
    else if (((this->point2.y - this->point1.y > 0) && (this->point1.y <= point.y && point.y <= this->point2.y)) ||
        (this->point2.y - this->point1.y < 0) && (this->point1.y >= point.y && point.y >= this->point2.y))
    {
        return true;
    }
    else if (((this->point2.z - this->point1.z > 0) && (this->point1.z <= point.z && point.z <= this->point2.z)) ||
        (this->point2.z - this->point1.z < 0) && (this->point1.z >= point.z && point.z >= this->point2.z))
    {
        return true;
    }
    else
    {
        return false;
    }


}

// Determines if point (known to be on a line) is contained within a ray.
bool Ray3D::contains(const Point3D& point, float* t) const
{
    if (glm::dot(point - this->origin, this->direction) >= 0.0f)
    {
        if (t != nullptr)
            *t = glm::length(point - this->origin) / glm::length(this->direction);

        return true;
    }
    else
        return false;

}

// Determines if point is contained within a box.
bool Box3D::contains(const Point3D& point) const
{
    bool xd = (point.x >= this->center.x - this->extents.x) && (point.x <= this->center.x + this->extents.x);
    bool yd = (point.y >= this->center.y - this->extents.y) && (point.y <= this->center.y + this->extents.y);
    bool zd = (point.z >= this->center.z - this->extents.z) && (point.z <= this->center.z + this->extents.z);

    if (xd && yd && zd)
        return true;
    else
        return false;
}

// Determines if point (known to be on a plane) is contained within a triangle.
bool Triangle3D::contains(const Point3D& point) const
{
    Vector3D N = glm::cross(this->operator[](1) - this->operator[](0), this->operator[](2) - this->operator[](0));

    float e1 = glm::dot(point - this->operator[](0), glm::cross(N, this->operator[](1) - this->operator[](0)));
    float e2 = glm::dot(point - this->operator[](1), glm::cross(N, this->operator[](2) - this->operator[](1)));
    float e3 = glm::dot(point - this->operator[](2), glm::cross(N, this->operator[](0) - this->operator[](2)));

    if (e1 >= 0.0f && e2 >= 0.0f && e3 >= 0.0f)
        return true;
    else
        return false;
}

////////////////////////////////////////////////////////////////////////
// Intersects functions
// In the following Intersects function these rules apply:
//
// * Most test are to determine if a *unique* solution exists. (Or in
//   some cases up to two intersection points exist.)  Parallel
//   objects have either zero or infinitely many solutions and so
//   return false.
//
// * If a unique solution exists, a function value of true is
//   returned.  (Or in the cases where several solutions can exist,
//   the number of intersection parameters are returned.)
//
// * If a unique solution does exist, the calling program may provide
//   a memory location into which the intersection parameter can be
//   returned.  Such pointer may be NULL to indicate that the
//   intersection parameter is not to be returned.
//
////////////////////////////////////////////////////////////////////////

// Determines if 2D segments have a unique intersection.
// If true and rt is not NULL, returns intersection parameter.
bool Intersects(const Segment2D& seg1, const Segment2D& seg2, float* rt)
{
    Line2D line1;
    line1.point = seg1.point1;
    line1.vector = seg1.point2 - seg1.point1;

    Line2D line2;
    line2.point = seg2.point1;
    line2.vector = seg2.point2 - seg2.point1;

    float dp = glm::dot(glm::normalize(line1.vector), glm::normalize(line2.vector));

    if (glm::abs(dp) == 1.0f)
        return false;
    else
    {
        if (rt != nullptr)
        {
            Vector2D u = line2.point - line1.point;
            Vector2D p = { line2.vector.y, line2.vector.x };

            float t = glm::dot(p, u) / glm::dot(p, line1.vector);
            *rt = t;
        }
        return true;
    }
}

// Determines if 2D lines have a unique intersection.
// If true and rt is not NULL, returns intersection parameter.
// May not have been discussed in class.
bool Intersects(const Line2D& line1, const Line2D& line2, float* rt)
{
    float dp = glm::dot(glm::normalize(line1.vector), glm::normalize(line2.vector));

    if (glm::abs(dp) == 1.0f)
        return false;
    else
    {
        if (rt != nullptr)
        {
            Vector2D u = line2.point - line1.point;
            Vector2D p = { line2.vector.y, line2.vector.x };

            float t = glm::dot(p, u) / glm::dot(p, line1.vector);

            *rt = t;
        }
        return true;
    }

}


// Determines if 3D line and plane have a unique intersection.  
// If true and t is not NULL, returns intersection parameter.
bool Intersects(const Line3D& line, const Plane3D& plane, float* rt)
{
    float d = glm::dot(glm::normalize(line.vector), plane.normal());

    if (d == 0.0f)
        return false;
    else
    {
        if (rt != nullptr)
        {
            *rt = (glm::dot(plane.normal(), -line.point) - plane[3]) / glm::dot(plane.normal(), line.vector);
        }
        return true;
    }
}

// Determines if 3D segment and plane have a unique intersection.  
// If true and rt is not NULL, returns intersection parameter.
bool Intersects(const Segment3D& seg, const Plane3D& plane, float* rt)
{
    float t;
    Intersects(Line3D(seg.point1, seg.point2 - seg.point1), plane, &t);

    if (t <= 1.0f && t >= 0.0f)
    {
        if (rt != nullptr)
            *rt = t;

        return true;
    }
    else
        return false;
}

// Determines if 3D segment and triangle have a unique intersection.  
// If true and rt is not NULL, returns intersection parameter.
bool Intersects(const Segment3D& seg, const Triangle3D& tri, float* rt)
{
    float t;

    if (!Intersects(Ray3D(seg.point1, seg.point2 - seg.point1), tri, &t))
        return false;
    else
    {
        if (t >= 0.0f && t <= 1.0f)
        {
            if (rt != nullptr)
            {
                *rt = t;
            }
            return true;
        }
        else
            return false;
    }

}

// Determines if 3D ray and sphere intersect.  
// If so and rt is not NULL, returns intersection parameter.
bool Intersects(const Ray3D& ray, const Sphere3D& sphere, float* rt)
{
    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0f * glm::dot((ray.origin - sphere.center), ray.direction);
    float c = glm::dot(ray.origin - sphere.center, ray.origin - sphere.center) - (sphere.radius * sphere.radius);

    float disc = (b * b) - (4.0f * a * c);

    if (disc <= 0.0f)
        return false;
    else
    {
        float t1 = (-b + glm::sqrt(disc)) / (2.0f * a);
        float t2 = (-b - glm::sqrt(disc)) / (2.0f * a);

        if (t1 < 0.0f && t2 <= 0.0f)
            return false;
        else
        {
            if (rt != nullptr)
                *rt = glm::min(t1, t2);

            return true;
        }
    }
}

// Determines if 3D ray and triangle have a unique intersection.  
// If true and rt is not NULL, returns intersection parameter.
bool Intersects(const Ray3D& ray, const Triangle3D& tri, float* rt)
{
    Vector3D E1 = tri[1] - tri[0];
    Vector3D E2 = tri[2] - tri[0];
    Vector3D p = glm::cross(ray.direction, E2);
    Vector3D S = ray.origin - tri[0];

    float d = glm::dot(p, E1);

    // Ray is parallel to triangle
    if (d == 0.0f)
        return false;

    float u = glm::dot(p, S) / d;

    // Intersects plane, but outside E2 edge
    if (u < 0.0f || u > 1.0f)
        return false;

    Vector3D q = glm::cross(S, E1);

    float v = glm::dot(ray.direction, q) / d;

    // Intersects plane, but outside other edges
    if (v < 0.0f || (u + v) > 1.0f)
    {
        return false;
    }

    float t = glm::dot(E2, q) / d;

    // Negative part of ray intersected triangle
    if (t < 0.0f)
    {
        return false;
    }

    if (rt != nullptr)
        *rt = t;

    return true;
}

// Is value x between a and b in no specific orders
static bool isBetween(float x, float a, float b)
{
    if ((x >= a && x <= b) || (x >= b && x <= a))
        return true;
    else
        return false;
}

// Determines if 3D ray and AABB intersect.  
// If so and rt is not NULL, returns intersection parameter.
bool Intersects(const Ray3D& ray, const Box3D& box, float* rt)
{
    float t0 = -99999.0f, t1 = 99999.0f;

    for (int i = 0; i < 3; i++)
    {
        float p0 = box.center[i] - box.extents[i];
        float p1 = box.center[i] + box.extents[i];

        if (ray.direction[i] != 0.0f)
        {
            float s0 = (p0 - ray.origin[i]) / ray.direction[i];
            float s1 = (p1 - ray.origin[i]) / ray.direction[i];

            t0 = glm::max(t0, glm::min(s0, s1));
            t1 = glm::min(t1, glm::max(s0, s1));
        }
        else
        {
            if (!isBetween(ray.origin[i], p0, p1))
            {
                return false;
            }
        }
    }

    if (t0 > t1)
        return false;

    if (t1 < 0.0f)
        return false;
    else if (t0 < 0.0f)
    {
        if (rt != nullptr)
            *rt = t1;

        return true;
    }
    else
    {
        if (rt != nullptr)
            *rt = t0;

        return true;
    }

}


static std::pair<bool, Point3D> intersectEdgeWithTriangle(const Segment3D& seg, const Triangle3D& tri)
{
    Vector3D n = glm::cross(tri[2] - tri[0], tri[1] - tri[0]);
    float pd = (n.x * tri[0].x) + (n.y * tri[0].y) + (n.z * tri[0].z);

    Plane3D triPlane(n.x, n.y, n.z, -pd); // Plane of triangle
    Line3D edgeLine(seg.point1, seg.point2 - seg.point1); // line of current edge

    float t;

    // Intersect edge line with plane of triangle
    if (!Intersects(edgeLine, triPlane, &t))
        return std::make_pair(false, Point3D());


    Point3D P = edgeLine.point + (t * edgeLine.vector); // Point of intersection 

    // Check if edge contains P
    if (!seg.contains(P))
        return std::make_pair(false, Point3D());


    // Check if triangle contains P
    if (!tri.contains(P))
        return std::make_pair(false, Point3D());

    return std::make_pair(true, P);
}

// Determines if 3D triangles intersect.  
// If parallel, returns false. (This may be considered misleading.)
// If true and rpoint is not NULL, returns two edge/triangle intersections.
int Intersects(const Triangle3D& tri1, const Triangle3D& tri2, std::pair<Point3D, Point3D>* rpoints)
{
    std::vector<Segment3D> edges1;
    edges1.push_back(Segment3D(tri1[0], tri1[2]));
    edges1.push_back(Segment3D(tri1[0], tri1[1]));
    edges1.push_back(Segment3D(tri1[1], tri1[2]));

    std::vector<Segment3D> edges2;
    edges2.push_back(Segment3D(tri2[0], tri2[2]));
    edges2.push_back(Segment3D(tri2[0], tri2[1]));
    edges2.push_back(Segment3D(tri2[1], tri2[2]));

    Point3D p1, p2;
    int n = 0;

    // Check tri1 against tri2
    for (int i = 0; i < 3; i++)
    {
        std::pair<bool, Point3D> P = intersectEdgeWithTriangle(edges1[i], tri2);

        if (P.first)
        {
            if (rpoints != nullptr)
            {
                if (n == 0)
                {
                    rpoints->first = P.second;
                    n++;
                }
                else
                {
                    rpoints->second = P.second;
                    n++;
                    break;
                }

            }

        }
    }

    // Check tri2 against tri1
    for (int i = 0; i < 3; i++)
    {
        std::pair<bool, Point3D> P = intersectEdgeWithTriangle(edges2[i], tri1);

        if (P.first)
        {
            if (rpoints != nullptr)
            {
                if (n == 0)
                {
                    rpoints->first = P.second;
                    n++;
                }
                else
                {
                    rpoints->second = P.second;
                    n++;
                    break;
                }
            }

        }
    }

    return n;
}

////////////////////////////////////////////////////////////////////////
// Geometric relationships
////////////////////////////////////////////////////////////////////////

// Compute angle between two geometric entities (in radians;  use acos)
float AngleBetween(const Line3D& line1, const Line3D& line2)
{
    float ct = glm::dot(line1.vector, line2.vector) / glm::length(line1.vector) * glm::length(line2.vector);
    return glm::acos(ct);
}

// Compute angle between two geometric entities (in radians;  use acos)
float AngleBetween(const Line3D& line, const Plane3D& plane)
{
    float ct = dot(plane.normal(), line.vector) / (length(plane.normal()) * length(line.vector));
    return (PI / 2.0f) - acosf(ct);;
}

// Compute angle between two geometric entities (in radians;  use acos)
float AngleBetween(const Plane3D& plane1, const Plane3D& plane2)
{
    float ct = glm::dot(plane1.normal(), plane2.normal()) / glm::length(plane1.normal()) * glm::length(plane2.normal());
    return glm::acos(ct);
}

// Determine if two vectors are parallel.
bool Parallel(const Vector3D& v1, const Vector3D& v2)
{
    float a = glm::length(glm::cross(v1, v2));
    if (a * a == 0.0f)
        return true;
    else
        return false;
}

bool Perpendicular(const Vector3D& v1, const Vector3D& v2)
{
    if (glm::dot(v1, v2) == 0.0f)
        return true;
    else
        return false;
}

// Determine if two lines are coplanar
bool Coplanar(const Line3D& line1, const Line3D& line2)
{
    Vector3D N = glm::cross(line1.vector, line2.vector);

    if (glm::abs(glm::dot(line2.point - line1.point, N)) < 0.0001f)
        return true;
    else
        return false;
}

// Determine if two lines are parallel.
bool Parallel(const Line3D& line1, const Line3D& line2)
{
    return Parallel(line1.vector, line2.vector);
}

// Determine if lines and plane are parallel.
bool Parallel(const Line3D& line, const Plane3D& plane)
{
    return Perpendicular(line.vector, plane.normal());
}

// Determine if two planes are parallel.
bool Parallel(const Plane3D& plane1, const Plane3D& plane2)
{
    return Parallel(plane1.normal(), plane2.normal());
}

// Determine if two lines are perpendicular.
bool Perpendicular(const Line3D& line1, const Line3D& line2)
{
    return Perpendicular(line1.vector, line2.vector);
}

// Determine if line and plane are perpendicular.
bool Perpendicular(const Line3D& line, const Plane3D& plane)
{
    return Parallel(line.vector, plane.normal());
}

// Determine if two planes are perpendicular.
bool Perpendicular(const Plane3D& plane1, const Plane3D& plane2)
{
    return Perpendicular(plane1.normal(), plane2.normal());
}

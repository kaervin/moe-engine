#include "HandmadeMath.h"
#include <float.h>
#include "typos.h"

#ifndef BAKA_H
#define BAKA_H

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))

#define MAX_CONTACTS 8

typedef struct baka_contact_static{
	v3 point;
	v3 normal;
	float depth;
	int broken;
} baka_contact_static;

typedef struct baka_triangle {
	v3 a, b, c;
} baka_triangle;


typedef struct baka_capsule{
	v3 p, q;
	float radius;
	int num_contacts;
	baka_contact_static static_contacts[MAX_CONTACTS];
	// TODO: temporarily we store dynamic contacts here the same way we do static ones
	// but we could do it in a different way
	int num_dyn_contacts;
	baka_contact_static dyn_contacts[MAX_CONTACTS];
} baka_capsule;

v3 baka_closestpt_point_triangle(
v3 p,
struct baka_triangle tri);

float ClosestPtSegmentSegment(
v3 p1,
v3 q1,
v3 p2,
v3 q2,
float *s_out,
float *t_out,
v3 *c1_out,
v3 *c2_out);

float baka_closestpt_segment_segment(
v3 p1,
v3 q1,
v3 p2,
v3 q2,
float *s_out,
float *t_out,
v3 *c1_out,
v3 *c2_out);

int baka_intersect_segment_triangle(
v3 p,
v3 q,
struct baka_triangle tri,
float *u_out,
float *v_out,
float *w_out,
float *t_out);

int baka_does_contact_capsule_triangle(
baka_capsule *cap,
v3 cap_pos,
Quat cap_rot,
baka_triangle tri);

v3 baka_resolve_capsule_position(
baka_capsule *cap,
v3 cap_pos);

float baka_closestpt_segment_aabb_centered(
v3 p,
v3 q,
v3 ex,
v3 * c1_out_min,
v3 * c2_out_min);

int baka_does_contact_capsule_aabb(
baka_capsule *cap,
v3 cap_pos,
Quat cap_rot,
v3 aabb_mid,
v3 aabb_ex);

#endif /* BAKA_H */

// -------------------------------------------------------------

#ifdef BAKA_IMPLEMENTATION

// page 128 real-time collision detection
// Given segment ab and point c, computes closest point d on ab.
// Also returns t for the position of d, d(t) = a + t*(b - a)
void baka_closestpt_point_segment(v3 c, v3 a, v3 b, float *t, v3 *d)
{
	v3 ab = sub_v3(b, a);
	// Project c onto ab, computing parameterized position d(t) = a + t*(b – a)
	*t = dot_v3(sub_v3(c, a), ab) / dot_v3(ab, ab);
	// If outside segment, clamp t (and therefore d) to the closest endpoint
	if (*t < 0.0f) *t = 0.0f;
	if (*t > 1.0f) *t = 1.0f;
	// Compute projected position from the clamped t
	*d = add_v3(a, mul_v3f(ab, *t));
}

// page 141 real-time collision detection
v3 baka_closestpt_point_triangle(
v3 p,
struct baka_triangle tri)
{
	
	v3 a = tri.a;
	v3 b = tri.b;
	v3 c = tri.c;
	
	// Check if P in vertex region outside A
	v3 ab = sub_v3(b, a);
	v3 ac = sub_v3(c, a);
	v3 ap = sub_v3(p, a);
	float d1 = dot_v3(ab, ap);
	float d2 = dot_v3(ac, ap);
	if (d1 <= 0.0f && d2 <= 0.0f) return a; // barycentric coordinates (1,0,0)
	
	// Check if P in vertex region outside B
	v3 bp = sub_v3(p, b);
	float d3 = dot_v3(ab, bp);
	float d4 = dot_v3(ac, bp);
	if (d3 >= 0.0f && d4 <= d3) return b; // barycentric coordinates (0,1,0)
	
	// Check if P in edge region of AB, if so return projection of P onto AB
	float vc = d1*d4 - d3*d2;
	if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
		float v = d1 / (d1 - d3);
		return add_v3(a, mul_v3f(ab, v)); // barycentric coordinates (1-v,v,0)
	}
	
	// Check if P in vertex region outside C
	v3 cp = sub_v3(p, c);
	float d5 = dot_v3(ab, cp);
	float d6 = dot_v3(ac, cp);
	if (d6 >= 0.0f && d5 <= d6) return c; // barycentric coordinates (0,0,1)
	
	// Check if P in edge region of AC, if so return projection of P onto AC
	float vb = d5*d2 - d1*d6;
	if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
		float w = d2 / (d2 - d6);
		return add_v3(a, mul_v3f(ac, w)); // barycentric coordinates (1-w,0,w)
	}
	
	// Check if P in edge region of BC, if so return projection of P onto BC
	float va = d3*d6 - d5*d4;
	if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
		float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		return add_v3(b, mul_v3f(sub_v3(c, b), w)); // barycentric coordinates (0,1-w,w)
	}
	
	// P inside face region. Compute Q through its barycentric coordinates (u,v,w)
	float denom = 1.0f / (va + vb + vc);
	float v = vb * denom;
	float w = vc * denom;
	return add_v3(add_v3(a, mul_v3f(ab, v)), mul_v3f(ac, w));
	// = u*a + v*b + w*c, u = va * denom = 1.0f-v-w
}


// page 149
// Clamp n to lie within the range [min, max]
float Clamp(float n, float min, float max) {
	if (n < min) return min;
	if (n > max) return max;
	return n;
}
/*
//#define EPSILON = FLT_EPSILON

// Computes closest points C1 and C2 of S1(s)=P1+s*(Q1-P1) and
// S2(t)=P2+t*(Q2-P2), returning s and t. Function result is squared
// distance between between S1(s) and S2(t)
float ClosestPtSegmentSegment(
v3 p1,
v3 q1,
v3 p2,
v3 q2,
float *s_out,
float *t_out,
v3 *c1_out,
v3 *c2_out)
{
 v3 d1 = sub_v3(q1, p1); // Direction vector of segment S1
 v3 d2 = sub_v3(q2, p2); // Direction vector of segment S2
 v3 r = sub_v3(p1, p2);
 float a = dot_v3(d1, d1); // Squared length of segment S1, always nonnegative
 float e = dot_v3(d2, d2); // Squared length of segment S2, always nonnegative
 float f = dot_v3(d2, r);
 
 float s;
 float t;
 
 v3 c1;
 v3 c2;
 
 // Check if either or both segments degenerate into points
 if (a <= FLT_EPSILON && e <= FLT_EPSILON) {
  // Both segments degenerate into points
  *s_out = *t_out = 0.0f;
  *c1_out = p1;
  *c2_out = p2;
  return dot_v3( sub_v3(c1,c2), sub_v3(c1,c2) );
 }
 if (a <= FLT_EPSILON) {
  // First segment degenerates into a point
  s = 0.0f;
  t = f / e; // s = 0 => t = (b*s + f) / e = f / e
  t = Clamp(t, 0.0f, 1.0f);
 } else {
  float c = dot_v3(d1, r);
  if (e <= FLT_EPSILON) {
   // Second segment degenerates into a point
   t = 0.0f;
   s = Clamp(-c / a, 0.0f, 1.0f); // t = 0 => s = (b*t - c) / a = -c / a
  } else {
   // The general nondegenerate case starts here
   float b = dot_v3(d1, d2);
   float denom = a*e-b*b; // Always nonnegative
   // If segments not parallel, compute closest point on L1 to L2 and
   // clamp to segment S1. Else pick arbitrary s (here 0)
   if (denom != 0.0f) {
 s = Clamp((b*f - c*e) / denom, 0.0f, 1.0f);
   } else s = 0.0f;
   
   // Compute point on L2 closest to S1(s) using
   // t = Dot((P1 + D1*s) - P2,D2) / Dot(D2,D2) = (b*s + f) / e
   t = (b*s + f) / e;
   // If t in [0,1] done. Else clamp t, recompute s for the new value
   // of t using s = Dot((P2 + D2*t) - P1,D1) / Dot(D1,D1)= (t*b - c) / a
   // and clamp s to [0, 1]
   if (t < 0.0f) {
 t = 0.0f;
 s = Clamp(-c / a, 0.0f, 1.0f);
   } else if (t > 1.0f) {
 t = 1.0f;
 s = Clamp((b - c) / a, 0.0f, 1.0f);
   }
  }
 }
 *s_out = s;
 *t_out = t; 
 *c1_out = add_v3(p1, mul_v3f(d1, s));
 *c2_out = add_v3(p2, mul_v3f(d2, t));
 return dot_v3( sub_v3(c1, c2), sub_v3(c1, c2));
}
*/

// make sure this isn't used for degenerate segments (length == 0)
float baka_closestpt_segment_segment(
v3 p1,
v3 q1,
v3 p2,
v3 q2,
float *s_out,
float *t_out,
v3 *c1_out,
v3 *c2_out)
{
	v3 d1 = sub_v3(q1, p1); // Direction vector of segment S1
	v3 d2 = sub_v3(q2, p2); // Direction vector of segment S2
	v3 r = sub_v3(p1, p2);
	float a = dot_v3(d1, d1); // Squared length of segment S1, always nonnegative
	float e = dot_v3(d2, d2); // Squared length of segment S2, always nonnegative
	float f = dot_v3(d2, r);
	
	float s;
	float t;
	
	v3 c1;
	v3 c2;
	
	float b = dot_v3(d1, d2);
	float c = dot_v3(d1, r);
	
	float denom = a*e-b*b; // Always nonnegative
	// If segments not parallel, compute closest point on L1 to L2 and
	// clamp to segment S1. Else pick arbitrary s (here 0)
	if (denom != 0.0f) {
		s = Clamp((b*f - c*e) / denom, 0.0f, 1.0f);
	} else s = 0.0f;
	
	// Compute point on L2 closest to S1(s) using
	// t = Dot((P1 + D1*s) - P2,D2) / Dot(D2,D2) = (b*s + f) / e
	t = (b*s + f) / e;
	// If t in [0,1] done. Else clamp t, recompute s for the new value
	// of t using s = Dot((P2 + D2*t) - P1,D1) / Dot(D1,D1)= (t*b - c) / a
	// and clamp s to [0, 1]
	if (t < 0.0f) {
		t = 0.0f;
		s = Clamp(-c / a, 0.0f, 1.0f);
	} else if (t > 1.0f) {
		t = 1.0f;
		s = Clamp((b - c) / a, 0.0f, 1.0f);
	}
	
	c1 = add_v3(p1, mul_v3f(d1, s));
	c2 = add_v3(p2, mul_v3f(d2, t));
	
	*s_out = s;
	*t_out = t; 
	*c1_out = c1;
	*c2_out = c2;
	return dot_v3(sub_v3(c1, c2), sub_v3(c1, c2));
}


// page 191 real-time collision detection
// Given segment pq and triangle abc, returns whether segment intersects
// triangle and if so, also returns the barycentric coordinates (u,v,w)
// of the intersection point
int baka_intersect_segment_triangle(
v3 p,
v3 q,
struct baka_triangle tri,
float *u_out,
float *v_out,
float *w_out,
float *t_out)
{
	
	v3 a = tri.a;
	v3 b = tri.b;
	v3 c = tri.c;
	
	float u = 0.0f;
	float v = 0.0f;
	float w = 0.0f;
	float t = 0.0f;
	
	v3 ab = sub_v3(b, a);
	v3 ac = sub_v3(c, a);
	v3 qp = sub_v3(p, q);
	
	// Compute triangle normal. Can be precalculated or cached if
	// intersecting multiple segments against the same triangle
	v3 n = cross(ab, ac);
	// Compute denominator d. If d <= 0, segment is parallel to or points
	// away from triangle, so exit early
	float d = dot_v3(qp, n);
	if (d <= 0.0f) return 0;
	// Compute intersection t value of pq with plane of triangle. A ray
	// intersects iff 0 <= t. Segment intersects iff 0 <= t <= 1. Delay
	// dividing by d until intersection has been found to pierce triangle
	
	v3 ap = sub_v3(p, a);
	t = dot_v3(ap, n);
	if (t < 0.0f) return 0;
	if (t > d) return 0; // For segment; exclude this code line for a ray test
	// Compute barycentric coordinate components and test if within bounds
	v3 e = cross(qp, ap);
	v = dot_v3(ac, e);
	if (v < 0.0f || v > d) return 0;
	w = -dot_v3(ab, e);
	if (w < 0.0f || v + w > d) return 0;
	// Segment/ray intersects triangle. Perform delayed division and
	// compute the last barycentric coordinate component
	float ood = 1.0f / d;
	t *= ood;
	v *= ood;
	w *= ood;
	u = 1.0f - v - w;
	
	*u_out = u;
	*v_out = v;
	*w_out = w;
	*t_out = t;
	return 1;
}

// returns uvw even when the segment doesn't intersect the triangle
// this way we can find out which edge to check against when
// calculating the distance of the segment to the triangle
int baka_uvw_segment_triangle(
v3 p,
v3 q,
struct baka_triangle tri,
float *u_out,
float *v_out,
float *w_out,
float *t_out)
{
	
	v3 a = tri.a;
	v3 b = tri.b;
	v3 c = tri.c;
	
	float u = 0.0f;
	float v = 0.0f;
	float w = 0.0f;
	float t = 0.0f;
	
	v3 ab = sub_v3(b, a);
	v3 ac = sub_v3(c, a);
	v3 qp = sub_v3(p, q);
	
	// Compute triangle normal. Can be precalculated or cached if
	// intersecting multiple segments against the same triangle
	v3 n = cross(ab, ac);
	// Compute denominator d. If d <= 0, segment is parallel to or points
	// away from triangle, so exit early
	float d = dot_v3(qp, n);
	/*
 if (d <= 0.0f) {
  qp = p;
  p = q;
  q = qp;
  sub_v3(p, q);
  return 2;
 }
 */
	if (d == 0) {
		return 0;
	}
	// Compute intersection t value of pq with plane of triangle. A ray
	// intersects iff 0 <= t. Segment intersects iff 0 <= t <= 1. Delay
	// dividing by d until intersection has been found to pierce triangle
	
	v3 ap = sub_v3(p, a);
	t = dot_v3(ap, n);
	//if (t < 0.0f) return 0;
	//if (t > d) return 0; // For segment; exclude this code line for a ray test
	// Compute barycentric coordinate components and test if within bounds
	v3 e = cross(qp, ap);
	v = dot_v3(ac, e);
	//if (v < 0.0f || v > d) return 0;
	w = -dot_v3(ab, e);
	//if (w < 0.0f || v + w > d) return 0;
	// Segment/ray intersects triangle. Perform delayed division and
	// compute the last barycentric coordinate component
	float ood = 1.0f / d;
	t *= ood;
	v *= ood;
	w *= ood;
	u = 1.0f - v - w;
	
	*u_out = u;
	*v_out = v;
	*w_out = w;
	*t_out = t;
	return 1;
}


enum 
{	
	INSIDE,
	A,
	B,
	C,
	AB,
	BC,
	AC
};


int baka_closestpt_point_triangle_sheet(
v3 p,
struct baka_triangle tri,
v3 *point_out,
float *sqdist)
{
	v3 a = tri.a;
	v3 b = tri.b;
	v3 c = tri.c;
	
	v3 ab = sub_v3(b, a);
	v3 ac = sub_v3(c, a);
	v3 ap = sub_v3(p, a);
	v3 bp = sub_v3(p, b);
	v3 cp = sub_v3(p, c);
	
	float d1 = dot_v3(ab, ap);
	float d2 = dot_v3(ac, ap);
	float d3 = dot_v3(ab, bp);
	float d4 = dot_v3(ac, bp);
	float d5 = dot_v3(ab, cp);
	float d6 = dot_v3(ac, cp);
	
	float va = d3*d6 - d5*d4;
	float vb = d5*d2 - d1*d6;	
	float vc = d1*d4 - d3*d2;
	
	// Check if P in vertex region outside A
	if (d1 < 0.0f && d2 < 0.0f) return A;
	
	// Check if P in vertex region outside B
	if (d3 > 0.0f && d4 < d3) return B;
	
	// Check if P in vertex region outside C
	if (d6 > 0.0f && d5 < d6) return C;
	
	// Check if P in edge region of AB, if so return projection of P onto AB
	if (vc < 0.0f && d1 > 0.0f && d3 < 0.0f) {
		return AB;
	}
	
	// Check if P in edge region of AC, if so return projection of P onto AC
	if (vb < 0.0f && d2 > 0.0f && d6 < 0.0f) {
		return AC;
	}
	
	// Check if P in edge region of BC, if so return projection of P onto BC
	if (va < 0.0f && (d4 - d3) > 0.0f && (d5 - d6) > 0.0f) {
		return BC;
	}
	
	float denom = 1.0f / (va + vb + vc);
	float v = vb * denom;
	float w = vc * denom;
	*point_out = add_v3(add_v3(a, mul_v3f(ab, v)), mul_v3f(ac, w));
	v3 ppoint_out = sub_v3(p, *point_out);
	*sqdist =  dot_v3( ppoint_out, ppoint_out);
	return INSIDE;
	// = u*a + v*b + w*c, u = va * denom = 1.0f-v-w
}


v3 baka_barycentric_to_cartesian(
struct baka_triangle tri,
float u,
float v,
float w)
{
	v3 a = tri.a;
	v3 b = tri.b;
	v3 c = tri.c;
	
	return add_v3( mul_v3f(a,u), add_v3( mul_v3f(b,v), mul_v3f(c,w)));
}


// returns whether it intersects (1) or not (0)
int baka_intersect_or_distance_segment_triangle_dumb(
v3 p,
v3 q,
struct baka_triangle tri,
float *sqdist,
v3 *min_dist_pt_tri,
v3 *min_dist_pt_seg,
int * closest_feature)
{
	v3 a = tri.a;
	v3 b = tri.b;
	v3 c = tri.c;
	
	
	// look if it intersects first (for now)
	
	float u_out;
	float v_out;
	float w_out;
	float t_out;
	
	int does_intersect = baka_intersect_segment_triangle(
		p,
		q,
		tri,
		&u_out,
		&v_out,
		&w_out,
		&t_out);
	
	
	if (does_intersect) {
		*min_dist_pt_tri = baka_barycentric_to_cartesian(tri,u_out,v_out,w_out);
		*min_dist_pt_seg = *min_dist_pt_tri;
		*sqdist = 0.0f;
		return 1;
	}
	
	// then look in which voronoi region the projection of p and q is
	// (and calculate the squared distance)
	
	v3 point_out_p;
	float sqdist_p;
	
	v3 point_out_q;
	float sqdist_q;
	
	int region_p = baka_closestpt_point_triangle_sheet(
		p,
		tri,
		&point_out_p,
		&sqdist_p);
	
	int region_q = baka_closestpt_point_triangle_sheet(
		q,
		tri,
		&point_out_q,
		&sqdist_q);
	
	if ( region_p == INSIDE && region_q == INSIDE ) {
		
		(*closest_feature) = INSIDE;
		
		if ( sqdist_p <= sqdist_q ) {
			
			*min_dist_pt_tri = point_out_p;
			*min_dist_pt_seg = p;
			*sqdist = sqdist_p;
			return 0;
			
		} else {
			
			*min_dist_pt_tri = point_out_q;
			*min_dist_pt_seg = q;
			*sqdist = sqdist_q;
			return 0;
			
		}
	}
	
	(*closest_feature) = A;
	
	float s_ab;
	float t_ab;
	v3 c1_ab;
	v3 c2_ab;
	
	float s_bc;
	float t_bc;
	v3 c1_bc;
	v3 c2_bc;
	
	float s_ca;
	float t_ca;
	v3 c1_ca;
	v3 c2_ca;
	
	float sqdist_ab = baka_closestpt_segment_segment(
		p,
		q,
		a,
		b,
		&s_ab,
		&t_ab,
		&c1_ab,
		&c2_ab);
	
	float sqdist_bc = baka_closestpt_segment_segment(
		p,
		q,
		b,
		c,
		&s_bc,
		&t_bc,
		&c1_bc,
		&c2_bc);
	
	float sqdist_ca = baka_closestpt_segment_segment(
		p,
		q,
		c,
		a,
		&s_ca,
		&t_ca,
		&c1_ca,
		&c2_ca);
	
	
	*sqdist = sqdist_ab;
	*min_dist_pt_tri = c2_ab;
	*min_dist_pt_seg = c1_ab;
	
	if ( sqdist_bc < sqdist_ab && sqdist_bc < sqdist_ca ) {
		*sqdist = sqdist_bc;
		*min_dist_pt_tri = c2_bc;
		*min_dist_pt_seg = c1_bc;
	}
	
	if ( sqdist_ca <= sqdist_ab && sqdist_ca <= sqdist_bc ) {
		*sqdist = sqdist_ca;
		*min_dist_pt_tri = c2_ca;
		*min_dist_pt_seg = c1_ca;
	}
	
	if ( region_p == INSIDE && sqdist_p < *sqdist ) {
		*sqdist = sqdist_p;
		*min_dist_pt_tri = point_out_p;
		*min_dist_pt_seg = p;
	}
	
	if ( region_q == INSIDE && sqdist_q < *sqdist ) {
		*sqdist = sqdist_q;
		*min_dist_pt_tri = point_out_q;
		*min_dist_pt_seg = q;
	}
	
	return 0;
}

int baka_does_contact_capsule_triangle(
baka_capsule *cap,
v3 cap_pos,
Quat cap_rot,
baka_triangle tri)
{
	
	if (cap->num_contacts >= MAX_CONTACTS) {
		return -1;
	}
	
    v3 p = add_v3(cap_pos, rotate_vec3_quat(cap->p, cap_rot));
    v3 q = add_v3(cap_pos, rotate_vec3_quat(cap->q, cap_rot));
	
	v3 contact_point;
	v3 contact_normal;
	float penetration_depth;
	
	float sqradius = cap->radius * cap->radius;
	
	float sqdist;
	v3 min_dist_pt_tri;	
	v3 min_dist_pt_seg;
	int closest_feature;
	
	int does_intersect = baka_intersect_or_distance_segment_triangle_dumb(
		p,
		q,
		tri,
		&sqdist,
		&min_dist_pt_tri,
		&min_dist_pt_seg,
		&closest_feature);
	
	//printf("sqdist:   %f \n", sqdist);
	//printf("sqradius: %f \n", sqradius);
	
	float margin = 0.02;
	if (closest_feature != INSIDE) {
		//margin = -0.1;
	} 
	
	if (sqdist > sqradius + margin) {
		return -1;
	}
	
	contact_point = min_dist_pt_tri;
	
	if (does_intersect) {
		v3 ab = sub_v3(tri.b, tri.a);
		v3 ac = sub_v3(tri.c, tri.a);
		contact_normal = normalize_v3(cross(ab, ac));
	} else {
		contact_normal = normalize_v3(sub_v3(min_dist_pt_seg, min_dist_pt_tri));
	}
	/*
  v3 ab = sub_v3(tri.b, tri.a);
  v3 ac = sub_v3(tri.c, tri.a);
  contact_normal = normalize_v3(cross(ab, ac));
 */
	penetration_depth = HMM_SquareRootF(sqdist) - cap->radius;
	
	cap->static_contacts[cap->num_contacts].point = contact_point;
	cap->static_contacts[cap->num_contacts].normal = contact_normal;
	cap->static_contacts[cap->num_contacts].depth = penetration_depth;
	cap->static_contacts[cap->num_contacts].broken = 0;
	cap->num_contacts++;
	
	return cap->num_contacts-1;
}


v3 baka_resolve_capsule_position(
baka_capsule *cap,
v3 cap_pos)
{
	
	v3 displacement_direction = vec3(0.0f, 0.0f, 0.0f);
	
	for (int times = 0; times < 8; times++) {
		
	    for (int i = 0; i < cap->num_contacts; i++) {
			
	        // TODO: optimize this
	        // there might be 100s of things here we do twice
	        // also all the names are probably wrong, so calculate on a piece of paper again
			
	        baka_contact_static i_contact = cap->static_contacts[i];
			
	        v3 penetration_direction = mul_v3f(i_contact.normal, i_contact.depth);
			
	        v3 contact_point_on_capsule = sub_v3(i_contact.point, penetration_direction);
			
	        v3 current_contact_point = sub_v3(contact_point_on_capsule, displacement_direction);
			
	        float i_contact_plane_distance = dot_v3(i_contact.normal, i_contact.point);
			
	        float new_displacement_distance = DistPointPlane(
	            current_contact_point,
	            i_contact.normal,
	            i_contact_plane_distance) * 0.9f; // - 0.001f;
			
	        // maybe this is unnecessary
	        if (new_displacement_distance > 0.0f) {
	            displacement_direction = add_v3(
	                displacement_direction,
	                mul_v3f(i_contact.normal,new_displacement_distance));
	        } 
	    }
	}
	
    return displacement_direction;
}
// TODO: this is stupid but works for now
// by simply mirroring you can probably skip half the checks already
float baka_closestpt_segment_aabb_centered(
v3 p,
v3 q,
v3 ex,
v3 * c1_out_min,
v3 * c2_out_min)
{
	
	// right side points
	v3 right_top_front    = vec3(ex.X, ex.Y, ex.Z);
	v3 right_top_back     = vec3(ex.X, ex.y, -ex.Z);
	v3 right_bottom_front = vec3(ex.X, -ex.Y, ex.Z);
	v3 right_bottom_back  = vec3(ex.X, -ex.Y, -ex.Z);
	
	// left side points
	v3 left_top_front    = vec3(-ex.X, ex.Y, ex.Z);
	v3 left_top_back     = vec3(-ex.X, ex.y, -ex.Z);
	v3 left_bottom_front = vec3(-ex.X, -ex.Y, ex.Z);
	v3 left_bottom_back  = vec3(-ex.X, -ex.Y, -ex.Z);
	
	v3 p2s[12];
	v3 q2s[12];
	
	float sqlen_min = 1000000.0f;
	
	p2s[0] = right_top_front;
	q2s[0] = left_top_front;
	
	p2s[1] = right_top_back;
	q2s[1] = left_top_back;
	
	p2s[2] = right_bottom_front;
	q2s[2] = left_bottom_front;
	
	p2s[3] = right_bottom_back;
	q2s[3] = left_bottom_back;
	
	
	p2s[4] = right_top_front;
	q2s[4] = right_top_back;
	
	p2s[5] = right_bottom_front;
	q2s[5] = right_bottom_back;
	
	p2s[6] = right_top_front;
	q2s[6] = right_bottom_front;
	
	p2s[7] = right_top_back;
	q2s[7] = right_bottom_back;
	
	
	p2s[8] = left_top_front;
	q2s[8] = left_top_back;
	
	p2s[9] = left_bottom_front;
	q2s[9] = left_bottom_back;
	
	p2s[10] = left_top_front;
	q2s[10] = left_bottom_front;
	
	p2s[11] = left_top_back;
	q2s[11] = left_bottom_back;
	
	for (int i = 0; i < 12; ++i)
	{
		float sqlen;
		float s_out;
		float t_out;
		v3 c1_out;
		v3 c2_out;
		
		sqlen = baka_closestpt_segment_segment(
			p,
			q,
			p2s[i],
			q2s[i],
			&s_out,
			&t_out,
			&c1_out,
			&c2_out);
		
		if (sqlen <= sqlen_min) {
			sqlen_min = sqlen;
			(*c1_out_min) = c1_out;
			(*c2_out_min) = c2_out;	
		}
	}
	
	v3 p_clip = vec3(
		HMM_Clamp(-ex.X, p.X, ex.X),
		HMM_Clamp(-ex.Y, p.Y, ex.Y),
		HMM_Clamp(-ex.Z, p.Z, ex.Z)
		);
	
	v3 q_clip = vec3(
		HMM_Clamp(-ex.X, q.X, ex.X),
		HMM_Clamp(-ex.Y, q.Y, ex.Y),
		HMM_Clamp(-ex.Z, q.Z, ex.Z)
		);
	
	float p_clip_sqdist = dot_v3( sub_v3(p, p_clip), sub_v3(p, p_clip));
	float q_clip_sqdist = dot_v3( sub_v3(q, q_clip), sub_v3(q, q_clip));
	
	if (p_clip_sqdist < sqlen_min) {
		sqlen_min = p_clip_sqdist;
		(*c1_out_min) = p;
		(*c2_out_min) = p_clip;
	}
	
	if (q_clip_sqdist < sqlen_min) {
		sqlen_min = q_clip_sqdist;
		(*c1_out_min) = q;
		(*c2_out_min) = q_clip;
	}
	
	return sqlen_min;
}

int baka_does_contact_capsule_aabb(
baka_capsule *cap,
v3 cap_pos,
Quat cap_rot,
v3 aabb_mid,
v3 aabb_ex)
{
	
	v3 c1_out_min;
	v3 c2_out_min;
	
	cap_pos = sub_v3(cap_pos, aabb_mid);
	
	float sqdist = baka_closestpt_segment_aabb_centered(add_v3(cap_pos, rotate_vec3_quat(cap->p, cap_rot)),add_v3(cap_pos, rotate_vec3_quat(cap->q, cap_rot)),aabb_ex, &c1_out_min, &c2_out_min);
	
	if (sqdist == 0.0f) {
		// here will be a special case later
		return -2;
	}
	
	float sqradius = cap->radius * cap->radius;
	if (sqdist > sqradius) {
		return -1;
	}
	
	c1_out_min = add_v3(c1_out_min, aabb_mid);
	c2_out_min = add_v3(c2_out_min, aabb_mid);
	
	v3 contact_normal = normalize_v3(sub_v3(c1_out_min, c2_out_min));
	float penetration_depth = HMM_SquareRootF(sqdist) - cap->radius;
	
	if (cap->num_contacts >= MAX_CONTACTS) {
		return -1;
	}
	
	cap->static_contacts[cap->num_contacts].point = c2_out_min;
	cap->static_contacts[cap->num_contacts].normal = contact_normal;
	cap->static_contacts[cap->num_contacts].depth = penetration_depth;
	cap->static_contacts[cap->num_contacts].broken = 0;
	cap->num_contacts++;
	
	return cap->num_contacts-1;
}

int baka_does_contact_capsule_obb(
baka_capsule *cap,
v3 cap_pos,
Quat cap_rot,
v3 obb_mid,
v3 obb_ex,
Quat obb_rotation)
{
	
	Quat inv_rotation = inverse_quat(obb_rotation);
	
	v3 c1_out_min;
	v3 c2_out_min;
	
	cap_pos = sub_v3(cap_pos, obb_mid);
	
	v3 p_cap_cubespace = rotate_vec3_quat(cap->p, cap_rot);
	v3 q_cap_cubespace = rotate_vec3_quat(cap->q, cap_rot);
	
	p_cap_cubespace = add_v3(p_cap_cubespace, cap_pos);
	q_cap_cubespace = add_v3(q_cap_cubespace, cap_pos);
	
	p_cap_cubespace = rotate_vec3_quat(p_cap_cubespace, inv_rotation);
	q_cap_cubespace = rotate_vec3_quat(q_cap_cubespace, inv_rotation);
	
	float sqdist = baka_closestpt_segment_aabb_centered( 
        p_cap_cubespace,
        q_cap_cubespace,
		obb_ex,
		&c1_out_min,
		&c2_out_min);
	
	if (sqdist == 0.0f) {
		// here will be a special case later
		return -2;
	}
	
	float sqradius = cap->radius * cap->radius;
	if (sqdist > sqradius) {
		return -1;
	}
	
	c1_out_min = rotate_vec3_quat(c1_out_min, obb_rotation);
	c2_out_min = rotate_vec3_quat(c2_out_min, obb_rotation);
	
	c1_out_min = add_v3(c1_out_min, obb_mid);
	c2_out_min = add_v3(c2_out_min, obb_mid);
	
	v3 contact_normal = normalize_v3(sub_v3(c1_out_min, c2_out_min));
	float penetration_depth = HMM_SquareRootF(sqdist) - cap->radius;
	
	if (cap->num_contacts >= MAX_CONTACTS) {
		return -1;
	}
	
	cap->static_contacts[cap->num_contacts].point = c2_out_min;
	cap->static_contacts[cap->num_contacts].normal = contact_normal;
	cap->static_contacts[cap->num_contacts].depth = penetration_depth;
	cap->static_contacts[cap->num_contacts].broken = 0;
	cap->num_contacts++;	
	
	return cap->num_contacts-1;
}


int baka_does_contact_capsule_sphere(
baka_capsule *cap,
v3 cap_pos,
Quat cap_rot,
v3 sphere_point,
float sphere_radius)
{
	v3 cap_a = add_v3(cap_pos, rotate_vec3_quat(cap->p, cap_rot));v3 cap_b = add_v3(cap_pos, rotate_vec3_quat(cap->q, cap_rot));
	v3 c = sphere_point;
	
	float t;
	v3 d;
	baka_closestpt_point_segment(sphere_point, cap_a, cap_b, &t, &d);
	
	v3 d_to_c = sub_v3(d, c);
	v3 c_to_d = sub_v3(c, d);
	float d_to_c_sqlen = dot_v3(d_to_c, d_to_c);
	float d_to_c_len = HMM_SquareRootF(d_to_c_sqlen);
	float d_to_sphere_len = d_to_c_len - sphere_radius;
	
	if (d_to_sphere_len > cap->radius) {
		return -1;
	}
	
	if (cap->num_contacts >= MAX_CONTACTS) {
		return -1;
	}
	
	float penetration_depth = d_to_sphere_len - cap->radius;
	
	// NOTE: this is the old point calculation, I don't know why it is that way
	//cap->static_contacts[cap->num_contacts].point = add_v3(d_to_c, mul_v3f(normalize_v3(c_to_d), t));
	
	cap->static_contacts[cap->num_contacts].point = add_v3(d, mul_v3f(normalize_v3(c_to_d), cap->radius));
	cap->static_contacts[cap->num_contacts].normal = normalize_v3(d_to_c);
	cap->static_contacts[cap->num_contacts].depth = penetration_depth;
	cap->static_contacts[cap->num_contacts].broken = 0;
	cap->num_contacts++;	
	
	return cap->num_contacts-1;
	
}


// for an explanation: https://tavianator.com/fast-branchless-raybounding-box-intersections/
// we are basically normalizing all the slopes in their direction, and then clipping the ray to them 
bool raycast_aabb(v3 ray_origin, v3 inv_ray_direction, v3 aabb_min, v3 aabb_max, float *tmin_ret) {
	float tmin = -INFINITY;
	float tmax = INFINITY;
	
    if (inv_ray_direction.X != 0.0f) {
		float tx1 = (aabb_min.X - ray_origin.X)*inv_ray_direction.X;
		float tx2 = (aabb_max.X - ray_origin.X)*inv_ray_direction.X;
		
        tmin = max(tmin, min(tx1, tx2));
        tmax = min(tmax, max(tx1, tx2));
		
    } else if (ray_origin.X <= aabb_min.X || ray_origin.X >= aabb_max.X) {
		return false;
	}
	
    if (inv_ray_direction.Y != 0.0f) {
		float ty1 = (aabb_min.Y - ray_origin.Y)*inv_ray_direction.Y;
		float ty2 = (aabb_max.Y - ray_origin.Y)*inv_ray_direction.Y;
		
        tmin = max(tmin, min(ty1, ty2));
        tmax = min(tmax, max(ty1, ty2));
    } else if (ray_origin.Y <= aabb_min.Y || ray_origin.Y >= aabb_max.Y) {
		return false;
	}
	
    if (inv_ray_direction.Z != 0.0f) {
		float tz1 = (aabb_min.Z - ray_origin.Z)*inv_ray_direction.Z;
		float tz2 = (aabb_max.Z - ray_origin.Z)*inv_ray_direction.Z;
		
        tmin = max(tmin, min(tz1, tz2));
        tmax = min(tmax, max(tz1, tz2));
    } else if (ray_origin.Z <= aabb_min.Z || ray_origin.Z >= aabb_max.Z) {
		return false;
	}
	
	*tmin_ret = tmin; 
    return tmax >= tmin;
}

// think about if this should affect normal and float if there is no hit
bool raycast_aabb_with__normal(v3 ray_origin, v3 inv_ray_direction, v3 aabb_min, v3 aabb_max, float *tmin_ret, v3 *normal) {
	float tmin = -INFINITY;
	float tmax = INFINITY;
	
    if (inv_ray_direction.X != 0.0f) {
		float tx1 = (aabb_min.X - ray_origin.X)*inv_ray_direction.X;
		float tx2 = (aabb_max.X - ray_origin.X)*inv_ray_direction.X;
		
		float minx;
		v3 x_normal;
		
		if (tx1 <= tx2) {
			minx = tx1;
			x_normal = vec3(-1.0f, 0.0f, 0.0f);
		} else {
			minx = tx2;
			x_normal = vec3(1.0f, 0.0f, 0.0f);
		}
		
		if (minx >= tmin) {
			tmin = minx;
			*normal = x_normal;
		}
		
        tmax = min(tmax, max(tx1, tx2));
		
    } else if (ray_origin.X <= aabb_min.X || ray_origin.X >= aabb_max.X) {
		return false;
	}
	
    if (inv_ray_direction.Y != 0.0f) {
		float ty1 = (aabb_min.Y - ray_origin.Y)*inv_ray_direction.Y;
		float ty2 = (aabb_max.Y - ray_origin.Y)*inv_ray_direction.Y;
		
		float miny;
		v3 y_normal;
		
		if (ty1 <= ty2) {
			miny = ty1;
			y_normal = vec3(0.0f, -1.0f, 0.0f);
		} else {
			miny = ty2;
			y_normal = vec3(0.0f, 1.0f, 0.0f);
		}
		
		if (miny >= tmin) {
			tmin = miny;
			*normal = y_normal;
		}
		
		tmax = min(tmax, max(ty1, ty2));
    } else if (ray_origin.Y <= aabb_min.Y || ray_origin.Y >= aabb_max.Y) {
		return false;
	}
	
    if (inv_ray_direction.Z != 0.0f) {
		float tz1 = (aabb_min.Z - ray_origin.Z)*inv_ray_direction.Z;
		float tz2 = (aabb_max.Z - ray_origin.Z)*inv_ray_direction.Z;
		
		float minz;
		v3 z_normal;
		
		if (tz1 <= tz2) {
			minz = tz1;
			z_normal = vec3(0.0f, 0.0f, -1.0f);
		} else {
			minz = tz2;
			z_normal = vec3(0.0f, 0.0f, 1.0f);
		}
		
		if (minz >= tmin) {
			tmin = minz;
			*normal = z_normal;
		}
		
		tmax = min(tmax, max(tz1, tz2));
    } else if (ray_origin.Z <= aabb_min.Z || ray_origin.Z >= aabb_max.Z) {
		return false;
	}
	
	*tmin_ret = tmin; 
    return tmax >= tmin;
}


bool raycast_obb(v3 ray_origin, v3 ray_direction, v3 obb_mid,
				 v3 obb_ex, Quat obb_rotation, float *tmin_ret) {
	// rotate the ray with the inverse obb rotation, so that we can just do an aabb check
	Quat inv_rotation = inverse_quat(obb_rotation);
	
	v3 ray_origin_obbspace = sub_v3(ray_origin, obb_mid);
	ray_origin_obbspace = rotate_vec3_quat(ray_origin_obbspace, inv_rotation);
	
	v3 ray_direction_obbspace = rotate_vec3_quat(ray_direction, inv_rotation);
	
	v3 inv_ray_direction_obbspace = vec3(1/ray_direction_obbspace.X, 1/ray_direction_obbspace.Y,1/ray_direction_obbspace.Z);
	
	v3 aabb_min = sub_v3(vec3(0.0f, 0.0f, 0.0f), obb_ex);
	v3 aabb_max = add_v3(vec3(0.0f, 0.0f, 0.0f), obb_ex);
	
    return raycast_aabb(ray_origin_obbspace, inv_ray_direction_obbspace, aabb_min, aabb_max, tmin_ret);
}


bool raycast_obb_with_normal(v3 ray_origin, v3 ray_direction, v3 obb_mid,
							 v3 obb_ex, Quat obb_rotation, float *tmin_ret, v3 *normal) {
	// rotate the ray with the inverse obb rotation, so that we can just do an aabb check
	Quat inv_rotation = inverse_quat(obb_rotation);
	
	v3 ray_origin_obbspace = sub_v3(ray_origin, obb_mid);
	ray_origin_obbspace = rotate_vec3_quat(ray_origin_obbspace, inv_rotation);
	
	v3 ray_direction_obbspace = rotate_vec3_quat(ray_direction, inv_rotation);
	
	v3 inv_ray_direction_obbspace = vec3(1/ray_direction_obbspace.X, 1/ray_direction_obbspace.Y,1/ray_direction_obbspace.Z);
	
	v3 aabb_min = sub_v3(vec3(0.0f, 0.0f, 0.0f), obb_ex);
	v3 aabb_max = add_v3(vec3(0.0f, 0.0f, 0.0f), obb_ex);
	
    if (raycast_aabb_with__normal(ray_origin_obbspace, inv_ray_direction_obbspace, aabb_min, aabb_max, tmin_ret, normal)) {
		*normal = rotate_vec3_quat(*normal, obb_rotation);
		return true;
	}
	return false;
	
}


typedef struct baka_AABB {
	v3 mid;
	v3 ex;
	v3 min;
	v3 max;
} baka_AABB;

baka_AABB baka_make_AABB(v3 mid, v3 ex) {
	baka_AABB aabb;
	aabb.mid = mid;
	aabb.ex = ex;
	aabb.min = sub_v3(mid, ex);
	aabb.max = add_v3(mid, ex);
	return aabb;
};

baka_AABB baka_make_AABB_min_max(v3 min, v3 max) {
	baka_AABB aabb;
	aabb.ex = mul_v3f(sub_v3(max, min), 0.5f);
	aabb.mid = add_v3(min, aabb.ex);
	aabb.min = min;
	aabb.max = max;
	return aabb;
};

bool baka_test_AABB_AABB(baka_AABB a, baka_AABB b)
{
	if (HMM_ABS(a.mid.X - b.mid.X) > (a.ex.X + b.ex.X)) return 0;
	if (HMM_ABS(a.mid.Y - b.mid.Y) > (a.ex.Y + b.ex.Y)) return 0;
	if (HMM_ABS(a.mid.Z - b.mid.Z) > (a.ex.Z + b.ex.Z)) return 0;
	return 1;
}


// from real time collision detection
bool baka_raycast_plane(v3 ray_point, v3 ray_direction, float plane_normal_distance, v3 plane_normal, float *t)
{
	*t = (plane_normal_distance - dot_v3(plane_normal, ray_point)) / dot_v3(plane_normal, ray_direction);
	// If t biger than 0  compute and return intersection point
	if (*t >= 0.0f) {
		return 1;
	}
	// Else no intersection
	return 0;
}

// think about if this should affect normal and float if there is no hit
bool baka_raycast_aabb_get_normal(v3 ray_origin, v3 inv_ray_direction, baka_AABB aabb, float *tmin_ret, v3 *normal) {
	float tmin = -INFINITY;
	float tmax = INFINITY;
	
    if (inv_ray_direction.X != 0.0f) {
		float tx1 = (aabb.min.X - ray_origin.X)*inv_ray_direction.X;
		float tx2 = (aabb.max.X - ray_origin.X)*inv_ray_direction.X;
		
		float minx;
		v3 x_normal;
		
		if (tx1 <= tx2) {
			minx = tx1;
			x_normal = vec3(-1.0f, 0.0f, 0.0f);
		} else {
			minx = tx2;
			x_normal = vec3(1.0f, 0.0f, 0.0f);
		}
		
		if (minx >= tmin) {
			tmin = minx;
			*normal = x_normal;
		}
		
        tmax = min(tmax, max(tx1, tx2));
		
    } else if (ray_origin.X <= aabb.min.X || ray_origin.X >= aabb.max.X) {
		return false;
	}
	
    if (inv_ray_direction.Y != 0.0f) {
		float ty1 = (aabb.min.Y - ray_origin.Y)*inv_ray_direction.Y;
		float ty2 = (aabb.max.Y - ray_origin.Y)*inv_ray_direction.Y;
		
		float miny;
		v3 y_normal;
		
		if (ty1 <= ty2) {
			miny = ty1;
			y_normal = vec3(0.0f, -1.0f, 0.0f);
		} else {
			miny = ty2;
			y_normal = vec3(0.0f, 1.0f, 0.0f);
		}
		
		if (miny >= tmin) {
			tmin = miny;
			*normal = y_normal;
		}
		
		tmax = min(tmax, max(ty1, ty2));
    } else if (ray_origin.Y <= aabb.min.Y || ray_origin.Y >= aabb.max.Y) {
		return false;
	}
	
    if (inv_ray_direction.Z != 0.0f) {
		float tz1 = (aabb.min.Z - ray_origin.Z)*inv_ray_direction.Z;
		float tz2 = (aabb.max.Z - ray_origin.Z)*inv_ray_direction.Z;
		
		float minz;
		v3 z_normal;
		
		if (tz1 <= tz2) {
			minz = tz1;
			z_normal = vec3(0.0f, 0.0f, -1.0f);
		} else {
			minz = tz2;
			z_normal = vec3(0.0f, 0.0f, 1.0f);
		}
		
		if (minz >= tmin) {
			tmin = minz;
			*normal = z_normal;
		}
		
		tmax = min(tmax, max(tz1, tz2));
    } else if (ray_origin.Z <= aabb.min.Z || ray_origin.Z >= aabb.max.Z) {
		return false;
	}
	
	*tmin_ret = tmin; 
    return tmax >= tmin;
}


typedef struct baka_sphere {
	v3 center;
	float radius;
} baka_sphere;

baka_sphere baka_make_sphere(v3 center, float radius) {
	baka_sphere sphere;
	sphere.center = center;
	sphere.radius = radius;
	return sphere;
};


// real time collision detection 179

// Intersects ray r = p + td, with sphere s and, if intersecting,
// returns t value of intersection and intersection point q
int baka_raycast_sphere_get_normal(v3 p, v3 d, baka_sphere s, float *t, v3 *normal)
{
	v3 m = sub_v3(p, s.center);
	float a = dot_v3(d, d);
	float b = dot_v3(m, d);
	float c = dot_v3(m, m) - s.radius * s.radius;
	// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0)
	if (c > 0.0f && b > 0.0f) return 0;
	float discr = b*b - a*c;
	// A negative discriminant corresponds to ray missing sphere
	if (discr < 0.0f) return 0;
	// Ray now found to intersect sphere, compute smallest t value of intersection
	*t = (-b - HMM_SquareRootF(discr)) / a;
	// If t is negative, ray started inside sphere so clamp t to zero
	if (*t < 0.0f) *t = 0.0f;
	
	v3 hitpoint = add_v3(p, mul_v3f(d, *t));
	v3 hitpoint_relative_to_sphere = sub_v3(hitpoint, s.center);
	
	*normal = normalize_v3(hitpoint_relative_to_sphere);
	return 1;
}



// page 191 real-time collision detection
// Given segment pq and triangle abc, returns whether segment intersects
// triangle and if so, also returns the barycentric coordinates (u,v,w)
// of the intersection point
int baka_raycast_triangle_get_normal(
v3 p,
v3 dir,
struct baka_triangle tri,
float *t_out,
v3 *normal)
{
	
	v3 a = tri.a;
	v3 b = tri.b;
	v3 c = tri.c;
	
	float u = 0.0f;
	float v = 0.0f;
	float w = 0.0f;
	float t = 0.0f;
	
	v3 ab = sub_v3(b, a);
	v3 ac = sub_v3(c, a);
	v3 qp = mul_v3f(dir, -1.0f);
	
	// Compute triangle normal. Can be precalculated or cached if
	// intersecting multiple segments against the same triangle
	v3 n = cross(ab, ac);
	// Compute denominator d. If d <= 0, segment is parallel to or points
	// away from triangle, so exit early
	float d = dot_v3(qp, n);
	if (d <= 0.0f) {
		return 0;
	}
	// Compute intersection t value of pq with plane of triangle. A ray
	// intersects iff 0 <= t. Segment intersects iff 0 <= t <= 1. Delay
	// dividing by d until intersection has been found to pierce triangle
	
	v3 ap = sub_v3(p, a);
	t = dot_v3(ap, n);
	if (t < 0.0f) {
		return 0;
	}
	//if (t > d) return 0; // For segment; exclude this code line for a ray test
	// Compute barycentric coordinate components and test if within bounds
	v3 e = cross(qp, ap);
	v = dot_v3(ac, e);
	if (v < 0.0f || v > d) {
		return 0;
	}
	w = -dot_v3(ab, e);
	if (w < 0.0f || v + w > d) {
		return 0;
	}
	// Segment/ray intersects triangle. Perform delayed division and
	// compute the last barycentric coordinate component
	
	float ood = 1.0f / d;
	t *= ood;
	v *= ood;
	w *= ood;
	u = 1.0f - v - w;
	
	*t_out = t;
	*normal = normalize_v3(n);
	return 1;
}

typedef struct baka_OBB {
	v3 mid;
	v3 ex;
	v3 min; //unrotated and centered
	v3 max; //unrotated and centered
	Quat rot;
	Quat inv_rot;
} baka_OBB;


baka_OBB baka_make_OBB(v3 mid, v3 ex, Quat rot) {
	baka_OBB obb;
	obb.mid = mid;
	obb.ex = ex;
	obb.min = sub_v3(vec3(0.0f, 0.0f, 0.0f), ex);
	obb.max = add_v3(vec3(0.0f, 0.0f, 0.0f), ex);
	obb.rot = rot;
	obb.inv_rot = inverse_quat(rot);
	return obb;
};

bool baka_raycast_obb_get_normal(v3 ray_origin, v3 ray_direction, baka_OBB obb, float *tmin_ret, v3 *normal) {
	
	v3 ray_origin_obbspace = sub_v3(ray_origin, obb.mid);
	ray_origin_obbspace = rotate_vec3_quat(ray_origin_obbspace, obb.inv_rot);
	
	v3 ray_direction_obbspace = rotate_vec3_quat(ray_direction, obb.inv_rot);
	
	v3 inv_ray_direction_obbspace = vec3(1/ray_direction_obbspace.X, 1/ray_direction_obbspace.Y,1/ray_direction_obbspace.Z);
	
    if (raycast_aabb_with__normal(ray_origin_obbspace, inv_ray_direction_obbspace, obb.min, obb.max, tmin_ret, normal)) {
		*normal = rotate_vec3_quat(*normal, obb.rot);
		return true;
	}
	return false;
	
}

// simple stack macro 
#define STACKMACRO(TYPE, FNAME)\
typedef struct TYPE##_Stack {\
	TYPE *els;\
	uint max;\
	uint num;\
} TYPE##_Stack;\
\
TYPE##_Stack baka_init_##FNAME(int number) {\
	TYPE##_Stack stack;\
	stack.els = malloc(number * sizeof(TYPE));\
	stack.num = 0;\
	stack.max = number;\
	return stack;\
}\
\
bool baka_push_##FNAME(TYPE##_Stack *stack, TYPE el) {\
	if (stack->num >= stack->max) {\
		return 0;\
	}\
	stack->els[stack->num] = el;\
	stack->num++;\
	return 1;\
}\
\

typedef struct baka_Shape {
	char type_id;
	union {
		baka_AABB aabb;
		baka_OBB obb;
		baka_sphere sphere;
		baka_triangle tri;
	};
	
} baka_Shape;

STACKMACRO(baka_Shape, shape_stack);

baka_AABB aabb_from_obb(baka_OBB obb) {
	v3 points[8];
	
	points[0] = vec3(1.0, 1.0, 1.0);
    points[1] = vec3(1.0, 1.0, -1.0);
    points[2] = vec3(1.0, -1.0, 1.0);
	points[3] = vec3(1.0, -1.0, -1.0);
	
    // left side points
    points[4] = vec3(-1.0, 1.0, 1.0);
    points[5] = vec3(-1.0, 1.0, -1.0);
    points[6] = vec3(-1.0, -1.0, 1.0);
	points[7] = vec3(-1.0, -1.0, -1.0);
	
	v3 max = vec3(0.0f, 0.0f, 0.0f);
	
	for (int i = 0; i < 8; i++) {
		v3 obb_point = mul_v3(obb.ex, points[i]); 
		obb_point = rotate_vec3_quat(obb_point, obb.rot);
		
		max.X = HMM_MAX(max.X, obb_point.X);
		max.Y = HMM_MAX(max.Y, obb_point.Y);
		max.Z = HMM_MAX(max.Z, obb_point.Z);
	}
	
	baka_AABB aabb = baka_make_AABB(obb.mid, max);
	return aabb;
}

baka_AABB aabb_from_sphere(baka_sphere sphere) {
	return baka_make_AABB(sphere.center, vec3(sphere.radius, sphere.radius, sphere.radius));
}

baka_AABB aabb_from_capsule(baka_capsule cap, v3 cap_pos, Quat cap_rot) {
	v3 p = add_v3(cap_pos, rotate_vec3_quat(cap.p, cap_rot));
	v3 q = add_v3(cap_pos, rotate_vec3_quat(cap.q, cap_rot));
	
	v3 min;
	v3 max;
	
	min.X = HMM_MIN(p.X, q.X);
	min.Y = HMM_MIN(p.Y, q.Y);
	min.Z = HMM_MIN(p.Z, q.Z);
	
	max.X = HMM_MAX(p.X, q.X);
	max.Y = HMM_MAX(p.Y, q.Y);
	max.Z = HMM_MAX(p.Z, q.Z);
	
	max = add_v3(max, vec3(cap.radius, cap.radius, cap.radius));
	min = sub_v3(min, vec3(cap.radius, cap.radius, cap.radius));
	
	return baka_make_AABB_min_max(min, max);
}

baka_AABB aabb_from_triangle(baka_triangle tri) {
	v3 min;
	v3 max;
	
	min.X = HMM_MIN(HMM_MIN(tri.a.X, tri.b.X), tri.c.X);// - 0.1f;
	min.Y = HMM_MIN(HMM_MIN(tri.a.Y, tri.b.Y), tri.c.Y);// - 0.1f;
	min.Z = HMM_MIN(HMM_MIN(tri.a.Z, tri.b.Z), tri.c.Z);// - 0.1f;
	
	max.X = HMM_MAX(HMM_MAX(tri.a.X, tri.b.X), tri.c.X);// + 0.1f;
	max.Y = HMM_MAX(HMM_MAX(tri.a.Y, tri.b.Y), tri.c.Y);// + 0.1f;
	max.Z = HMM_MAX(HMM_MAX(tri.a.Z, tri.b.Z), tri.c.Z);// + 0.1f;
	
	return baka_make_AABB_min_max(min, max);
}

enum PHYS_TYPE {
	PHYS_OBB,
	PHYS_SPHERE,
	PHYS_TRIANGLE
};


baka_AABB aabb_from_object(baka_Shape object) {
	
	if (object.type_id == PHYS_OBB) {
		return aabb_from_obb(object.obb);
	} 
	
	if (object.type_id == PHYS_SPHERE) {
		return aabb_from_sphere(object.sphere);
	} 
	
	if (object.type_id == PHYS_TRIANGLE) {
		return aabb_from_triangle(object.tri);
	}
	// TODO: return a default aabb and think about what a default aabb should be
	else {
		return aabb_from_triangle(object.tri);
	}
}

typedef struct aabb_tree_node {
	baka_AABB aabb;
	int object_type;
	int object_index;
	
	int parent;
	int left_child;
	int right_child;
	
	uint64_t id;
} aabb_tree_node;

// TODO: B trees 
typedef struct baka_aabb_binary_tree {
	uint max_object;
	uint next_object;
	
	aabb_tree_node *nodes;
	
	int root_node_index;
	
	int *free_indices;
	uint next_free_index;
	
} baka_aabb_binary_tree;


typedef struct baka_Body {
	uint shape_index;
	uint num_shapes;
	// TODO: add a tree for large objects
	//baka_aabb_binary_tree tree;
} baka_Body;

STACKMACRO(baka_Body, body_stack);

typedef struct baka_Body_Collection {
	baka_Body_Stack bodies;
	baka_Shape_Stack shapes;
} baka_Body_Collection;

baka_Body_Collection baka_init_body_collection(uint num_shapes, uint num_bodies) {
	baka_Body_Collection col;
	col.shapes = baka_init_shape_stack(num_shapes);
	col.bodies = baka_init_body_stack(num_bodies);
	return col;
}

bool is_leaf(aabb_tree_node *node) {
	return (node->left_child == -1 || node->right_child == -1);
}


void baka_aabb_find_contacts_tree(baka_aabb_binary_tree *tree, baka_AABB *aabb, uint64_t *collision_ids, int *collision_ids_ret_count, int collision_ids_max) {
	START_TIME;
	int collision_ids_count = 0;
	if (tree->next_object == 0) {
		*collision_ids_ret_count = 0;
		return;
	}
	int ov_stack[tree->next_object];
	int ov_current = 0;
	ov_stack[0] = tree->root_node_index;
	
	while(ov_current >= 0) {
		
		int node_index = ov_stack[ov_current];
		aabb_tree_node *node = &tree->nodes[node_index];
		
		ov_current--;
		
		bool didhit = baka_test_AABB_AABB(node->aabb, *aabb);
		
		if (didhit) {
			if(is_leaf(node)) {
				if (collision_ids_count < collision_ids_max) {
					collision_ids[collision_ids_count] = node->id;
					collision_ids_count++;
				}
				
			} else {
				ov_current++;
				ov_stack[ov_current] = node->left_child;
				
				ov_current++;
				ov_stack[ov_current] = node->right_child;
			}
		}
	}
	
	*collision_ids_ret_count = collision_ids_count;
	END_TIME;
	return;
}


bool baka_raycast_tree_return_normal(
baka_aabb_binary_tree *tree, v3 ray_origin, v3 ray_direction, float *tmin_ret, v3 *normal_ret, uint64_t *collision_ids, int *collision_ids_ret_count, int collision_ids_max) {
	
	v3 inv_ray_direction = vec3(1/ray_direction.X, 1/ray_direction.Y, 1/ray_direction.Z);
	
	float tmin = INFINITY;
	float temp_tmin = INFINITY;
	v3 temp_normal;
	v3 normal;
	
	int collision_ids_count = 0;
	
	int ov_stack[tree->next_object];
	int ov_current = 0;
	ov_stack[0] = tree->root_node_index;
	
	while(ov_current >= 0) {
		
		int node_index = ov_stack[ov_current];
		aabb_tree_node *node = &tree->nodes[node_index];
		
		ov_current--;
		
		bool didhit =baka_raycast_aabb_get_normal(ray_origin, inv_ray_direction, node->aabb, &temp_tmin, &temp_normal);
		
		if (didhit) {
			tmin = temp_tmin;
			normal = temp_normal;
			
			if(is_leaf(node)) {
				if (collision_ids_count < collision_ids_max) {
					collision_ids[collision_ids_count] = node->id;
					collision_ids_count++;
				}
				
			} else {
				ov_current++;
				ov_stack[ov_current] = node->left_child;
				
				ov_current++;
				ov_stack[ov_current] = node->right_child;
			}
		}
	}
	
	*collision_ids_ret_count = collision_ids_count;
	*normal_ret = normal;
	*tmin_ret = tmin;
	return true;
}



baka_AABB merge_aabb(baka_AABB left, baka_AABB right) {
	v3 max;
	v3 min;
	
	max.X = HMM_MAX(left.max.X, right.max.X);
	max.Y = HMM_MAX(left.max.Y, right.max.Y);
	max.Z = HMM_MAX(left.max.Z, right.max.Z);
	
	min.X = HMM_MIN(left.min.X, right.min.X);
	min.Y = HMM_MIN(left.min.Y, right.min.Y);
	min.Z = HMM_MIN(left.min.Z, right.min.Z);
	
	return baka_make_AABB_min_max(min, max);
};

float aabb_surface_area(baka_AABB aabb) {
	float surface;
	surface = aabb.ex.X * aabb.ex.Y * 2.0f;
	surface = surface + (aabb.ex.X * aabb.ex.Z * 2.0f);
	surface = surface + (aabb.ex.Y * aabb.ex.Z * 2.0f);
	
	return surface;
}

// returns the index of the allocated node
int tree_allocate_node(baka_aabb_binary_tree *tree) {
	//printf("allocing tree->next_object %i\n", tree->next_object);
	if (tree->next_free_index != 0) {
		
		// there is some freed node we can use
		tree->next_free_index = tree->next_free_index-1;
		int free_node_index = tree->free_indices[tree->next_free_index];
		
		tree->nodes[free_node_index].parent = -1;
		tree->nodes[free_node_index].left_child = -1;
		tree->nodes[free_node_index].right_child = -1;
		
		//printf("allocating node %i which was freed before\n", free_node_index);
		return free_node_index;
	}
	else {
		if (tree->next_object < tree->max_object) {
			int new_node_index = tree->next_object;
			tree->next_object = tree->next_object + 1;
			//printf("allocating node %i which is new\n", new_node_index);
			return new_node_index;
		}
	}
	printf("couldn't allocate tree node\n");
	return -1;
}

void tree_deallocate_node(baka_aabb_binary_tree *tree, int node_index) {
	tree->nodes[node_index].parent = -1;
	tree->nodes[node_index].left_child = -1;
	tree->nodes[node_index].right_child = -1;
	if (tree->next_free_index < tree->max_object) {
		//printf("freeing node %i\n", node_index);
		tree->free_indices[tree->next_free_index] = node_index;
		tree->next_free_index++;
	}
}


// from https://github.com/JamesRandall/SimpleVoxelEngine/blob/master/voxelEngine/src/AABBTree.cpp

void fix_upwards_tree(baka_aabb_binary_tree *tree, int tree_node_index) {
	//printf("fix_upwards_tree tree_node_index %i parent %i\n", tree_node_index, tree->nodes[tree_node_index].parent);
	while (1) {
		if (tree_node_index == -1) {
			return;
		}
		//printf("in here again\n");
		
		aabb_tree_node *tree_node = &tree->nodes[tree_node_index];
		
		aabb_tree_node *left_node = &tree->nodes[tree_node->left_child];
		aabb_tree_node *right_node = &tree->nodes[tree_node->right_child];
		
		tree_node->aabb = merge_aabb(left_node->aabb, right_node->aabb);
		
		tree_node_index = tree_node->parent;
		
	}
}

int tree_insert_aabb(baka_aabb_binary_tree *tree, baka_AABB aabb, uint64_t identifier) {
	START_TIME;
	int node_index = tree_allocate_node(tree);
	
	//printf("inserting node %i\n", node_index);
	
	if (node_index == -1) {
		printf("aabbb node could not be allocated\n");
		return -1;
	}
	
	aabb_tree_node * node = &tree->nodes[node_index];
	node->aabb = aabb;
	
	node->parent = -1;
	node->left_child = -1;
	node->right_child = -1;
	
	node->id = identifier;
	
	if (tree->root_node_index == -1) {
		//printf("inserting as root node\n");
		tree->root_node_index = node_index;
		return node_index;
	}
	
	int tree_node_index = tree->root_node_index;
	
	while(1) {
		aabb_tree_node * tree_node = &tree->nodes[tree_node_index];
		int left_index = tree_node->left_child;
		int right_index = tree_node->right_child;
		if (is_leaf(tree_node)) {
			break;
		}
		
		aabb_tree_node * left_node = &tree->nodes[left_index];
		aabb_tree_node * right_node = &tree->nodes[right_index];
		
		baka_AABB combined_aabb = merge_aabb(tree_node->aabb, node->aabb);
		
		float new_parent_node_cost = 2.0f * aabb_surface_area(combined_aabb);
		float minimum_pushdown_cost = 2.0f * (aabb_surface_area(combined_aabb) - aabb_surface_area(tree_node->aabb));
		
		float cost_left;
		float cost_right;
		
		baka_AABB left_merge = merge_aabb(node->aabb, left_node->aabb);
		baka_AABB right_merge = merge_aabb(node->aabb, right_node->aabb);
		
		if (is_leaf(left_node)) {
			cost_left = minimum_pushdown_cost + aabb_surface_area(left_merge); 
		} else {
			cost_left = aabb_surface_area(left_merge) - aabb_surface_area(left_node->aabb) + minimum_pushdown_cost; 
		}
		
		if (is_leaf(right_node)) {
			cost_right = minimum_pushdown_cost + aabb_surface_area(right_merge); 
		} else {
			cost_right = aabb_surface_area(right_merge) - aabb_surface_area(right_node->aabb) + minimum_pushdown_cost; 
		}
		
		if (new_parent_node_cost < cost_left && new_parent_node_cost < cost_right) {
			break;
		}
		
		if (cost_left < cost_right) {
			tree_node_index = left_index;
		} else {
			tree_node_index = right_index;
		}
	}
	
	int leaf_sibling_index = tree_node_index;
	aabb_tree_node * leaf_sibling = &tree->nodes[leaf_sibling_index];
	
	int old_parent_index = leaf_sibling->parent;
	
	// allocate a new node
	//printf("here we are allocating a new parent node\n");
	int new_parent_index = tree_allocate_node(tree);
	aabb_tree_node * new_parent = &tree->nodes[new_parent_index];
	
	new_parent->parent = old_parent_index;
	new_parent->aabb = merge_aabb(node->aabb, leaf_sibling->aabb);
	new_parent->left_child = leaf_sibling_index;
	new_parent->right_child = node_index;
	
	node->parent = new_parent_index;
	leaf_sibling->parent = new_parent_index;
	
	if (old_parent_index == -1) {
		tree->root_node_index = new_parent_index;
	}
	else {
		aabb_tree_node *old_parent = &tree->nodes[old_parent_index];
		if (old_parent->left_child == leaf_sibling_index) {
			old_parent->left_child = new_parent_index;
		}
		else {
			old_parent->right_child = new_parent_index;
		}
	}
	tree_node_index = node->parent;
	fix_upwards_tree(tree, tree_node_index);
	
	//printf("and this fix upwards tree never returns\n");
	END_TIME;
	return node_index;
}

void tree_remove_leaf_node(baka_aabb_binary_tree *tree, int leaf_node_index) {
	
	//printf("removing leaf node %i parent %i children %i %i\n", leaf_node_index, tree->nodes[leaf_node_index].parent, tree->nodes[leaf_node_index].left_child, tree->nodes[leaf_node_index].right_child);
	//printf("tree->root_node_index %i\n", tree->root_node_index);
	
	if (leaf_node_index == tree->root_node_index) {
		tree->root_node_index = -1;
		tree_deallocate_node(tree, leaf_node_index);
		//printf("removed leaf node %i\n", leaf_node_index);
		return;
	}
	
	aabb_tree_node *node = &tree->nodes[leaf_node_index];
	
	int parent_node_index = node->parent;
	aabb_tree_node *parent = &tree->nodes[node->parent];
	
	int grandparent_node_index = parent->parent;
	
	int sibling_node_index = -1;
	if (leaf_node_index == parent->left_child) {
		sibling_node_index = parent->right_child;
	}
	else {
		sibling_node_index = parent->left_child;
	}
	assert(sibling_node_index != -1);
	
	aabb_tree_node *sibling_node = &tree->nodes[sibling_node_index];
	
	if (grandparent_node_index != -1) {
		
		aabb_tree_node *grandparent_node = &tree->nodes[grandparent_node_index];
		
		if (grandparent_node->left_child == node->parent) {
			grandparent_node->left_child = sibling_node_index;
		}
		else {
			grandparent_node->right_child = sibling_node_index;
		}
		sibling_node->parent = grandparent_node_index;
		
		tree_deallocate_node(tree, parent_node_index);
		
		//printf("remove tree->nodes[tree->root_node_index].parent %i\n", tree->nodes[tree->root_node_index].parent);
		
		fix_upwards_tree(tree, grandparent_node_index);
	}
	else {
		// with no grandparent the parent is the root, and our sibling becomes the new root
		tree->root_node_index = sibling_node_index;
		sibling_node->parent = -1;
		tree_deallocate_node(tree, parent_node_index);
	}
	tree_deallocate_node(tree, leaf_node_index);
	//printf("removed leaf node %i\n", leaf_node_index);
}

// NOTE: temporary update that could be better, by reusing the same node instead of allocating and deallocing, 
// any benefit would probably be pretty marginal though
int tree_update_aabb_node(baka_aabb_binary_tree *tree, int node_index, baka_AABB aabb) {
	//printf("tree_update_aabb_node %i\n", node_index);
	if(node_index == -1) {
		return -1;
	}
	
	uint64_t id = tree->nodes[node_index].id; 
	tree_remove_leaf_node(tree, node_index);
	//printf("tree->root_node_index %i children %i %i\n", tree->root_node_index, tree->nodes[tree->root_node_index].left_child, tree->nodes[tree->root_node_index].right_child);
	int ret = tree_insert_aabb(tree, aabb, id);
	return ret;
	//return -1;
}

void tree_insert_obb(baka_aabb_binary_tree *tree, baka_OBB obb, uint64_t identifier) {
	
	baka_AABB aabb = aabb_from_obb(obb);
	
	tree_insert_aabb(tree, aabb, identifier);
	
}

void tree_insert_sphere(baka_aabb_binary_tree *tree, baka_sphere sphere, uint64_t identifier) {
	
	baka_AABB aabb = aabb_from_sphere(sphere);
	
	tree_insert_aabb(tree, aabb, identifier);
	
}

void tree_insert_triangle(baka_aabb_binary_tree *tree, baka_triangle tri, uint64_t identifier) {
	
	baka_AABB aabb = aabb_from_triangle(tri);
	
	tree_insert_aabb(tree, aabb, identifier);
	
}



void tree_insert_object(baka_aabb_binary_tree *tree, baka_Shape object, uint64_t identifier) {
	tree_insert_aabb(tree, aabb_from_object(object), identifier);
	
}


void test_character_resolution(baka_capsule *cap, v3 *cap_positions, Quat * cap_rotations,v3 * cap_displacement, int num_cap) {
	START_TIME;
	for (int i = 0; i < num_cap; i++) {
		baka_capsule * cap_i = &cap[i]; 
		v3 p1 = add_v3(rotate_vec3_quat(cap_i->p, cap_rotations[i]), cap_positions[i]);
		v3 q1 = add_v3(rotate_vec3_quat(cap_i->q, cap_rotations[i]), cap_positions[i]);
		
		
		for (int j = i+1; j < num_cap; j++) {
			baka_capsule * cap_j = &cap[j];
			
			v3 p2 = add_v3(rotate_vec3_quat(cap_j->p, cap_rotations[j]), cap_positions[j]);
			v3 q2 = add_v3(rotate_vec3_quat(cap_j->q, cap_rotations[j]), cap_positions[j]);
			
			float s;
			float t;
			v3 c1;
			v3 c2;
			
			float ret_len = baka_closestpt_segment_segment(p1, q1, p2, q2, &s, &t, &c1, &c2);
			float combined_radius = cap_i->radius + cap_j->radius;
			
			v3 c1_to_c2 = sub_v3(c1, c2);
			v3 c2_to_c1 = sub_v3(c2, c1);
			
			float c1_to_c2_sqlen = dot_v3(c1_to_c2, c1_to_c2);
			float c1_to_c2_len = HMM_SquareRootF(c1_to_c2_sqlen);
			float c1_to_cap_j_len = c1_to_c2_len - cap_j->radius;
			float c2_to_cap_i_len = c1_to_c2_len - cap_i->radius;
			
			if (c1_to_cap_j_len > cap_i->radius) {
				continue;
			}
			
			if (c2_to_cap_i_len > cap_j->radius) {
				continue;
			}
			
			float penetration_depth_i = c1_to_cap_j_len - cap_i->radius;
			float penetration_depth_j = c2_to_cap_i_len - cap_j->radius;
			
			if (cap_i->num_dyn_contacts < MAX_CONTACTS) {
				
				cap_i->dyn_contacts[cap_i->num_dyn_contacts].point = add_v3(c1, mul_v3f(normalize_v3(c1_to_c2), cap_i->radius));
				cap_i->dyn_contacts[cap_i->num_dyn_contacts].normal = normalize_v3(c1_to_c2);
				cap_i->dyn_contacts[cap_i->num_dyn_contacts].depth = penetration_depth_i;
				cap_i->dyn_contacts[cap_i->num_dyn_contacts].broken = 0;
				cap_i->num_dyn_contacts++;
			}
			
			if (cap_j->num_dyn_contacts < MAX_CONTACTS) {
				// TODO: this can be optimized by using calculate results from cap_i
				cap_j->dyn_contacts[cap_j->num_dyn_contacts].point = add_v3(c2, mul_v3f(normalize_v3(c2_to_c1), cap_j->radius));
				cap_j->dyn_contacts[cap_j->num_dyn_contacts].normal = normalize_v3(c2_to_c1);
				cap_j->dyn_contacts[cap_j->num_dyn_contacts].depth = penetration_depth_j;
				cap_j->dyn_contacts[cap_j->num_dyn_contacts].broken = 0;
				cap_j->num_dyn_contacts++;
			}
		}
	}
	END_TIME;
}

/*
baka_intersect_or_distance_segment_triangle(
v3 p,
v3 q,
struct baka_triangle tri)
{
v3 a = tri.a;
v3 b = tri.b;
v3 c = tri.c;

v3 ab = sub_v3(b, a);
v3 ac = sub_v3(c, a);
v3 ap = sub_v3(p, a);
v3 bp = sub_v3(p, b);
v3 cp = sub_v3(p, c);

float d1 = dot_v3(ab, ap);
float d2 = dot_v3(ac, ap);
float d3 = dot_v3(ab, bp);
float d4 = dot_v3(ac, bp);
float d5 = dot_v3(ab, cp);
float d6 = dot_v3(ac, cp);

float va = d3*d6 - d5*d4;
float vb = d5*d2 - d1*d6;	
float vc = d1*d4 - d3*d2;

// Check if P in vertex region outside A
if (d1 <= 0.0f && d2 <= 0.0f) return a; // barycentric coordinates (1,0,0)

// Check if P in vertex region outside B
if (d3 >= 0.0f && d4 <= d3) return b; // barycentric coordinates (0,1,0)

// Check if P in vertex region outside C
if (d6 >= 0.0f && d5 <= d6) return c; // barycentric coordinates (0,0,1)

// Check if P in edge region of AB, if so return projection of P onto AB
if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
float v = d1 / (d1 - d3);
return add_v3(a, mul_v3f(ab, v)); // barycentric coordinates (1-v,v,0)
}

// Check if P in edge region of AC, if so return projection of P onto AC
if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
float w = d2 / (d2 - d6);
return add_v3(a, mul_v3f(ac, w)); // barycentric coordinates (1-w,0,w)
}

// Check if P in edge region of BC, if so return projection of P onto BC
if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
return add_v3(b, mul_v3f(sub_v3(c, b), w)); // barycentric coordinates (0,1-w,w)
}

// P inside face region. Compute Q through its barycentric coordinates (u,v,w)
float denom = 1.0f / (va + vb + vc);
float v = vb * denom;
float w = vc * denom;
return add_v3(add_v3(a, mul_v3f(ab, v)), mul_v3f(ac, w));
// = u*a + v*b + w*c, u = va * denom = 1.0f-v-w
}
*/




#endif /* BAKA_IMPLEMENTATION */

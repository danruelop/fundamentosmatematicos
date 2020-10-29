#ifndef _MATHS_FUNCS_H_
#define _MATHS_FUNCS_H_

#define _USE_MATH_DEFINES
#include <math.h>
// const used to convert degrees into radians
#define TAU 2.0 * M_PI
#define ONE_DEG_IN_RAD static_cast<float>(( 2.0f * M_PI ) / 360.0) // 0.017444444
#define ONE_RAD_IN_DEG static_cast<float>(360.0 / ( 2.0 * M_PI )) // 57.2957795

struct vec2;
struct vec3;
struct vec4;
struct versor;

struct vec2 {
	vec2();
	vec2( float x, float y );
	union {
		float v[2];
		struct {
			float x, y;
		};
	};
};

struct vec3 {
	vec3();
	// create from 3 scalars
	vec3( float x, float y, float z );
	// create from vec2 and a scalar
	vec3( const vec2 &vv, float z );
	// create from truncated vec4
	vec3( const vec4 &vv );
	// add vector to vector
	vec3 operator+( const vec3 &rhs ) const;
	// add scalar to vector
	vec3 operator+( float rhs ) const;
	// because user's expect this too
	vec3 &operator+=( const vec3 &rhs );
	// subtract vector from vector
	vec3 operator-( const vec3 &rhs ) const;
	// add vector to vector
	vec3 operator-( float rhs ) const ;
	// because users expect this too
	vec3 &operator-=( const vec3 &rhs );
	// multiply with scalar
	vec3 operator*( float rhs );
	// because users expect this too
	vec3 &operator*=( float rhs );
	// divide vector by scalar
	vec3 operator/( float rhs );
	// because users expect this too
	vec3 &operator=( const vec3 &rhs );

	// internal data
	union {
		float v[3];
		struct {
			float x, y, z;
		};
	};
	inline float & operator[] (size_t Comp) {
		return v[Comp];
	}
	inline const float & operator[] (size_t Comp) const {
		return v[Comp];
	}

};

struct vec4 {
	vec4();
	vec4( float x, float y, float z, float w );
	vec4( const vec2 &vv, float z, float w );
	vec4( const vec3 &vv, float w );

	union {
		float v[4];
		struct {
			float x, y, z, w;
		};
	};
	// multiply with scalar
	vec4 operator*( float rhs );
	inline float & operator[] (size_t Comp) {
		return v[Comp];
	}
	inline const float & operator[] (size_t Comp) const {
		return v[Comp];
	}
};

/* stored like this:
a d g
b e h
c f i */
struct mat3 {
	mat3();
	mat3( float a, float b, float c, float d, float e, float f, float g, float h, float i );
	union {
		float m[9];
		float c[3][3];
		vec3 col[3];
	};

	//void QDUdecomposition(mat3& kQ, vec3& kD, vec3& kU) const;
	inline const vec3& operator[] (size_t i) const {
		return col[i];
	}
	inline vec3& operator[] (size_t i) {
		return col[i];
	}
};

/* stored like this: (column-major
0 4 8  12
1 5 9  13
2 6 10 14
3 7 11 15*/
struct versor;

struct mat4 {
	mat4();
	// note! this is entering components in ROW-major order
	mat4( float a, float b, float c, float d, float e, float f, float g, float h, float i, float j, float k, float l, float mm, float n, float o, float p );
	explicit mat4(const mat3& rot) { setRotation(rot); }
	vec4 operator*( const vec4 &rhs );
	mat4 operator*( const mat4 &rhs );
	mat4 &operator=( const mat4 &rhs );
	mat3 getRotation() const;
	void setRotation(const mat3& rot);
	vec4 getColumn(int i) const;
	vec4 getRow(int i) const;
	void setColumn(int i, const vec4& v );
	void setRow(int i, const vec4& v);
	void decompose(versor& q, vec3& pos, vec3& scale) const;
	//void decompose2(versor& q, vec3& pos, vec3& scale) const;

	union {
		float m[16];
		float c[4][4];
		vec4 col[4];
	};

	inline vec4& operator[] (size_t i) {
		return col[i];
	}
	inline const vec4& operator[] (size_t i) const {
		return col[i];
	}
};

struct versor {
	versor();
	versor(float x, float y, float z, float w);
	versor(const mat3& rot) ;
	versor operator/( float rhs );
	versor operator*( float rhs );
	versor operator*( const versor &rhs );
	versor operator+( const versor &rhs );
	union {
		float q[4];
		struct {
			float w, x, y, z;
		};
	};
};

void print( const vec2 &v );
void print( const vec3 &v );
void print( const vec4 &v );
void print( const mat3 &m );
void print( const mat4 &m );
// vector functions
float length( const vec3 &v );
float length2( const vec3 &v );
vec3 normalise( const vec3 &v );
float dot( const vec3 &a, const vec3 &b );
vec3 cross( const vec3 &a, const vec3 &b );
vec3 homogeneous( const vec4 &a );
float get_squared_dist( vec3 from, vec3 to );
float direction_to_heading( vec3 d );
vec3 heading_to_direction( float degrees );
// matrix functions
mat3 zero_mat3();
mat3 identity_mat3();
mat4 zero_mat4();
mat4 identity_mat4();
float determinant( const mat4 &mm );
mat4 inverse( const mat4 &mm );
mat4 transpose( const mat4 &mm );
mat3 transpose( const mat3 &mm );
// affine functions
mat4 translate( const mat4 &m, const vec3 &v );
mat4 rotate_x_deg( const mat4 &m, float deg );
mat4 rotate_y_deg( const mat4 &m, float deg );
mat4 rotate_z_deg( const mat4 &m, float deg );
mat4 scaler( const mat4 &m, const vec3 &v );
// camera functions
mat4 look_at( const vec3 &cam_pos, vec3 targ_pos, const vec3 &up );
mat4 perspective( float fovy, float aspect, float near, float far );
// quaternion functions
versor quat_from_axis_rad( float radians, float x, float y, float z );
versor quat_from_axis_deg( float degrees, float x, float y, float z );
mat4 quat_to_mat4( const versor &q );
float dot( const versor &q, const versor &r );
versor slerp( const versor &q, const versor &r );
// stupid overloading wouldn't let me use const
versor normalise( versor &q );
void print( const versor &q );
versor slerp( versor &q, versor &r, float t );
#endif

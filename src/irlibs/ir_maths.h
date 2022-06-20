#pragma once

#include "ir_types.h"

#ifndef IR_ASSERT
#define IR_ASSERT(ASSERT)
#define IR_NOT_NULL(PTR)
#define IR_INVALID_CASE
#define IR_SOFT_ASSERT(ASSERT)
#endif

#ifndef IR_INTEGER_MATHS
#define IR_INTEGER_MATHS


inline u64 
intr_bsr(u64 v)
{
    u64 result = 0;
#ifdef IR_ARCH_ARM
    IR_INVALID_CASE;
    NOT_IMPLEMENTED
#else
    asm(
        "bsrq %1, %0"
        :"=r"( result )        /* output */
        :"r"( v )         /* input */
        :         /* clobbered register */
        );
    
#endif
    return result;
}

/**
 * @brief integer operations
 */

inline s32
s32_abs(s32 value)
{
    s32 result = value * ((value <= 0) ? -1 : 1);
    return result;
}

inline s64
s64_abs(s64 value)
{
    s64 result = value * ((value <= 0) ? -1 : 1);
    return result;
}

inline s32
s32_sign(s32 value)
{
    s32 result = value >=0 ? 1 : -1;
    return result;
}

inline u64
u64_max(u64 v1, u64 v2)
{
    u64 result = v1 > v2 ? v1 : v2;
    return result;
}

inline u64
u64_min(u64 v1, u64 v2)
{
    u64 result = v1 < v2 ? v1 : v2;
    return result;
}

inline s64
s64_max(s64 v1, s64 v2)
{
    s64 result = v1 > v2 ? v1 : v2;
    return result;
}

inline s64
s64_min(s64 v1, s64 v2)
{
    s64 result = v1 < v2 ? v1 : v2;
    return result;
}

//http://graphics.stanford.edu/~seander/bithacks.html
inline
u64 u64_get_nearest_higher_or_equal_pow2(u64 v)
{
    //TODO(Michael) use find first set intrinsic ?/ SIMD ?
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    v++;
    
    return v;
}

inline
u64 u64_get_nearest_lower_or_equal_pow2(u64 v)
{
    //TODO(Michael) use find first set intrinsic ?
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    return (v & ~(v >> 1));
}
inline s32
u64_log2_rounded_down(u64 v)
{
    return intr_bsr(v);
}

inline s32
u64_log2_rounded_up(u64 v)
{
    return intr_bsr(u64_get_nearest_higher_or_equal_pow2(v));
}


#define SWAPFUNCTION(TYPE) \
inline void \
TYPE_swap(TYPE* a, TYPE* b) \
{\
TYPE t = *a;\
*a = *b;\
*b = t;\
}

SWAPFUNCTION(u8);
SWAPFUNCTION(u16);
SWAPFUNCTION(u32);
SWAPFUNCTION(u64);
SWAPFUNCTION(b8);
SWAPFUNCTION(s8);
SWAPFUNCTION(s16);
SWAPFUNCTION(s32);
SWAPFUNCTION(s64);


#undef SWAPFUNCTION
#endif

#ifndef IR_FLOAT_MATHS
#define IR_FLOAT_MATHS
#define IR_PI 3.1415926535f
#define IR_F32_EPS 0.00000006


/**
 * @brief float operations
 */

inline f32
intr_sqrt(f32 x)
{
#ifdef IR_ARCH_ARM
    IR_INVALID_CASE;
    NOT IMPLEMENTED
#else
    asm(
        "flds %1;"
        "fsqrt;"
        "fstps %1;"
        : "+m" ( x )
        );
#endif
    
    return x;
}

inline f32
intr_sin(f32 x)
{
#ifdef IR_ARCH_ARM
    IR_INVALID_CASE;
    NOT IMPLEMENTED
#else
    asm(
        "flds %1;"
        "fsin;"
        "fstps %1;"
        : "+m" ( x )
        );
#endif
    
    return x;
}

inline f64
f64_abs(f64 value)
{
    f64 result = value * ((value <= 0) ? -1 : 1);
    return result;
}

inline b8
f64_eq(f64 v1, f64 v2)
{
    return f64_abs(v1 - v2) < IR_F32_EPS;
}


inline f32
f32_abs(f32 value)
{
    f32 result = value * ((value <= 0) ? -1 : 1);
    return result;
}

inline f32
f32_floor(f32 value)
{
    f32 result = (f32)(s64)value;
    return result;
}

inline f32
f32_ceil(f32 value)
{
    f32 result = float(value+1);
    return result;
}

inline f32
f32_round(f32 value)
{
    f32 result = f32_floor(value+0.5f);
    return result;
}

inline f32
f32_sign(f32 value)
{
    f32 result = value >=0 ? 1 : -1;
    return result;
}

inline f32
f32_sq(f32 value)
{
    f32 result = value * value;
    return result;
}

inline f32
f32_sqrt(f32 value)
{
    f32 result = intr_sqrt(value);
    return result;
}

inline f32
f32_sin(f32 value)
{
    f32 result = intr_sin(value);
    return result;
}

inline f32
f32_cos(f32 value)
{
    f32 result = f32_sin(value + IR_PI/2.0f);
    return result;
}

inline f32
f32_tan(f32 value)
{
    f32 result = f32_sin(value)/f32_cos(value);
    return result;
}

inline f32
f32_toRad(f32 degrees)
{
    f32 result = degrees * IR_PI / 180.0f;
    return result;
}

inline f32
f32_toDeg(f32 radians)
{
    f32 result = radians * 180.0f/IR_PI;
    return result;
}

inline f32
f32_max(f32 v1, f32 v2)
{
    f32 result = v1 > v2 ? v1 : v2;
    return result;
}

inline f32
f32_max(f32* values, u32 count)
{
    f32 result=0;
    if(count != 0)
    {
        result = values[0];
        for(u32 i = 1; i < count; ++i)
        {
            result = f32_max(result,values[i]);
        }
    }
    return result;
}

inline f32
f32_min(f32 v1, f32 v2)
{
    f32 result = v1 < v2 ? v1 : v2;
    return result;
}

inline f32
f32_min(f32* values, u32 count)
{
    f32 result=0;
    if(count != 0)
    {
        result = values[0];
        for(u32 i = 1; i < count; ++i)
        {
            result = f32_min(result,values[i]);
        }
    }
    return result;
}

/**
 * @param value to be clamped
 * @param minimum (inclusive)
 * @param maximum (inclusive)
 * @return clamped value
 */
inline f32
f32_clamp(f32 value, f32 minimum, f32 maximum)
{
    f32 result = f32_min(maximum, f32_max(value, minimum));
    return result;
}

#endif

#ifndef IR_VECTOR_MATHS
#define IR_VECTOR_MATHS

/**
 * @brief v2
 */

union v2
{
    struct
    {
        f32 x,y;
    };
    
    f32 E[2];
};

inline b8
operator==(v2 left, v2 right)
{
    b8 result = left.x == right.x && left.y == right.y;
    return result;
}

inline b8
operator!=(v2 left, v2 right)
{
    b8 result = !(left==right);
    return result;
}

inline v2
operator+(v2 left, v2 right)
{
    v2 result = {left.x + right.x, left.y + right.y};
    return result;
}

inline v2
operator-(v2 left, v2 right)
{
    v2 result = {left.x - right.x, left.y - right.y};
    return result;
}

inline v2
operator-(v2 self)
{
    v2 result = {-self.x, -self.y};
    return result;
}

inline v2
operator*(v2 self, f32 scalar)
{
    v2 result = {self.x * scalar,self.y * scalar};
    return result;
}

inline v2
operator*(f32 scalar, v2 self)
{
    v2 result = self * scalar;
    return result;
}

inline v2
operator/(v2 self, f32 scalar)
{
    v2 result = {self.x / scalar,self.y / scalar};
    return result;
}

inline v2
operator/(f32 scalar, v2 self)
{
    v2 result = self / scalar;
    return result;
}

inline f32
lensq(v2 self)
{
    f32 result = f32_sq(self.x) + f32_sq(self.y);
    return result;
}

inline f32
len(v2 self)
{
    f32 result = f32_sqrt(lensq(self));
    return result;
}

inline v2
normalized(v2 self)
{
    v2 result = self / len(self);
    return result;
}

/**
 * @brief v3
 */

union v3
{
    struct
    {
        f32 x,y,z;
    };
    struct
    {
        f32 r,g,b;
    };
    struct
    {
        v2 xy;
        f32 _ignored;
    };
    struct
    {
        f32 _ignored2;
        v2 yz;
    };
    
    f32 E[3];
};

inline b8
operator==(v3 left, v3 right)
{
    b8 result = left.x == right.x && left.y == right.y && left.z == right.z;
    return result;
}

inline b8
operator!=(v3 left, v3 right)
{
    b8 result = !(left==right);
    return result;
}

inline v3
operator+(v3 left,v3 right)
{
    v3 result = {left.x + right.x, left.y + right.y, left.z + right.z};
    return result;
}

inline v3
operator-(v3 left,v3 right)
{
    v3 result = {left.x - right.x, left.y - right.y, left.z - right.z};
    return result;
}

inline v3
operator-(v3 self)
{
    v3 result = {-self.x, -self.y, -self.z};
    return result;
}

inline v3
operator*(v3 self, f32 scalar)
{
    v3 result = {self.x * scalar,self.y * scalar, self.z * scalar};
    return result;
}

inline v3
operator*(f32 scalar, v3 self)
{
    v3 result = self * scalar;
    return result;
}

inline v3
operator/(v3 self, f32 scalar)
{
    v3 result = {self.x / scalar,self.y / scalar, self.z / scalar};
    return result;
}

inline v3
operator/(f32 scalar, v3 self)
{
    v3 result = self / scalar;
    return result;
}

inline f32
lensq(v3 self)
{
    f32 result = f32_sq(self.x) + f32_sq(self.y) + f32_sq(self.z);
    return result;
}

inline f32
len(v3 self)
{
    f32 result = f32_sqrt(lensq(self));
    return result;
}

inline v3
normalized(v3 self)
{
    v3 result = self / len(self);
    return result;
}

/**
 * @brief Inner or dot product
 */
inline f32
inner(v3 left, v3 right)
{
    f32 result = left.x * right.x + left.y * right.y + left.z * right.z;
    return result;
}

inline v3
cross(v3 left, v3 right)
{
    v3 result = {left.y*right.z - left.z*right.y,
        left.z*right.x - left.x*right.z,
        left.x*right.y - left.y*right.x};
    return result;
}


/**
 * @brief v4
 */

union v4
{
    struct
    {
        f32 x,y,z,w;
    };
    struct
    {
        f32 r,g,b,a;
    };
    struct
    {
        union
        {
            v3 xyz;
            v3 rgb;
        };
        f32 _ignored;
    };
    struct
    {
        union
        {
            v3 yzw;
            v3 gba;
        };
        f32 _ignored2;
    };
    struct
    {
        union
        {
            v2 xy;
            v2 rg;
        };
        union
        {
            v2 zw;
            v2 ba;
        };
    };
    struct
    {
        f32 _ignored3;
        union
        {
            v2 yz;
            v2 gb;
        };
        f32 _ignored4;
    };
    
    f32 E[4];
};

inline b8
operator==(v4 left, v4 right)
{
    b8 result = left.x == right.x && left.y == right.y && left.z == right.z && right.w == left.w;
    return result;
}

inline b8
operator!=(v4 left, v4 right)
{
    b8 result = !(left==right);
    return result;
}

inline v4
operator+(v4 left,v4 right)
{
    v4 result = {left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w};
    return result;
}

inline v4
operator-(v4 left,v4 right)
{
    v4 result = {left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w};
    return result;
}

inline v4
operator-(v4 self)
{
    v4 result = {-self.x, -self.y, -self.z, -self.w};
    return result;
}

inline v4
operator*(v4 self, f32 scalar)
{
    v4 result = {self.x * scalar,self.y * scalar, self.z * scalar, self.w * scalar};
    return result;
}

inline v4
operator*(f32 scalar, v4 self)
{
    v4 result = self * scalar;
    return result;
}

inline v4
operator/(v4 self, f32 scalar)
{
    v4 result = {self.x / scalar,self.y / scalar, self.z / scalar, self.w / scalar};
    return result;
}

inline
v4 operator/(f32 scalar, v4 self)
{
    v4 result = self / scalar;
    return result;
}

inline f32
lensq(v4 self)
{
    f32 result = f32_sq(self.x) + f32_sq(self.y) + f32_sq(self.z) + f32_sq(self.w);
    return result;
}

inline f32
len(v4 self)
{
    f32 result = f32_sqrt(lensq(self));
    return result;
}

inline v4
normalized(v4 self)
{
    v4 result = self / len(self);
    return result;
}

/**
 * @brief mat2
 */

union mat2
{
    struct
    {
        f32 E11, E21;
        f32 E12, E22;
    };
    struct
    {
        f32 a, c;
        f32 b, d;
    };
    struct
    {
        v2 col1;
        v2 col2;
    };
    
    f32 E[2][2];
};

inline mat2
iMat2()
{
    mat2 result =  {1,0,
        0,1};
    return result;
}

inline mat2
operator+(mat2 l, mat2 r)
{
    mat2 result;
    result.col1 = l.col1 + r.col1;
    result.col2 = l.col2 + r.col2;
    return result;
}

inline mat2
operator-(mat2 l, mat2 r)
{
    mat2 result;
    result.col1 = l.col1 - r.col1;
    result.col2 = l.col2 - r.col2;
    return result;
}

inline v2
operator*(mat2 l, v2 r)
{
    v2 result = r.x * l.col1 + r.y * l.col2;
    return result;
}

inline mat2
operator*(mat2 l, mat2 r)
{
    mat2 result;
    result.col1 = l * r.col1;
    result.col2 = l * r.col2;
    return result;
}

inline mat2
operator*(mat2 l, f32 scalar)
{
    mat2 result;
    result.col1 = l.col1 * scalar;
    result.col2 = l.col2 * scalar;
    return result;
}

inline mat2
operator*(f32 scalar, mat2 r)
{
    mat2 result = r * scalar;
    return result;
}

inline mat2
operator/(mat2 l, f32 scalar)
{
    mat2 result;
    result.col1 = l.col1 / scalar;
    result.col2 = l.col2 / scalar;
    return result;
}

inline mat2
operator/(f32 scalar, mat2 r)
{
    mat2 result;
    result.col1 = scalar / r.col1;
    result.col2 = scalar / r.col2;
    return result;
}

inline b8
operator==(mat2 l, mat2 r)
{
    b8 result = l.col1 == r.col1 &&
        l.col2 == r.col2;
    return result;
}

inline mat2
trnspos(mat2 m)
{
    mat2 result = { m.E11, m.E12,
        m.E21, m.E22};
    return result;
}

inline f32
det(mat2 m)
{
    f32 result = m.E11 * m.E22 - m.E12 * m.E21;
    return result;
}

/**
 * @param m Matrix to invert
 * @return returns inverted matrix if able otherwise returns {0, 0, 0, 0}
 */
inline mat2
invert(mat2 m)
{
    f32 determant = det(m);
    mat2 result = {};
    if(determant != 0)
    {
        result = 1/determant*
            mat2{
            m.E22, -m.E21,
            -m.E12, m.E11,
        };
    }
    return result;
}


/**
 * @brief mat3
 */

union mat3
{
    struct
    {
        f32 E11, E21, E31;
        f32 E12, E22, E32;
        f32 E13, E23, E33;
    };
    struct
    {
        f32 a, d, g;
        f32 b, e, h;
        f32 c, f, i;
    };
    struct
    {
        v3 col1;
        v3 col2;
        v3 col3;
    };
    
    f32 E[3][3];
};


inline mat3
iMat3()
{
    mat3 result = { 1,0,0,
        0,1,0,
        0,0,1};
    return result;
}

inline mat3
operator+(mat3 l, mat3 r)
{
    mat3 result;
    result.col1 = l.col1 + r.col1;
    result.col2 = l.col2 + r.col2;
    result.col3 = l.col3 + r.col3;
    return result;
}

inline mat3
operator-(mat3 l, mat3 r)
{
    mat3 result;
    result.col1 = l.col1 - r.col1;
    result.col2 = l.col2 - r.col2;
    result.col3 = l.col3 - r.col3;
    return result;
}

inline v3
operator*(mat3 l, v3 r)
{
    v3 result = r.x * l.col1 + r.y * l.col2 + r.z * l.col3;
    return result;
}

inline mat3
operator*(mat3 l, mat3 r)
{
    mat3 result;
    result.col1 = l * r.col1;
    result.col2 = l * r.col2;
    result.col3 = l * r.col3;
    return result;
}

inline mat3
operator*(mat3 l, f32 scalar)
{
    mat3 result;
    result.col1 = l.col1 * scalar;
    result.col2 = l.col2 * scalar;
    result.col3 = l.col3 * scalar;
    return result;
}

inline mat3
operator*(f32 scalar, mat3 r)
{
    mat3 result = r * scalar;
    return result;
}


inline mat3
operator/(mat3 l, f32 scalar)
{
    mat3 result;
    result.col1 = l.col1 / scalar;
    result.col2 = l.col2 / scalar;
    result.col3 = l.col3 / scalar;
    return result;
}

inline mat3
operator/(f32 scalar, mat3 r)
{
    mat3 result;
    result.col1 = scalar / r.col1;
    result.col2 = scalar / r.col2;
    result.col3 = scalar / r.col3;
    return result;
}

inline mat3
trnspos(mat3 m)
{
    mat3 result = { m.E11, m.E12, m.E13,
        m.E21, m.E22, m.E23,
        m.E31, m.E32, m.E33};
    return result;
}

inline f32
det(mat3 m)
{
    f32 result =    m.E11 * m.E22 * m.E33 +
        m.E12 * m.E23 * m.E31 +
        m.E13 * m.E21 * m.E32 -
        m.E31 * m.E22 * m.E13 -
        m.E32 * m.E23 * m.E11 -
        m.E33 * m.E21 * m.E12;
    
    return result;
}

inline mat3
invert(mat3 m)
{
    mat3 result;
    f32 scalar = 1/det(m);
    
    f32 A = (m.e*m.i - m.f*m.h);
    f32 B = -(m.d*m.i - m.f*m.g);
    f32 C = (m.d*m.h - m.e*m.g);
    f32 D = -(m.b*m.i - m.c*m.h);
    f32 E = (m.a*m.i - m.c*m.g);
    f32 F = -(m.a*m.h - m.b*m.g);
    f32 G = (m.b*m.f - m.c*m.e);
    f32 H = -(m.a*m.f - m.c*m.d);
    f32 I = (m.a*m.e - m.b*m.d);
    
    result = scalar * mat3{
        A,B,C,
        D,E,F,
        G,H,I
    };
    
    return result;
}


/**
 * @brief mat4
 */

union mat4
{
    struct
    {
        f32 E11, E21, E31, E41;
        f32 E12, E22, E32, E42;
        f32 E13, E23, E33, E43;
        f32 E14, E24, E34, E44;
    };
    struct
    {
        f32 a, e, i, m;
        f32 b, f, j, n;
        f32 c, g, k, o;
        f32 d, h, l, p;
    };
    struct
    {
        v4 col1;
        v4 col2;
        v4 col3;
        v4 col4;
    };
    
    f32 E[4][4];
};

inline mat4
iMat4()
{
    mat4 result = { 1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1};
    return result;
}


inline mat4
perspectiveMat(f32 fov,f32 near,f32 far,f32 aspect)
{
    IR_ASSERT(near < far);
    IR_ASSERT(aspect > 0);
    
    f32 diffZ = near - far;
    f32 tanHalfFov = f32_tan(fov / 2.0f);
    
    
    f32 a = 1.0f/(aspect * tanHalfFov);
    f32 b = 1.0f/tanHalfFov;
    f32 c = (-near-far)/diffZ;
    f32 d = (2 * far * near)/diffZ;
    
    mat4 result={a,0,0,0,
        0,b,0,0,
        0,0,c,1,
        0,0,d,0};
    
    return result;
}

inline mat4
transMat(v3 dir)
{
    mat4 result=iMat4();
    
    result.col4.xyz = dir;
    
    return result;
};

inline mat4
rotMat(v3 axis, f32 angle)
{
    mat4 result=iMat4();
    
    f32 cosA = f32_cos(angle);
    f32 sinA = f32_sin(angle);
    f32 omcosA = 1 - cosA; // One Minus COS Angle
    
    result.E11 = cosA + f32_sq(axis.x)*omcosA;
    result.E12 = axis.x * axis.y * omcosA - axis.z * sinA;
    result.E13 = axis.x * axis.z * omcosA + axis.y * sinA;
    result.E21 = axis.y * axis.x * omcosA + axis.z * sinA;
    result.E22 = cosA + f32_sq(axis.y) * omcosA;
    result.E23 = axis.y * axis.z * omcosA - axis.x * sinA;
    result.E31 = axis.z * axis.x * omcosA - axis.y * sinA;
    result.E32 = axis.z * axis.y * omcosA + axis.x * sinA;
    result.E33 = cosA + f32_sq(axis.z) * omcosA;
    
    return result;
};

inline mat4
scaleMat(v3 scale)
{
    mat4 result = iMat4();
    result.col1.x = scale.x;
    result.col2.y = scale.y;
    result.col3.z = scale.z;
    return result;
}


inline mat4
operator+(mat4 l, mat4 r)
{
    mat4 result;
    result.col1 = l.col1 + r.col1;
    result.col2 = l.col2 + r.col2;
    result.col3 = l.col3 + r.col3;
    result.col4 = l.col4 + r.col4;
    return result;
}

inline mat4
operator-(mat4 l, mat4 r)
{
    mat4 result;
    result.col1 = l.col1 - r.col1;
    result.col2 = l.col2 - r.col2;
    result.col3 = l.col3 - r.col3;
    result.col4 = l.col4 - r.col4;
    return result;
}

inline v4
operator*(mat4 l, v4 r)
{
    v4 result = l.col1 * r.x + l.col2 * r.y + l.col3 * r.z + l.col4 * r.w;
    return result;
}

inline mat4
operator*(mat4 l, mat4 r)
{
    mat4 result;
    result.col1 = l * r.col1;
    result.col2 = l * r.col2;
    result.col3 = l * r.col3;
    result.col4 = l * r.col4;
    return result;
}

inline mat4
operator*(mat4 l, f32 scalar)
{
    mat4 result;
    result.col1 = l.col1 * scalar;
    result.col2 = l.col2 * scalar;
    result.col3 = l.col3 * scalar;
    result.col4 = l.col4 * scalar;
    return result;
}

inline mat4
operator*(f32 scalar, mat4 r)
{
    mat4 result = r * scalar;
    return result;
}


inline mat4
operator/(mat4 l, f32 scalar)
{
    mat4 result;
    result.col1 = l.col1 / scalar;
    result.col2 = l.col2 / scalar;
    result.col3 = l.col3 / scalar;
    result.col4 = l.col4 / scalar;
    return result;
}

inline mat4
operator/(f32 scalar, mat4 r)
{
    mat4 result;
    result.col1 =  scalar / r.col1;
    result.col2 =  scalar / r.col2;
    result.col3 =  scalar / r.col3;
    result.col4 =  scalar / r.col4;
    return result;
}

inline mat4
trnspos(mat4 self)
{
    mat4 result = { self.E11, self.E12, self.E13, self.E14,
        self.E21, self.E22, self.E23, self.E24,
        self.E31, self.E32, self.E33, self.E34,
        self.E41, self.E42, self.E43, self.E44};
    return result;
}

inline mat4
transform(v3 pos, v3 axis, f32 angle, v3 scale)
{
    mat4 result = scaleMat(scale);
    result = rotMat(axis, angle) * result;
    result.col4.xyz = pos;
    return result;
}

inline mat4
lookAt(v3 pos, v3 target, v3 up)
{
    v3 dir = normalized(target - pos);
    v3 camRight = normalized(cross(up, dir));
    v3 camUp = cross(dir, camRight);
    
    mat4 tmp = iMat4();
    
    tmp.col1 = v4{camRight.x , camUp.x, dir.x, 0.0f};
    tmp.col2 = v4{camRight.y , camUp.y, dir.y, 0.0f};
    tmp.col3 = v4{camRight.z , camUp.z, dir.z, 0.0f};
    
    
    return tmp * transMat(-pos);
}

inline mat3
toMat3(mat4 m)
{
    mat3 result;
    result.col1 = m.col1.xyz;
    result.col2 = m.col2.xyz;
    result.col3 = m.col3.xyz;
    
    return result;
}

inline mat4
toMat4(mat3 m)
{
    mat4 result = {};
    result.col1.xyz = m.col1;
    result.col2.xyz = m.col2;
    result.col3.xyz = m.col3;
    
    return result;
}




#endif
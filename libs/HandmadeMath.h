/*
  HandmadeMath.h v1.7.1
  
  This is a single header file with a bunch of useful functions for game and
  graphics math operations.
  
  =============================================================================
  
  You MUST
  
     #define HANDMADE_MATH_IMPLEMENTATION
     
  in EXACTLY one C or C++ file that includes this header, BEFORE the
  include, like this:
  
     #define HANDMADE_MATH_IMPLEMENTATION
     #include "HandmadeMath.h"
     
  All other files should just #include "HandmadeMath.h" without the #define.
  
  =============================================================================
  
  To disable SSE intrinsics, you MUST
  
  #define HANDMADE_MATH_NO_SSE
  
  in EXACTLY one C or C++ file that includes this header, BEFORE the
  include, like this:
  
     #define HANDMADE_MATH_IMPLEMENTATION
     #define HANDMADE_MATH_NO_SSE
     #include "HandmadeMath.h"
	 
  =============================================================================
  
  To use HandmadeMath without the CRT, you MUST 
  
     #define HMM_SINF MySinF
     #define HMM_COSF MyCosF
     #define HMM_TANF MyTanF
     #define HMM_SQRTF MySqrtF
     #define HMM_EXPF MyExpF
     #define HMM_LOGF MyLogF
     #define HMM_ACOSF MyACosF
     #define HMM_ATANF MyATanF
     #define HMM_ATAN2F MYATan2F
     
  Provide your own implementations of SinF, CosF, TanF, ACosF, ATanF, ATan2F, 
  ExpF, and LogF in EXACTLY one C or C++ file that includes this header,
  BEFORE the include, like this:     
  
     #define HMM_SINF MySinF
     #define HMM_COSF MyCosF
     #define HMM_TANF MyTanF
     #define HMM_SQRTF MySqrtF
     #define HMM_EXPF MyExpF
     #define HMM_LOGF MyLogF
     #define HMM_ACOSF MyACosF
     #define HMM_ATANF MyATanF
     #define HMM_ATAN2F MyATan2F
     #define HANDMADE_MATH_IMPLEMENTATION
     #include "HandmadeMath.h"
     
  If you do not define all of these, HandmadeMath.h will use the
  versions of these functions that are provided by the CRT.
  
  =============================================================================
  
  Version History:
      0.2 (*) Updated documentation
          (*) Better C compliance
          (*) Prefix all handmade math functions 
          (*) Better operator overloading
      0.2a
          (*) Prefixed Macros
      0.2b
          (*) Disabled warning 4201 on MSVC as it is legal is C11
          (*) Removed the f at the end of HMM_PI to get 64bit precision
      0.3
          (*) Added +=, -=, *=, /= for hmm_vec2, hmm_vec3, v4
      0.4
          (*) SSE Optimized HMM_SqrtF
          (*) SSE Optimized HMM_RSqrtF
          (*) Removed CRT
      0.5
          (*) Added scalar multiplication and division for vectors
              and matrices
          (*) Added matrix subtraction and += for hmm_mat4
          (*) Reconciled all headers and implementations
          (*) Tidied up, and filled in a few missing operators
      0.5.1
          (*) Ensured column-major order for matrices throughout
          (*) Fixed HMM_Translate producing row-major matrices
      0.5.2
          (*) Fixed SSE code in HMM_SqrtF
          (*) Fixed SSE code in HMM_RSqrtF
      0.6
          (*) Added Unit testing
          (*) Made HMM_Power faster
          (*) Fixed possible efficiency problem with HMM_Normalize 
          (*) RENAMED HMM_LengthSquareRoot to HMM_LengthSquared
          (*) RENAMED HMM_RSqrtF to HMM_RSquareRootF
          (*) RENAMED HMM_SqrtF to HMM_SquareRootF
          (*) REMOVED Inner function (user should use Dot now)
          (*) REMOVED HMM_FastInverseSquareRoot function declaration
      0.7 
          (*) REMOVED HMM_LengthSquared in HANDMADE_MATH_IMPLEMENTATION (should
              use len_squared_v3, or HANDMADE_MATH_CPP_MODE for function
              overloaded version)
          (*) REMOVED HMM_Length in HANDMADE_MATH_IMPLEMENTATION (should use
              HMM_LengthVec3, HANDMADE_MATH_CPP_MODE for function
              overloaded version)
          (*) REMOVED HMM_Normalize in HANDMADE_MATH_IMPLEMENTATION (should use
              normalize_v3, or HANDMADE_MATH_CPP_MODE for function
              overloaded version)
          (*) Added len_squared_v2
          (*) Added len_squared_v4
          (*) Addd HMM_LengthVec2
          (*) Added HMM_LengthVec4
          (*) Added normalize_v2
          (*) Added normalize_v4
     1.0
          (*) Lots of testing!
     1.1
          (*) Quaternion support
          (*) Added type Quat
          (*) Added quaternion
          (*) Added quaternionV4
          (*) Added HMM_AddQuaternion
          (*) Added HMM_SubtractQuaternion
          (*) Added mul_quat
          (*) Added mul_quatF
          (*) Added HMM_DivideQuaternionF
          (*) Added inverse_quat
          (*) Added dot_quat
          (*) Added normalize_quat
          (*) Added HMM_Slerp
          (*) Added quaternionToMat4
          (*) Added quaternionFromAxisAngle
     1.1.1
          (*) Resolved compiler warnings on gcc and g++
     1.1.2
          (*) Fixed invalid HMMDEF's in the function definitions
     1.1.3
          (*) Fixed compile error in C mode
     1.1.4
          (*) Fixed SSE being included on platforms that don't support it
          (*) Fixed divide-by-zero errors when normalizing zero vectors.
     1.1.5
          (*) Add Width and Height to vec2
          (*) Made it so you can supply your own SqrtF 
     1.2.0
          (*) Added equality functions for vec2, vec3, and vec4.
              (*) Added HMM_EqualsVec2, HMM_EqualsVec3, and HMM_EqualsVec4
              (*) Added C++ overloaded HMM_Equals for all three
              (*) Added C++ == and != operators for all three
          (*) SSE'd HMM_MultiplyMat4 (this is _WAY_ faster)
          (*) SSE'd HMM_Transpose
     1.3.0
          (*) Remove need to #define HANDMADE_MATH_CPP_MODE
     1.4.0
          (*) Fixed bug when using HandmadeMath in C mode
          (*) SSEd all vec4 operations          
          (*) Removed all zero-ing
     1.5.0
          (*) Changed internal structure for better performance and inlining.
          (*) As a result, HANDMADE_MATH_NO_INLINE has been removed and no
              longer has any effect.
     1.5.1
          (*) Fixed a bug with uninitialized elements in HMM_LookAt.
     1.6.0
          (*) Added array subscript operators for vector and matrix types in
              C++. This is provided as a convenience, but be aware that it may
              incur an extra function call in unoptimized builds.
     1.7.0
          (*) Renamed the 'Rows' member of hmm_mat4 to 'Columns'. Since our
              matrices are column-major, this should have been named 'Columns'
              from the start. 'Rows' is still present, but has been deprecated.
     1.7.1
          (*) Changed operator[] to take in a const ref int instead of a int. 
              Simple dumb mistake. NOTE: The compiler still wont inline operator[]
              for some reason 
			  
  LICENSE
  
  This software is in the public domain. Where that dedication is not
  recognized, you are granted a perpetual, irrevocable license to copy,
  distribute, and modify this file as you see fit.
  
  CREDITS
  
  Written by Zakary Strange (zak@strangedev.net && @strangezak)
  
  Functionality:
   Matt Mascarenhas (@miblo_)
   Aleph
   FieryDrake (@fierydrake)
   Gingerbill (@TheGingerBill)
   Ben Visness (@bvisness) 
   Trinton Bullard (@Peliex_Dev)
   
  Fixes:
   Jeroen van Rijn (@J_vanRijn)
   Kiljacken (@Kiljacken)
   Insofaras (@insofaras)
   Daniel Gibson (@DanielGibson)
*/


/* let's figure out if SSE is really available (unless disabled anyway)
   (it isn't on non-x86/x86_64 platforms or even x86 without explicit SSE support)
   => only use "#ifdef HANDMADE_MATH__USE_SSE" to check for SSE support below this block! */
#ifndef HANDMADE_MATH_NO_SSE

# ifdef _MSC_VER
/* MSVC supports SSE in amd64 mode or _M_IX86_FP >= 1 (2 means SSE2) */
#  if defined(_M_AMD64) || ( defined(_M_IX86_FP) && _M_IX86_FP >= 1 )
#   define HANDMADE_MATH__USE_SSE 1
#  endif
# else /* not MSVC, probably GCC, clang, icc or something that doesn't support SSE anyway */
#  ifdef __SSE__ /* they #define __SSE__ if it's supported */
#   define HANDMADE_MATH__USE_SSE 1
#  endif /*  __SSE__ */
# endif /* not _MSC_VER */

#endif /* #ifndef HANDMADE_MATH_NO_SSE */

#include <stdint.h> // This is for types

#ifdef HANDMADE_MATH__USE_SSE
#include <xmmintrin.h>
#endif

#ifndef HANDMADE_MATH_H
#define HANDMADE_MATH_H

#ifdef _MSC_VER
#pragma warning(disable:4201)
#endif

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#endif

#ifdef __cplusplus
extern "C"
{
#endif
	
#define HMM_INLINE static inline
#define HMM_EXTERN extern
	
#if !defined(HMM_SINF) || !defined(HMM_COSF) || !defined(HMM_TANF) || \
    !defined(HMM_SQRTF) || !defined(HMM_EXPF) || !defined(HMM_LOGF) || \
    !defined(HMM_ACOSF) || !defined(HMM_ATANF)|| !defined(HMM_ATAN2F)
#include <math.h>    
#endif
    
#ifndef HMM_SINF
#define HMM_SINF sinf
#endif    
	
#ifndef HMM_COSF
#define HMM_COSF cosf
#endif    
	
#ifndef HMM_TANF
#define HMM_TANF tanf
#endif        
	
#ifndef HMM_SQRTF
#define HMM_SQRTF sqrtf
#endif    
    
#ifndef HMM_EXPF
#define HMM_EXPF expf
#endif
	
#ifndef HMM_LOGF
#define HMM_LOGF logf
#endif
	
#ifndef HMM_ACOSF
#define HMM_ACOSF acosf
#endif
	
#ifndef HMM_ATANF
#define HMM_ATANF atanf
#endif
	
#ifndef HMM_ATAN2F
#define HMM_ATAN2F atan2f
#endif
	
#define HMM_PI32 3.14159265359f
#define HMM_PI 3.14159265358979323846
	
#define HMM_MIN(a, b) (a) > (b) ? (b) : (a)
#define HMM_MAX(a, b) (a) < (b) ? (b) : (a)
#define HMM_ABS(a) ((a) > 0 ? (a) : -(a))
#define HMM_MOD(a, m) ((a) % (m)) >= 0 ? ((a) % (m)) : (((a) % (m)) + (m))
#define HMM_SQUARE(x) ((x) * (x))
	
	typedef union v2
	{
		struct
		{
			float X, Y;
		};
		
		struct
		{
			float x, y;
		};
		
		struct
		{
			float U, V;
		};
		
		struct
		{
			float Left, Right;
		};
		
		struct
		{
			float Width, Height;
		};
		
		float Elements[2];
		
#ifdef __cplusplus
		inline float &operator[](const int &Index)
		{
			return Elements[Index];
		}
#endif
	} v2;
	
	typedef union v3
	{
		struct
		{
			float X, Y, Z;
		};
		
		struct
		{
			float x, y, z;
		};
		
		struct
		{
			float U, V, W;
		};
		
		struct
		{
			float R, G, B;
		};
		
		struct
		{
			v2 XY;
			float Ignored0_;
		};
		
		struct
		{
			float Ignored1_;
			v2 YZ;
		};
		
		struct
		{
			v2 UV;
			float Ignored2_;
		};
		
		struct
		{
			float Ignored3_;
			v2 VW;
		};
		
		float Elements[3];
		
#ifdef __cplusplus
		inline float &operator[](const int &Index)
		{
			return Elements[Index];
		}
#endif
	} v3;
	
	typedef union v4
	{
		struct
		{
			union
			{
				v3 XYZ;
				struct
				{
					float X, Y, Z;
				};
				struct
				{
					float x, y, z;
				};
			};
			
			float W;
		};
		struct
		{
			union
			{
				v3 RGB;
				struct
				{
					float R, G, B;
				};
			};
			
			float A;
		};
		
		struct
		{
			v2 XY;
			float Ignored0_;
			float Ignored1_;
		};
		
		struct
		{
			float Ignored2_;
			v2 YZ;
			float Ignored3_;
		};
		
		struct
		{
			float Ignored4_;
			float Ignored5_;
			v2 ZW;
		};
		
		float Elements[4];
		
#ifdef HANDMADE_MATH__USE_SSE    
		__m128 InternalElementsSSE;
#endif
		
#ifdef __cplusplus
		inline float &operator[](const int &Index)
		{
			return Elements[Index];
		}
#endif
	} v4;
	
	typedef union mat4
	{
		float Elements[4][4];
        
        struct {
            v4 X;
            v4 Y;
            v4 Z;
            v4 W;
        };
		
#ifdef HANDMADE_MATH__USE_SSE
		__m128 Columns[4];
		
		// DEPRECATED. Our matrices are column-major, so this was named
		// incorrectly. Use Columns instead.
		__m128 Rows[4];
#endif
		
#ifdef __cplusplus
		inline v4 operator[](const int &Index)
		{
			float* col = Elements[Index];
			
			v4 result;
			result.Elements[0] = col[0];
			result.Elements[1] = col[1];
			result.Elements[2] = col[2];
			result.Elements[3] = col[3];
			
			return result;
		}
#endif
	} mat4;
	
	typedef union Quat
	{
		struct
		{
			union
			{
				v3 XYZ;
				struct
				{
					float X, Y, Z;
				};
			};
			
			float W;
		};
		
		float Elements[4];
	} Quat;
	
	typedef int32_t hmm_bool;
	
	typedef v2 hmm_v2;
	typedef v3 hmm_v3;
	typedef v4 hmm_v4;
	typedef mat4 hmm_m4;    
	
	
	/*
  * Floating-point math functions
  */
	
	HMM_INLINE float HMM_SinF(float Radians)
	{
		float Result = HMM_SINF(Radians);
		
		return (Result);
	}
	
	HMM_INLINE float HMM_CosF(float Radians)
	{
		float Result = HMM_COSF(Radians);
		
		return (Result);
	}
	
	HMM_INLINE float HMM_TanF(float Radians)
	{
		float Result = HMM_TANF(Radians);
		
		return (Result);
	}
	
	HMM_INLINE float HMM_ACosF(float Radians)
	{
		float Result = HMM_ACOSF(Radians);
		
		return (Result);
	}
	
	HMM_INLINE float HMM_ATanF(float Radians)
	{
		float Result = HMM_ATANF(Radians);
		
		return (Result);
	}
	
	HMM_INLINE float HMM_ATan2F(float Left, float Right)
	{
		float Result = HMM_ATAN2F(Left, Right);
		
		return (Result);
	}
	
	HMM_INLINE float HMM_ExpF(float Float)
	{
		float Result = HMM_EXPF(Float);
		
		return (Result);
	}
	
	HMM_INLINE float HMM_LogF(float Float)
	{
		float Result = HMM_LOGF(Float);
		
		return (Result);
	}
	
	HMM_INLINE float HMM_SquareRootF(float Float)
	{
		float Result;
		
#ifdef HANDMADE_MATH__USE_SSE
		__m128 In = _mm_set_ss(Float);
		__m128 Out = _mm_sqrt_ss(In);
		Result = _mm_cvtss_f32(Out);
#else
		Result = HMM_SQRTF(Float);
#endif 
		
		return(Result);
	}
	
	HMM_INLINE float HMM_RSquareRootF(float Float)
	{
		float Result;
		
#ifdef HANDMADE_MATH__USE_SSE
		__m128 In = _mm_set_ss(Float);
		__m128 Out = _mm_rsqrt_ss(In);
		Result = _mm_cvtss_f32(Out);
#else
		Result = 1.0f/HMM_SquareRootF(Float);
#endif
		
		return(Result);
	}
	
	HMM_EXTERN float HMM_Power(float Base, int Exponent);
	
	HMM_INLINE float HMM_PowerF(float Base, float Exponent)
	{
		float Result = HMM_EXPF(Exponent * HMM_LOGF(Base));
		
		return (Result);
	}
	
	
	/*
  * Utility functions
  */
	HMM_INLINE float HMM_ToRadians(float Degrees)
	{
		float Result = Degrees * (HMM_PI32 / 180.0f);
		
		return (Result);
	}
	
	HMM_INLINE float lerp(float A, float Time, float B)
	{
		float Result = (1.0f - Time) * A + Time * B;
		
		return (Result);
	}
	
	HMM_INLINE float HMM_Clamp(float Min, float Value, float Max)
	{
		float Result = Value;
		
		if(Result < Min)
		{
			Result = Min;
		}
		else if(Result > Max)
		{
			Result = Max;
		}
		
		return (Result);
	}
	
	
	/*
  * Vector initialization
  */
	
	HMM_INLINE v2 vec2(float X, float Y)
	{
		v2 Result;
		
		Result.X = X;
		Result.Y = Y;
		
		return (Result);
	}
	
	HMM_INLINE v2 vec2i(int X, int Y)
	{
		v2 Result;
		
		Result.X = (float)X;
		Result.Y = (float)Y;
		
		return (Result);
	}
	
	HMM_INLINE v3 vec3(float X, float Y, float Z)
	{
		v3 Result;
		
		Result.X = X;
		Result.Y = Y;
		Result.Z = Z;
		
		return (Result);
	}
	
	HMM_INLINE v3 vec3i(int X, int Y, int Z)
	{
		v3 Result;
		
		Result.X = (float)X;
		Result.Y = (float)Y;
		Result.Z = (float)Z;
		
		return (Result);
	}
	
	HMM_INLINE v4 vec4(float X, float Y, float Z, float W)
	{
		v4 Result;
		
#ifdef HANDMADE_MATH__USE_SSE
		Result.InternalElementsSSE = _mm_setr_ps(X, Y, Z, W);
#else
		Result.X = X;
		Result.Y = Y;
		Result.Z = Z;
		Result.W = W;
#endif
		
		return (Result);
	}
	
	HMM_INLINE v4 vec4i(int X, int Y, int Z, int W)
	{
		v4 Result;
		
#ifdef HANDMADE_MATH__USE_SSE
		Result.InternalElementsSSE = _mm_setr_ps((float)X, (float)Y, (float)Z, (float)W);
#else
		Result.X = (float)X;
		Result.Y = (float)Y;
		Result.Z = (float)Z;
		Result.W = (float)W;
#endif
		
		return (Result);
	}
	
	HMM_INLINE v4 vec4v(v3 Vector, float W)
	{
		v4 Result;
		
#ifdef HANDMADE_MATH__USE_SSE
		Result.InternalElementsSSE = _mm_setr_ps(Vector.X, Vector.Y, Vector.Z, W);
#else
		Result.XYZ = Vector;
		Result.W = W;
#endif
		
		return (Result);
	}
	
	
	/*
  * Binary vector operations
  */
	
	HMM_INLINE v2 add_v2(v2 Left, v2 Right)
	{
		v2 Result;
		
		Result.X = Left.X + Right.X;
		Result.Y = Left.Y + Right.Y;
		
		return (Result);
	}
	
	HMM_INLINE v3 add_v3(v3 Left, v3 Right)
	{
		v3 Result;
		
		Result.X = Left.X + Right.X;
		Result.Y = Left.Y + Right.Y;
		Result.Z = Left.Z + Right.Z;
		
		return (Result);
	}
	
	HMM_INLINE v4 add_v4(v4 Left, v4 Right)
	{
		v4 Result;
		
#ifdef HANDMADE_MATH__USE_SSE
		Result.InternalElementsSSE = _mm_add_ps(Left.InternalElementsSSE, Right.InternalElementsSSE);
#else    
		Result.X = Left.X + Right.X;
		Result.Y = Left.Y + Right.Y;
		Result.Z = Left.Z + Right.Z;
		Result.W = Left.W + Right.W;    
#endif
		
		return (Result);
	}
	
	HMM_INLINE v2 sub_v2(v2 Left, v2 Right)
	{
		v2 Result;
		
		Result.X = Left.X - Right.X;
		Result.Y = Left.Y - Right.Y;
		
		return (Result);
	}
	
	HMM_INLINE v3 sub_v3(v3 Left, v3 Right)
	{
		v3 Result;
		
		Result.X = Left.X - Right.X;
		Result.Y = Left.Y - Right.Y;
		Result.Z = Left.Z - Right.Z;
		
		return (Result);
	}
	
	HMM_INLINE v4 sub_v4(v4 Left, v4 Right)
	{
		v4 Result;
		
#ifdef HANDMADE_MATH__USE_SSE
		Result.InternalElementsSSE = _mm_sub_ps(Left.InternalElementsSSE, Right.InternalElementsSSE);
#else    
		Result.X = Left.X - Right.X;
		Result.Y = Left.Y - Right.Y;
		Result.Z = Left.Z - Right.Z;
		Result.W = Left.W - Right.W;    
#endif
		
		return (Result);
	}
	
	HMM_INLINE v2 mul_v2(v2 Left, v2 Right)
	{
		v2 Result;
		
		Result.X = Left.X * Right.X;
		Result.Y = Left.Y * Right.Y;
		
		return (Result);
	}
	
	HMM_INLINE v2 mul_v2f(v2 Left, float Right)
	{
		v2 Result;
		
		Result.X = Left.X * Right;
		Result.Y = Left.Y * Right;
		
		return (Result);
	}
	
	HMM_INLINE v3 mul_v3(v3 Left, v3 Right)
	{
		v3 Result;
		
		Result.X = Left.X * Right.X;
		Result.Y = Left.Y * Right.Y;
		Result.Z = Left.Z * Right.Z;
		
		return (Result);
	}
	
	HMM_INLINE v3 mul_v3f(v3 Left, float Right)
	{
		v3 Result;
		
		Result.X = Left.X * Right;
		Result.Y = Left.Y * Right;
		Result.Z = Left.Z * Right;
		
		return (Result);
	}
	
	HMM_INLINE v4 mul_v4(v4 Left, v4 Right)
	{
		v4 Result;
		
#ifdef HANDMADE_MATH__USE_SSE
		Result.InternalElementsSSE = _mm_mul_ps(Left.InternalElementsSSE, Right.InternalElementsSSE);
#else
		Result.X = Left.X * Right.X;
		Result.Y = Left.Y * Right.Y;
		Result.Z = Left.Z * Right.Z;
		Result.W = Left.W * Right.W;    
#endif
		
		return (Result);
	}
	
	HMM_INLINE v4 mul_v4f(v4 Left, float Right)
	{
		v4 Result;
		
#ifdef HANDMADE_MATH__USE_SSE
		__m128 Scalar = _mm_set1_ps(Right);
		Result.InternalElementsSSE = _mm_mul_ps(Left.InternalElementsSSE, Scalar);
#else    
		Result.X = Left.X * Right;
		Result.Y = Left.Y * Right;
		Result.Z = Left.Z * Right;
		Result.W = Left.W * Right;
#endif
		
		return (Result);
	}
	
	HMM_INLINE v2 div_v2(v2 Left, v2 Right)
	{
		v2 Result;
		
		Result.X = Left.X / Right.X;
		Result.Y = Left.Y / Right.Y;
		
		return (Result);
	}
	
	HMM_INLINE v2 div_v2f(v2 Left, float Right)
	{
		v2 Result;
		
		Result.X = Left.X / Right;
		Result.Y = Left.Y / Right;
		
		return (Result);
	}
	
	HMM_INLINE v3 div_v3(v3 Left, v3 Right)
	{
		v3 Result;
		
		Result.X = Left.X / Right.X;
		Result.Y = Left.Y / Right.Y;
		Result.Z = Left.Z / Right.Z;
		
		return (Result);
	}
	
	HMM_INLINE v3 div_v3f(v3 Left, float Right)
	{
		v3 Result;
		
		Result.X = Left.X / Right;
		Result.Y = Left.Y / Right;
		Result.Z = Left.Z / Right;
		
		return (Result);
	}
	
	HMM_INLINE v4 div_v4(v4 Left, v4 Right)
	{
		v4 Result;
		
#ifdef HANDMADE_MATH__USE_SSE
		Result.InternalElementsSSE = _mm_div_ps(Left.InternalElementsSSE, Right.InternalElementsSSE);
#else
		Result.X = Left.X / Right.X;
		Result.Y = Left.Y / Right.Y;
		Result.Z = Left.Z / Right.Z;
		Result.W = Left.W / Right.W;
#endif
		
		return (Result);
	}
	
	HMM_INLINE v4 div_v4f(v4 Left, float Right)
	{
		v4 Result;
		
#ifdef HANDMADE_MATH__USE_SSE
		__m128 Scalar = _mm_set1_ps(Right);
		Result.InternalElementsSSE = _mm_div_ps(Left.InternalElementsSSE, Scalar);
#else    
		Result.X = Left.X / Right;
		Result.Y = Left.Y / Right;
		Result.Z = Left.Z / Right;
		Result.W = Left.W / Right;
#endif
		
		return (Result);
	}
	
	HMM_INLINE hmm_bool HMM_EqualsVec2(v2 Left, v2 Right)
	{
		hmm_bool Result = (Left.X == Right.X && Left.Y == Right.Y);
		
		return (Result);
	}
	
	HMM_INLINE hmm_bool HMM_EqualsVec3(v3 Left, v3 Right)
	{
		hmm_bool Result = (Left.X == Right.X && Left.Y == Right.Y && Left.Z == Right.Z);
		
		return (Result);
	}
	
	HMM_INLINE hmm_bool HMM_EqualsVec4(v4 Left, v4 Right)
	{
		hmm_bool Result = (Left.X == Right.X && Left.Y == Right.Y && Left.Z == Right.Z && Left.W == Right.W);
		
		return (Result);
	}
	
	HMM_INLINE float dot_v2(v2 VecOne, v2 VecTwo)
	{
		float Result = (VecOne.X * VecTwo.X) + (VecOne.Y * VecTwo.Y);
		
		return (Result);
	}
	
	HMM_INLINE float dot_v3(v3 VecOne, v3 VecTwo)
	{
		float Result = (VecOne.X * VecTwo.X) + (VecOne.Y * VecTwo.Y) + (VecOne.Z * VecTwo.Z);
		
		return (Result);
	}
	
	HMM_INLINE float dot_v4(v4 VecOne, v4 VecTwo)
	{
		float Result;
		
		// NOTE(zak): IN the future if we wanna check what version SSE is support 
		// we can use _mm_dp_ps (4.3) but for now we will use the old way. 
		// Or a r = _mm_mul_ps(v1, v2), r = _mm_hadd_ps(r, r), r = _mm_hadd_ps(r, r) for SSE3
#ifdef HANDMADE_MATH__USE_SSE
		__m128 SSEResultOne = _mm_mul_ps(VecOne.InternalElementsSSE, VecTwo.InternalElementsSSE);
		__m128 SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(2, 3, 0, 1));
		SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
		SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(0, 1, 2, 3));
		SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);       
		_mm_store_ss(&Result, SSEResultOne);
#else
		Result = (VecOne.X * VecTwo.X) + (VecOne.Y * VecTwo.Y) + (VecOne.Z * VecTwo.Z) + (VecOne.W * VecTwo.W);
#endif
		
		return (Result);
	}
	
	HMM_INLINE v3 cross(v3 VecOne, v3 VecTwo)
	{
		v3 Result;
		
		Result.X = (VecOne.Y * VecTwo.Z) - (VecOne.Z * VecTwo.Y);
		Result.Y = (VecOne.Z * VecTwo.X) - (VecOne.X * VecTwo.Z);
		Result.Z = (VecOne.X * VecTwo.Y) - (VecOne.Y * VecTwo.X);
		
		return (Result);
	}
	
	
	/*
  * Unary vector operations
  */
	
	HMM_INLINE float len_squared_v2(v2 A)
	{
		float Result = dot_v2(A, A);
		
		return(Result);
	}
	
	HMM_INLINE float len_squared_v3(v3 A)
	{
		float Result = dot_v3(A, A);
		
		return (Result);
	}
	
	HMM_INLINE float len_squared_v4(v4 A)
	{
		float Result = dot_v4(A, A);
		
		return (Result);
	}
	
	HMM_INLINE float HMM_LengthVec2(v2 A)
	{
		float Result = HMM_SquareRootF(len_squared_v2(A));
		
		return (Result);
	}
	
	HMM_INLINE float HMM_LengthVec3(v3 A)
	{
		float Result = HMM_SquareRootF(len_squared_v3(A));
		
		return (Result);
	}
	
	HMM_INLINE float HMM_LengthVec4(v4 A)
	{
		float Result = HMM_SquareRootF(len_squared_v4(A));
		
		return(Result);
	}
	
	HMM_INLINE v2 normalize_v2(v2 A)
	{
		v2 Result = {0};
		
		float VectorLength = HMM_LengthVec2(A);
		
		/* NOTE(kiljacken): We need a zero check to not divide-by-zero */
		if (VectorLength != 0.0f)
		{
			Result.X = A.X * (1.0f / VectorLength);
			Result.Y = A.Y * (1.0f / VectorLength);
		}
		
		return (Result);
	}
	
	HMM_INLINE v3 normalize_v3(v3 A)
	{
		v3 Result = {0};
		
		float VectorLength = HMM_LengthVec3(A);
		
		/* NOTE(kiljacken): We need a zero check to not divide-by-zero */
		if (VectorLength != 0.0f)
		{
			Result.X = A.X * (1.0f / VectorLength);
			Result.Y = A.Y * (1.0f / VectorLength);
			Result.Z = A.Z * (1.0f / VectorLength);
		}
		
		return (Result);
	}
	
	HMM_INLINE v4 normalize_v4(v4 A)
	{
		v4 Result = {0};
		
		float VectorLength = HMM_LengthVec4(A);
		
		/* NOTE(kiljacken): We need a zero check to not divide-by-zero */
		if (VectorLength != 0.0f)
		{
			float Multiplier = 1.0f / VectorLength;
			
#ifdef HANDMADE_MATH__USE_SSE
			__m128 SSEMultiplier = _mm_set1_ps(Multiplier);
			Result.InternalElementsSSE = _mm_mul_ps(A.InternalElementsSSE, SSEMultiplier);        
#else 
			Result.X = A.X * Multiplier;
			Result.Y = A.Y * Multiplier;
			Result.Z = A.Z * Multiplier;
			Result.W = A.W * Multiplier;
#endif
		}
		
		return (Result);
	}
	
	
	/*
  * SSE stuff
  */
	
#ifdef HANDMADE_MATH__USE_SSE
	HMM_INLINE __m128 HMM_LinearCombineSSE(__m128 Left, mat4 Right)
	{
		__m128 Result;
		Result = _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0x00), Right.Columns[0]);
		Result = _mm_add_ps(Result, _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0x55), Right.Columns[1]));
		Result = _mm_add_ps(Result, _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0xaa), Right.Columns[2]));
		Result = _mm_add_ps(Result, _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0xff), Right.Columns[3]));
		
		return (Result);
	}
#endif
	
	
	/*
  * Matrix functions
  */
	
	HMM_INLINE mat4 HMM_Mat4(void)
	{
		mat4 Result = {0};
		
		return (Result);
	}
	
	HMM_INLINE mat4 HMM_Mat4d(float Diagonal)
	{
		mat4 Result = HMM_Mat4();
		
		Result.Elements[0][0] = Diagonal;
		Result.Elements[1][1] = Diagonal;
		Result.Elements[2][2] = Diagonal;
		Result.Elements[3][3] = Diagonal;
		
		return (Result);
	}
	
#ifdef HANDMADE_MATH__USE_SSE
	HMM_INLINE mat4 HMM_Transpose(mat4 Matrix)
	{
		mat4 Result = Matrix;
		
		_MM_TRANSPOSE4_PS(Result.Columns[0], Result.Columns[1], Result.Columns[2], Result.Columns[3]);
		
		return (Result);
	}
#else
	HMM_EXTERN mat4 HMM_Transpose(mat4 Matrix);
#endif
	
#ifdef HANDMADE_MATH__USE_SSE
	HMM_INLINE mat4 HMM_AddMat4(mat4 Left, mat4 Right)
	{
		mat4 Result;
		
		Result.Columns[0] = _mm_add_ps(Left.Columns[0], Right.Columns[0]);
		Result.Columns[1] = _mm_add_ps(Left.Columns[1], Right.Columns[1]);
		Result.Columns[2] = _mm_add_ps(Left.Columns[2], Right.Columns[2]);
		Result.Columns[3] = _mm_add_ps(Left.Columns[3], Right.Columns[3]);    
		
		return (Result);
	}
#else
	HMM_EXTERN mat4 HMM_AddMat4(mat4 Left, mat4 Right);
#endif
	
#ifdef HANDMADE_MATH__USE_SSE
	HMM_INLINE mat4 HMM_SubtractMat4(mat4 Left, mat4 Right)
	{
		mat4 Result;
		
		Result.Columns[0] = _mm_sub_ps(Left.Columns[0], Right.Columns[0]);
		Result.Columns[1] = _mm_sub_ps(Left.Columns[1], Right.Columns[1]);
		Result.Columns[2] = _mm_sub_ps(Left.Columns[2], Right.Columns[2]);
		Result.Columns[3] = _mm_sub_ps(Left.Columns[3], Right.Columns[3]);
		
		return (Result);
	}
#else
	HMM_EXTERN mat4 HMM_SubtractMat4(mat4 Left, mat4 Right);
#endif
	
	HMM_EXTERN mat4 HMM_MultiplyMat4(mat4 Left, mat4 Right);
	
#ifdef HANDMADE_MATH__USE_SSE
	HMM_INLINE mat4 HMM_MultiplyMat4f(mat4 Matrix, float Scalar)
	{
		mat4 Result;
		
		__m128 SSEScalar = _mm_set1_ps(Scalar);
		Result.Columns[0] = _mm_mul_ps(Matrix.Columns[0], SSEScalar);
		Result.Columns[1] = _mm_mul_ps(Matrix.Columns[1], SSEScalar);
		Result.Columns[2] = _mm_mul_ps(Matrix.Columns[2], SSEScalar);
		Result.Columns[3] = _mm_mul_ps(Matrix.Columns[3], SSEScalar);
		
		return (Result);
	}
#else
	HMM_EXTERN mat4 HMM_MultiplyMat4f(mat4 Matrix, float Scalar);
#endif
	
	HMM_EXTERN v4 HMM_MultiplyMat4ByVec4(mat4 Matrix, v4 Vector);
	
#ifdef HANDMADE_MATH__USE_SSE
	HMM_INLINE mat4 HMM_DivideMat4f(mat4 Matrix, float Scalar)
	{
		mat4 Result;
		
		__m128 SSEScalar = _mm_set1_ps(Scalar);
		Result.Columns[0] = _mm_div_ps(Matrix.Columns[0], SSEScalar);
		Result.Columns[1] = _mm_div_ps(Matrix.Columns[1], SSEScalar);
		Result.Columns[2] = _mm_div_ps(Matrix.Columns[2], SSEScalar);
		Result.Columns[3] = _mm_div_ps(Matrix.Columns[3], SSEScalar);    
		
		return (Result);
	}
#else
	HMM_EXTERN mat4 HMM_DivideMat4f(mat4 Matrix, float Scalar);
#endif
	
	
	/*
  * Common graphics transformations
  */
	
	HMM_INLINE mat4 HMM_Orthographic(float Left, float Right, float Bottom, float Top, float Near, float Far)
	{
		mat4 Result = HMM_Mat4();
		
		Result.Elements[0][0] = 2.0f / (Right - Left);
		Result.Elements[1][1] = 2.0f / (Top - Bottom);
		Result.Elements[2][2] = 2.0f / (Near - Far);
		Result.Elements[3][3] = 1.0f;
		
		Result.Elements[3][0] = (Left + Right) / (Left - Right);
		Result.Elements[3][1] = (Bottom + Top) / (Bottom - Top);
		Result.Elements[3][2] = (Far + Near) / (Near - Far);
		
		return (Result);
	}
	
	HMM_INLINE mat4 HMM_Perspective(float FOV, float AspectRatio, float Near, float Far)
	{
		mat4 Result = HMM_Mat4();
		
		float TanThetaOver2 = HMM_TanF(FOV * (HMM_PI32 / 360.0f));
		
		Result.Elements[0][0] = 1.0f / TanThetaOver2;
		Result.Elements[1][1] = AspectRatio / TanThetaOver2;
		Result.Elements[2][3] = -1.0f;
		Result.Elements[2][2] = (Near + Far) / (Near - Far);
		Result.Elements[3][2] = (2.0f * Near * Far) / (Near - Far);
		Result.Elements[3][3] = 0.0f;
		
		return (Result);
	}
	
	HMM_INLINE mat4 HMM_Translate(v3 Translation)
	{
		mat4 Result = HMM_Mat4d(1.0f);
		
		Result.Elements[3][0] = Translation.X;
		Result.Elements[3][1] = Translation.Y;
		Result.Elements[3][2] = Translation.Z;
		
		return (Result);
	}
	
	HMM_EXTERN mat4 HMM_Rotate(float Angle, v3 Axis);
	
	HMM_INLINE mat4 HMM_Scale(v3 Scale)
	{
		mat4 Result = HMM_Mat4d(1.0f);
		
		Result.Elements[0][0] = Scale.X;
		Result.Elements[1][1] = Scale.Y;
		Result.Elements[2][2] = Scale.Z;
		
		return (Result);
	}
	
	HMM_EXTERN mat4 HMM_LookAt(v3 Eye, v3 Center, v3 Up);
	
	
	/*
  * Quaternion operations
  */
	
	HMM_INLINE Quat quaternion(float X, float Y, float Z, float W)
	{
		Quat Result;
		
		Result.X = X;
		Result.Y = Y;
		Result.Z = Z;
		Result.W = W;
		
		return (Result);
	}
	
	HMM_INLINE Quat quaternionV4(v4 Vector)
	{
		Quat Result;
		
		Result.X = Vector.X;
		Result.Y = Vector.Y;
		Result.Z = Vector.Z;
		Result.W = Vector.W;
		
		return (Result);
	}
	
	HMM_INLINE Quat HMM_AddQuaternion(Quat Left, Quat Right)
	{
		Quat Result;
		
		Result.X = Left.X + Right.X;
		Result.Y = Left.Y + Right.Y;
		Result.Z = Left.Z + Right.Z;
		Result.W = Left.W + Right.W;
		
		return (Result);
	}
	
	HMM_INLINE Quat HMM_SubtractQuaternion(Quat Left, Quat Right)
	{
		Quat Result;
		
		Result.X = Left.X - Right.X;
		Result.Y = Left.Y - Right.Y;
		Result.Z = Left.Z - Right.Z;
		Result.W = Left.W - Right.W;
		
		return (Result);
	}
	
	HMM_INLINE Quat mul_quat(Quat Left, Quat Right)
	{
		Quat Result;
		
		Result.X = (Left.X * Right.W) + (Left.Y * Right.Z) - (Left.Z * Right.Y) + (Left.W * Right.X);
		Result.Y = (-Left.X * Right.Z) + (Left.Y * Right.W) + (Left.Z * Right.X) + (Left.W * Right.Y);
		Result.Z = (Left.X * Right.Y) - (Left.Y * Right.X) + (Left.Z * Right.W) + (Left.W * Right.Z);
		Result.W = (-Left.X * Right.X) - (Left.Y * Right.Y) - (Left.Z * Right.Z) + (Left.W * Right.W);
		
		return (Result);
	}
	
	HMM_INLINE Quat mul_quatF(Quat Left, float Multiplicative)
	{
		Quat Result;
		
		Result.X = Left.X * Multiplicative;
		Result.Y = Left.Y * Multiplicative;
		Result.Z = Left.Z * Multiplicative;
		Result.W = Left.W * Multiplicative;
		
		return (Result);
	}
	
	HMM_INLINE Quat HMM_DivideQuaternionF(Quat Left, float Dividend)
	{
		Quat Result;
		
		Result.X = Left.X / Dividend;
		Result.Y = Left.Y / Dividend;
		Result.Z = Left.Z / Dividend;
		Result.W = Left.W / Dividend;
		
		return (Result);
	}
	
	HMM_EXTERN Quat inverse_quat(Quat Left);
	
	HMM_INLINE float dot_quat(Quat Left, Quat Right)
	{
		float Result = (Left.X * Right.X) + (Left.Y * Right.Y) + (Left.Z * Right.Z) + (Left.W * Right.W);
		
		return (Result);
	}
	
	HMM_INLINE Quat normalize_quat(Quat Left)
	{
		Quat Result;
		
		float Length = HMM_SquareRootF(dot_quat(Left, Left));
		Result = HMM_DivideQuaternionF(Left, Length);
		
		return (Result);
	}
	
	HMM_INLINE Quat HMM_NLerp(Quat Left, float Time, Quat Right)
	{
		Quat Result;
		
		Result.X = lerp(Left.X, Time, Right.X);
		Result.Y = lerp(Left.Y, Time, Right.Y);
		Result.Z = lerp(Left.Z, Time, Right.Z);
		Result.W = lerp(Left.W, Time, Right.W);
		
		Result = normalize_quat(Result);
		
		return (Result);
	}
	
	HMM_EXTERN Quat HMM_Slerp(Quat Left, float Time, Quat Right);
	HMM_EXTERN mat4 quaternionToMat4(Quat Left);
	HMM_EXTERN Quat quaternionFromAxisAngle(v3 Axis, float AngleOfRotation);
	
#ifdef __cplusplus
}
#endif

#ifdef __clang__
#pragma GCC diagnostic pop
#endif

#endif /* HANDMADE_MATH_H */

#ifdef HANDMADE_MATH_IMPLEMENTATION

float HMM_Power(float Base, int Exponent)
{
    float Result = 1.0f;
    float Mul = Exponent < 0 ? 1.f / Base : Base;
    unsigned int X = Exponent < 0 ? -Exponent : Exponent;
    while (X)
    {
        if (X & 1)
        {
            Result *= Mul;
        }
        
        Mul *= Mul;
        X >>= 1;
    }
    
    return (Result);
}

#ifndef HANDMADE_MATH__USE_SSE
mat4 HMM_Transpose(mat4 Matrix)
{
    mat4 Result;
	
    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Rows][Columns] = Matrix.Elements[Columns][Rows];
        }
    }
	
    return (Result);
}
#endif

#ifndef HANDMADE_MATH__USE_SSE
mat4 HMM_AddMat4(mat4 Left, mat4 Right)
{
    mat4 Result;
	
    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Left.Elements[Columns][Rows] + Right.Elements[Columns][Rows];
        }
    }
	
    return (Result);
}
#endif

#ifndef HANDMADE_MATH__USE_SSE
mat4 HMM_SubtractMat4(mat4 Left, mat4 Right)
{
    mat4 Result;
	
    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Left.Elements[Columns][Rows] - Right.Elements[Columns][Rows];
        }
    }
	
    return (Result);
}
#endif

mat4 HMM_MultiplyMat4(mat4 Left, mat4 Right)
{
    mat4 Result;
	
#ifdef HANDMADE_MATH__USE_SSE
	
    Result.Columns[0] = HMM_LinearCombineSSE(Right.Columns[0], Left);
    Result.Columns[1] = HMM_LinearCombineSSE(Right.Columns[1], Left);
    Result.Columns[2] = HMM_LinearCombineSSE(Right.Columns[2], Left);
    Result.Columns[3] = HMM_LinearCombineSSE(Right.Columns[3], Left);     
    
#else
    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            float Sum = 0;
            int CurrentMatrice;
            for(CurrentMatrice = 0; CurrentMatrice < 4; ++CurrentMatrice)
            {
                Sum += Left.Elements[CurrentMatrice][Rows] * Right.Elements[Columns][CurrentMatrice];
            }
			
            Result.Elements[Columns][Rows] = Sum;
        }
    }
#endif
	
    return (Result);
}

#ifndef HANDMADE_MATH__USE_SSE
mat4 HMM_MultiplyMat4f(mat4 Matrix, float Scalar)
{
    mat4 Result;
	
    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Matrix.Elements[Columns][Rows] * Scalar;
        }
    }
	
    return (Result);
}
#endif

v4 HMM_MultiplyMat4ByVec4(mat4 Matrix, v4 Vector)
{
    v4 Result;
	
#ifdef HANDMADE_MATH__USE_SSE
	Result.InternalElementsSSE = HMM_LinearCombineSSE(Vector.InternalElementsSSE, Matrix);
#else
    int Columns, Rows;
    for(Rows = 0; Rows < 4; ++Rows)
    {
        float Sum = 0;
        for(Columns = 0; Columns < 4; ++Columns)
        {
            Sum += Matrix.Elements[Columns][Rows] * Vector.Elements[Columns];
        }
        
        Result.Elements[Rows] = Sum;
    }
#endif
	
    return (Result);
}

#ifndef HANDMADE_MATH__USE_SSE
mat4 HMM_DivideMat4f(mat4 Matrix, float Scalar)
{
    mat4 Result;
    
    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Matrix.Elements[Columns][Rows] / Scalar;
        }
    }
	
    return (Result);
}
#endif

mat4 HMM_Rotate(float Angle, v3 Axis)
{
    mat4 Result = HMM_Mat4d(1.0f);
    
    Axis = normalize_v3(Axis);
    
    float SinTheta = HMM_SinF(HMM_ToRadians(Angle));
    float CosTheta = HMM_CosF(HMM_ToRadians(Angle));
    float CosValue = 1.0f - CosTheta;
    
    Result.Elements[0][0] = (Axis.X * Axis.X * CosValue) + CosTheta;
    Result.Elements[0][1] = (Axis.X * Axis.Y * CosValue) + (Axis.Z * SinTheta);
    Result.Elements[0][2] = (Axis.X * Axis.Z * CosValue) - (Axis.Y * SinTheta);
    
    Result.Elements[1][0] = (Axis.Y * Axis.X * CosValue) - (Axis.Z * SinTheta);
    Result.Elements[1][1] = (Axis.Y * Axis.Y * CosValue) + CosTheta;
    Result.Elements[1][2] = (Axis.Y * Axis.Z * CosValue) + (Axis.X * SinTheta);
    
    Result.Elements[2][0] = (Axis.Z * Axis.X * CosValue) + (Axis.Y * SinTheta);
    Result.Elements[2][1] = (Axis.Z * Axis.Y * CosValue) - (Axis.X * SinTheta);
    Result.Elements[2][2] = (Axis.Z * Axis.Z * CosValue) + CosTheta;
    
    return (Result);
}

mat4 HMM_LookAt(v3 Eye, v3 Center, v3 Up)
{
    mat4 Result;
	
    v3 F = normalize_v3(sub_v3(Center, Eye));
    v3 S = normalize_v3(cross(F, Up));
    v3 U = cross(S, F);
	
    Result.Elements[0][0] = S.X;
    Result.Elements[0][1] = U.X;
    Result.Elements[0][2] = -F.X;
    Result.Elements[0][3] = 0.0f;
	
    Result.Elements[1][0] = S.Y;
    Result.Elements[1][1] = U.Y;
    Result.Elements[1][2] = -F.Y;
    Result.Elements[1][3] = 0.0f;
	
    Result.Elements[2][0] = S.Z;
    Result.Elements[2][1] = U.Z;
    Result.Elements[2][2] = -F.Z;
    Result.Elements[2][3] = 0.0f;
	
    Result.Elements[3][0] = -dot_v3(S, Eye);
    Result.Elements[3][1] = -dot_v3(U, Eye);
    Result.Elements[3][2] = dot_v3(F, Eye);
    Result.Elements[3][3] = 1.0f;
	
    return (Result);
}

Quat inverse_quat(Quat Left)
{
    Quat Conjugate;
    Quat Result;
    float Norm = 0;
    float NormSquared = 0;
	
    Conjugate.X = -Left.X;
    Conjugate.Y = -Left.Y;
    Conjugate.Z = -Left.Z;
    Conjugate.W = Left.W;
	
    Norm = HMM_SquareRootF(dot_quat(Left, Left));
    NormSquared = Norm * Norm;
	
    Result.X = Conjugate.X / NormSquared;
    Result.Y = Conjugate.Y / NormSquared;
    Result.Z = Conjugate.Z / NormSquared;
    Result.W = Conjugate.W / NormSquared;
	
    return (Result);
}

Quat HMM_Slerp(Quat Left, float Time, Quat Right)
{
    Quat Result;
    Quat QuaternionLeft;
    Quat QuaternionRight;
	
    float Cos_Theta = dot_quat(Left, Right);
    float Angle = HMM_ACosF(Cos_Theta);
    
    float S1 = HMM_SinF((1.0f - Time) * Angle);
    float S2 = HMM_SinF(Time * Angle);
    float Is = 1.0f / HMM_SinF(Angle);
	
    QuaternionLeft = mul_quatF(Left, S1);
    QuaternionRight = mul_quatF(Right, S2);
	
    Result = HMM_AddQuaternion(QuaternionLeft, QuaternionRight);
    Result = mul_quatF(Result, Is);
	
    return (Result);
}

mat4 quaternionToMat4(Quat Left)
{
    mat4 Result;
    Result = HMM_Mat4d(1);
	
    Quat NormalizedQuaternion = normalize_quat(Left);
    
    float XX, YY, ZZ,
	XY, XZ, YZ,
	WX, WY, WZ;
	
    XX = NormalizedQuaternion.X * NormalizedQuaternion.X;
    YY = NormalizedQuaternion.Y * NormalizedQuaternion.Y;
    ZZ = NormalizedQuaternion.Z * NormalizedQuaternion.Z;
    XY = NormalizedQuaternion.X * NormalizedQuaternion.Y;
    XZ = NormalizedQuaternion.X * NormalizedQuaternion.Z;
    YZ = NormalizedQuaternion.Y * NormalizedQuaternion.Z;
    WX = NormalizedQuaternion.W * NormalizedQuaternion.X;
    WY = NormalizedQuaternion.W * NormalizedQuaternion.Y;
    WZ = NormalizedQuaternion.W * NormalizedQuaternion.Z;
	
    Result.Elements[0][0] = 1.0f - 2.0f * (YY + ZZ);
    Result.Elements[0][1] = 2.0f * (XY + WZ);
    Result.Elements[0][2] = 2.0f * (XZ - WY);
	
    Result.Elements[1][0] = 2.0f * (XY - WZ);
    Result.Elements[1][1] = 1.0f - 2.0f * (XX + ZZ);
    Result.Elements[1][2] = 2.0f * (YZ + WX);
	
    Result.Elements[2][0] = 2.0f * (XZ + WY);
    Result.Elements[2][1] = 2.0f * (YZ - WX);
    Result.Elements[2][2] = 1.0f - 2.0f * (XX + YY);
	
    return (Result);
}

Quat quaternionFromAxisAngle(v3 Axis, float AngleOfRotation)
{
    Quat Result;
    
    v3 RotatedVector;
    
    float AxisNorm = 0;
    float SineOfRotation = 0;
	
    AxisNorm = HMM_SquareRootF(dot_v3(Axis, Axis));
    SineOfRotation = HMM_SinF(AngleOfRotation / 2.0f);
    RotatedVector = mul_v3f(Axis, SineOfRotation);
	
    Result.W = HMM_CosF(AngleOfRotation / 2.0f);
    Result.XYZ = div_v3f(RotatedVector, AxisNorm);
	
    return (Result);
}

#endif /* HANDMADE_MATH_IMPLEMENTATION */

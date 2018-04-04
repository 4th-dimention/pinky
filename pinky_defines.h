/*
Allen Webster
30.03.2018
*/

// TOP

#if !defined(PINKY_DEFINES_H)
#define PINKY_DEFINES_H

#include <stdint.h>
typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef int8_t  u8;
typedef int16_t u16;
typedef int32_t u32;
typedef int32_t u64;
typedef int8_t  b8;
typedef int32_t b32;
typedef float f32;
typedef double f64;

typedef i64 imem;
typedef u64 umem;

#define Stmnt(S) do{ S }while(0)
#define Break() exit(1)
#define AssertBreak() \
Stmnt( fprintf(stderr, "%s:%d: assert\n", __FILE__, __LINE__); Break(); )
#define Assert(c) Stmnt( if (!(c)) { AssertBreak(); } )
#define Implies(a,b) Assert(!(a) || (b))
#define InvalidPath AssertBreak()
#define NotImplemented AssertBreak()

#define ArrayCount(a) (sizeof(a)/sizeof((a)[0]))
#define PtrDif(a,b) ((u8*)a - (u8*)b)
#define PtrRawAdd(a,s) ((u8*)a + s)
#define PtrAsInt(p) (PtrDif(p, 0))
#define MemberOffsetAddr(T,m) (&((T*)0)->m)
#define MemberOffset(T,m) PtrAsInt(MemberOffsetAddr(T,m))
#define CastFromMember(T,m,p) (T*)(PtrRawAdd(p, -MemberOffset(T,m)))

#define internal static
#define global static
#define global_const static const
#define local_persist static
#define local_const static const

#define  B(x)  x
#define KB(x) (x << 10)
#define MB(x) (x << 20)
#define GB(x) (x << 30)
#define TB(x) (x << 40)

#endif

// BOTTOM


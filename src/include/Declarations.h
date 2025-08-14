#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <optional>
#include <atomic>

// Disable array indexing warnings
#pragma warning(disable : 26482 26446)

using std::string;
using std::nullopt;
using std::nullopt_t;
using std::optional;

template <class T>
using atomic_shared_ptr = std::atomic<std::shared_ptr<T>>;

typedef std::u16string UnicodeString;
typedef char16_t UnicodeChar;

#ifndef __cpp_lib_bitops
	namespace std
	{
		constexpr uint32_t bit_width(uint32_t v) noexcept
		{
			constexpr int MultiplyDeBruijnBitPosition[32] = {0, 9,  1,  10, 13, 21, 2,  29, 11, 14, 16, 18, 22, 25, 3, 30,
				8, 12, 20, 28, 15, 17, 24, 7,  19, 27, 23, 6,  26, 5,  4, 31};

			v |= v >> 1;  // first round up to one less than a power of 2
			v |= v >> 2;
			v |= v >> 4;
			v |= v >> 8;
			v |= v >> 16;

			const auto r = MultiplyDeBruijnBitPosition[(uint32_t)(v * 0x07C4ACDDU) >> 27];
			return r + 1;
		}

		constexpr uint32_t bit_ceil(uint32_t x) noexcept
		{
			--x;
			x |= (x >> 1);
			x |= (x >> 2);
			x |= (x >> 4);
			x |= (x >> 8);
			x |= (x >> 16);
			return ++x;
		}
	}
#endif

#define SIMD_SSE 1
#define SIMD_NEON 2

#ifdef _MSC_VER
	#define IFACE __declspec(novtable)
	#define RESTRICT __declspec(restrict)
	#define FUNC_NORETURN __declspec(noreturn)
#else
	#define IFACE
	#define RESTRICT
	#define __vectorcall
#endif

#ifdef ANDROID
	#define SIMD SIMD_NEON
#else
	#define SIMD SIMD_SSE
#endif

namespace System
{
	typedef uint8_t Byte;
	typedef uint8_t UInt8;
	typedef uint16_t UInt16;
	typedef uint32_t UInt32;
	typedef uint64_t UInt64;

	typedef int8_t Char;
	typedef int8_t Int8;
	typedef int16_t Int16;
	typedef int32_t Int32;
	typedef int64_t Int64;

	typedef uint_fast8_t FastByte;
	typedef uint_fast16_t FastUInt16;
	typedef uint_fast32_t FastUInt32;
	typedef uint_fast64_t FastUInt64;

	typedef int_fast8_t FastChar;
	typedef int_fast16_t FastInt16;
	typedef int_fast32_t FastInt32;
	typedef int_fast64_t FastInt64;

	typedef string String;
}  // namespace System

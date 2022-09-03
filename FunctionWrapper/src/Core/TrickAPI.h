#pragma once

// Core:

#if defined(_WIN64) && defined(_MSC_VER)
	#define T3D_PLATFORM_WINDOWS
#elif __linux__
	#define T3D_PLATFORM_LINUX
#endif

#if defined T3D_PLATFORM_WINDOWS
	#if defined T3D_BUILD_DLL
		#define T3D_API __declspec(dllexport)
	#else
		#define T3D_API __declspec(dllimport)
	#endif
#elif T3D_PLATFORM_LINUX
	#if defined T3D_BUILD_DLL
		#define T3D_API
	#else
		#define T3D_API
	#endif
#endif

#if defined _DEBUG
	#define T3D_DEBUG
#endif

#define T3D_AS_CHAR_ARRAY(Value) #Value

#define T3D_AS_STRING(Value) std::string(#Value)

#define T3D_AS_WIDE_STRING(Value) std::wstring(#Value)

#define T3D_PAIR(Value) { Value, #Value }

#define T3D_INLINE inline

#define T3D_CALL __fastcall

#define T3D_NO_DISCARD [[nodiscard]]

#define T3D_FALL_THROUGH [[fallthrough]]

#define T3D_PATH_LENGTH 256u

// Logger:

#if _MSC_VER
	#define T3D_FUNCTION __FUNCTION__
#else
	#define T3D_FUNCTION __func__
#endif


// Constructors:

// Removes copy constructor and copy assignment operator.
#define T3D_NO_COPY(Class) Class(const Class&) = delete; \
                           Class& operator = (const Class&) = delete

// Removes move constructor and move assignment operator.
#define T3D_NO_MOVE(Class) Class(Class&&) = delete; \
						   Class& operator = (Class&&) = delete

// Defines default copy constructor and copy assignment operator.
#define T3D_DEFAULT_COPY(Class) Class(const Class&) noexcept = default; \
                           Class& operator = (const Class&) = default

// Defines default move constructor and move assignment operator.
#define T3D_DEFAULT_MOVE(Class) Class(Class&&) noexcept = default; \
						   Class& operator = (Class&&) = default

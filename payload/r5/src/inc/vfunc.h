#pragma once

namespace vfunc
{
	template <typename Type>
	__forceinline Type get(void *instance, size_t index)
	{
		return (*reinterpret_cast<Type**>(instance))[index];
	}

	template <size_t Index, typename ReturnType, typename... Args>
	__forceinline ReturnType call(void *instance, Args... args)
	{
		using Fn = ReturnType(__thiscall *)(void *, Args...);
		auto function = (*reinterpret_cast<Fn **>(instance))[Index];
		return function(instance, args...);
	}
}

template <u64 length>
struct fake_class_t
{
	fake_class_t() = delete;
	fake_class_t(u64 vta)
	{
		vtable_addr = vta;
		memset(space, 0, length);
	}

	u64 vtable_addr;	// 0x0
	char space[length]; // 0x8
};

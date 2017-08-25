#pragma once

template <typename T>
class __declspec(novtable) Hook
{
public:
	Hook(PBYTE trampoline, T target) : m_pTrampoline(trampoline), fn(target) { }
	virtual ~Hook();
	virtual void add(PBYTE address) = 0;
	virtual void remove() = 0;
	T fn;

	PBYTE getTrampoline() const {
		return m_pTrampoline;
	}

protected:
	PBYTE m_pTrampoline;
	std::vector<PBYTE> addresses;
};

template <typename T>
Hook<T>::~Hook()
{
}

template <typename T>
void Hook<T>::add(PBYTE address)
{
	addresses.push_back(address);
}

template <typename T>
class CallHook : public Hook<T>
{
public:
	CallHook(PBYTE trampoline, T target) : Hook<T>(trampoline, target) { }
	~CallHook();
	void add(PBYTE address) override;
	void remove() override;
};

template <typename T>
void CallHook<T>::add(PBYTE address)
{
	*(BYTE*)address = 0xE8;

	*reinterpret_cast<int32_t*>(address + 1) =
		static_cast<int32_t>((intptr_t)m_pTrampoline - (intptr_t)address - 5);

	Hook<T>::add(address);
}

template <typename T>
void CallHook<T>::remove()
{
	for (auto & address : addresses)
	{
		*reinterpret_cast<int32_t*>(address + 1) =
			static_cast<int32_t>((intptr_t)fn - (intptr_t)address - 5);
	}
}

template <typename T>
CallHook<T>::~CallHook()
{
	CallHook<T>::remove();
}


template <typename T>
class JmpHook : public Hook<T>
{
public:
	JmpHook(PBYTE trampoline, T target) : Hook<T>(trampoline, target) { }
	~JmpHook();
	void add(PBYTE address) override;
	void remove() override;
};

template <typename T>
void JmpHook<T>::add(PBYTE address)
{
	*reinterpret_cast<int32_t*>(address + 2) =
		static_cast<int32_t>((intptr_t)m_pTrampoline - (intptr_t)address - 6);

	Hook<T>::add(address);
}

template <typename T>
void JmpHook<T>::remove()
{
	for (auto & address : addresses)
	{
		*reinterpret_cast<int32_t*>(address + 2) =
			static_cast<int32_t>((intptr_t)fn - (intptr_t)address - 6);
	}
}

template <typename T>
JmpHook<T>::~JmpHook()
{
	JmpHook<T>::remove();
}

template <typename T>
class VirtualHook : public Hook<T>
{
public:
	VirtualHook(PBYTE trampoline, T func) : Hook<T>(trampoline, func) { }
	~VirtualHook();	
	void add(PBYTE address) override;
	void remove() override;
};

template <typename T>
void VirtualHook<T>::add(PBYTE address)
{
	*reinterpret_cast<LPBYTE*>(address) = m_pTrampoline;

	Hook<T>::add(address);
}

template <typename T>
void VirtualHook<T>::remove()
{
	for (auto & address : addresses)
	{
		*reinterpret_cast<uintptr_t*>(address) = (uintptr_t)fn;
	}
}

template <typename T>
VirtualHook<T>::~VirtualHook()
{
	VirtualHook<T>::remove();
}

class HookManager
{
	static PBYTE CreateTrampolineFunction(PVOID lpTarget) 
	{
		auto pModule = GetModuleHandle( nullptr );
		return (PBYTE)AllocateFunctionStub(pModule, lpTarget, 0);
	}

public:
	template <typename T>
	static CallHook<T> * SetCall(PBYTE address, T fn)
	{
		T target = reinterpret_cast<T>(*reinterpret_cast<int32_t *>(address + 1) + (address + 5));

		auto pResult = new CallHook<T>(CreateTrampolineFunction(fn), target);

		pResult->add(address);

		return pResult;
	}

	template <typename T>
	static JmpHook<T> * SetJmp(PBYTE address, T fn)
	{
		T target = reinterpret_cast<T>(*reinterpret_cast<int32_t *>(address + 1) + (address + 5));

		auto pResult = new JmpHook<T>(CreateTrampolineFunction(fn), target);

		pResult->add(address);

		return pResult;
	}

	template <typename T>
	static VirtualHook<T> * HookVirtual(PBYTE address, T fn)
	{
		T orig = *reinterpret_cast<T*>(address);

		auto pResult = new VirtualHook<T>(CreateTrampolineFunction(fn), orig);

		pResult->add(address);

		return pResult;
	}

	static PVOID AllocateFunctionStub(PVOID origin, PVOID function, int type);

	static LPVOID FindPrevFreeRegion(LPVOID pAddress, LPVOID pMinAddr, DWORD dwAllocationGranularity);
};

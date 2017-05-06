#pragma once

namespace HAP
{
	struct ModuleInformation
	{
		ModuleInformation(const char* name) : Name(name)
		{
			MODULEINFO info;
			
			K32GetModuleInformation
			(
				GetCurrentProcess(),
				GetModuleHandleA(name),
				&info,
				sizeof(info)
			);

			MemoryBase = info.lpBaseOfDll;
			MemorySize = info.SizeOfImage;
		}

		const char* Name;

		void* MemoryBase;
		size_t MemorySize;
	};

	struct HookModuleBase
	{
		HookModuleBase
		(
			const char* module,
			const char* name,
			void* newfunc
		) :
			DisplayName(name),
			Module(module),
			NewFunction(newfunc)
		{

		}

		const char* DisplayName;
		const char* Module;

		void* TargetFunction;
		void* NewFunction;
		void* OriginalFunction;

		virtual MH_STATUS Create() = 0;
	};

	void Setup();
	void Close();

	void LogMessageText(const char* message);

	template <typename... Args>
	void LogMessage(const char* format, Args&&... args)
	{
		if (sizeof...(args) == 0)
		{
			LogMessageText(format);
			return;
		}

		char buf[256];
		sprintf_s(buf, format, std::forward<Args>(args)...);

		LogMessageText(buf);
	}

	constexpr auto MemoryPattern(const char* input)
	{
		return reinterpret_cast<const uint8_t*>(input);
	}

	void AddModule(HookModuleBase* module);

	using ShutdownFuncType = void(*)();
	void AddPluginShutdownFunction(ShutdownFuncType function);

	void* GetAddressFromPattern
	(
		const ModuleInformation& library,
		const uint8_t* pattern,
		const char* mask
	);

	struct PluginShutdownFunctionAdder
	{
		PluginShutdownFunctionAdder(ShutdownFuncType function)
		{
			AddPluginShutdownFunction(function);
		}
	};

	template <typename FuncSignature>
	class HookModuleMask final : public HookModuleBase
	{
	public:
		HookModuleMask
		(
			const char* module,
			const char* name,
			FuncSignature newfunction,
			const uint8_t* pattern,
			const char* mask
		) :
			HookModuleBase(module, name, newfunction),
			Pattern(pattern),
			Mask(mask)
		{
			AddModule(this);
		}

		inline auto GetOriginal() const
		{
			return static_cast<FuncSignature>(OriginalFunction);
		}

		virtual MH_STATUS Create() override
		{
			ModuleInformation info(Module);
			TargetFunction = GetAddressFromPattern(info, Pattern, Mask);

			auto res = MH_CreateHookEx
			(
				TargetFunction,
				NewFunction,
				&OriginalFunction
			);

			return res;
		}

	private:
		const uint8_t* Pattern;
		const char* Mask;
	};

	/*
		For use with unmodified addresses straight from IDA
	*/
	template <typename FuncSignature>
	class HookModuleStaticAddressTest final : public HookModuleBase
	{
	public:
		HookModuleStaticAddressTest
		(
			const char* module,
			const char* name,
			FuncSignature newfunction,
			uintptr_t address
		) :
			HookModuleBase(module, name, newfunction),
			Address(address)
		{
			AddModule(this);
		}

		inline auto GetOriginal() const
		{
			return static_cast<FuncSignature>(OriginalFunction);
		}

		virtual MH_STATUS Create() override
		{
			/*
				IDA starts addresses at this value
			*/
			Address -= 0x10000000;
			
			ModuleInformation info(Module);
			Address += (uintptr_t)info.MemoryBase;

			TargetFunction = (void*)Address;

			auto res = MH_CreateHookEx
			(
				TargetFunction,
				NewFunction,
				&OriginalFunction
			);

			return res;
		}

	private:
		uintptr_t Address;
	};

	/*
		For use where the correct address is found
		through another method
	*/
	template <typename FuncSignature>
	class HookModuleStaticAddress final : public HookModuleBase
	{
	public:
		HookModuleStaticAddress
		(
			const char* module,
			const char* name,
			FuncSignature newfunction,
			void* address
		) :
			HookModuleBase(module, name, newfunction),
			Address(address)
		{
			AddModule(this);
		}

		inline auto GetOriginal() const
		{
			return static_cast<FuncSignature>(OriginalFunction);
		}

		virtual MH_STATUS Create() override
		{
			TargetFunction = Address;

			auto res = MH_CreateHookEx
			(
				TargetFunction,
				NewFunction,
				&OriginalFunction
			);

			return res;
		}

	private:
		void* Address;
	};

	template <typename FuncSignature>
	class HookModuleAPI final : public HookModuleBase
	{
	public:
		HookModuleAPI
		(
			const char* module,
			const char* name,
			const char* exportname,
			FuncSignature newfunction
		) :
			HookModuleBase(module, name, newfunction),
			ExportName(exportname)
		{
			AddModule(this);
		}

		inline auto GetOriginal() const
		{
			return static_cast<FuncSignature>(OriginalFunction);
		}

		virtual MH_STATUS Create() override
		{
			wchar_t module[64];
			swprintf_s(module, L"%S", Module);

			auto res = MH_CreateHookApiEx
			(
				module,
				ExportName,
				NewFunction,
				&OriginalFunction,
				&TargetFunction
			);

			return res;
		}

	private:
		const char* ExportName;
	};

	struct AddressFinder
	{
		AddressFinder
		(
			const char* module,
			const uint8_t* pattern,
			const char* mask,
			int offset = 0
		)
		{
			auto addr = GetAddressFromPattern
			(
				module,
				pattern,
				mask
			);

			auto addrmod = static_cast<uint8_t*>(addr);

			/*
				Increment for any extra instructions
			*/
			addrmod += offset;

			Address = addrmod;
		}

		void* Get() const
		{
			return Address;
		}

		void* Address;
	};

	/*
		First byte at target address should be E8
	*/
	struct RelativeJumpFunctionFinder
	{
		RelativeJumpFunctionFinder
		(
			AddressFinder address
		)
		{
			auto addrmod = static_cast<uint8_t*>(address.Get());

			/*
				Skip the E8 byte
			*/
			addrmod += 1;

			auto offset = *reinterpret_cast<ptrdiff_t*>(addrmod);

			/*
				E8 jumps count relative distance from the next instruction,
				in 32 bit the distance will be measued in 4 bytes.
			*/
			addrmod += 4;

			/*
				Do the jump, address will now be the target function
			*/
			addrmod += offset;

			Address = addrmod;
		}

		void* Get() const
		{
			return Address;
		}

		void* Address;
	};
}
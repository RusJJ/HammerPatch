#include "PrecompiledHeader.hpp"
#include <process.h>

#include "Application\Application.hpp"

#define APPLICATION_VERSION	"v1.4.1 (RusJJ's branch)"

namespace
{
	unsigned int __stdcall MainThread(void* args)
	{
		HAP::CreateConsole();

		HAP::MessageNormal("Current version: " APPLICATION_VERSION "\n");

		try
		{
			HAP::CreateModules();
		}

		catch (MH_STATUS status)
		{
			HAP::MessageWarning("MinHook failed to initialize (error %d)\n", status);
			return 1;
		}

		try
		{
			HAP::CallStartupFunctions();
		}

		catch (const char* name)
		{
			HAP::MessageWarning("Startup procedure \"%s\" failed\n", name);
			return 1;
		}

		if (HAP::IsCSGO())
		{
			HAP::MessageNormal("CSGO's Hammer Patch applying:\n");
			HAP::BytePattern Pattern_Format03f = HAP::GetPatternFromString("25 2E 33 66 00");
			HAP::ModuleInformation info("hammer_dll.dll");
			void* address = HAP::GetAddressFromPattern(info, Pattern_Format03f);

			if (address)
			{
				DWORD old;
				if (VirtualProtect((void*)(address), 5, PAGE_EXECUTE_READWRITE, &old))
				{
					char* cSym = (char*)(address); cSym[2] += 2;
					HAP::MessageNormal("Hope it's patched!");
				}
				else
				{
					HAP::MessageNormal("Failed to.\n");
				}
			}
		}
		HAP::MessageNormal("HammerPatch finished loaded\n");
		return 1;
	}
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		_beginthreadex(nullptr, 0, MainThread, nullptr, 0, nullptr);
		break;
	}

	case DLL_PROCESS_DETACH:
	{
		HAP::Close();
		break;
	}
	}

	return 1;
}

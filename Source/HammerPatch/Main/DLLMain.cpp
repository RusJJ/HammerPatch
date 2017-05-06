#include "PrecompiledHeader.hpp"
#include <process.h>

#include "Application\Application.hpp"

namespace
{

}

namespace
{
	unsigned int __stdcall MainThread(void* args)
	{
		try
		{
			HAP::Setup();
		}

		catch (MH_STATUS status)
		{
			return 1;
		}

		HAP::LogMessage("HAP: HammerPatch loaded");

		return 1;
	}
}

BOOL APIENTRY DllMain
(
	HMODULE module,
	DWORD reason,
	LPVOID
)
{
	switch (reason)
	{
		case DLL_PROCESS_ATTACH:
		{
			_beginthreadex
			(
				nullptr,
				0,
				MainThread,
				nullptr,
				0,
				nullptr
			);

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
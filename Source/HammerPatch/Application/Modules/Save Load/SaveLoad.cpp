#include "PrecompiledHeader.hpp"
#include "Application\Application.hpp"

namespace
{
	struct Vector3
	{
		float X;
		float Y;
		float Z;
	};

	namespace HammerFunctions
	{
		namespace Types
		{
			using MapFaceCalcPlaneFromFacePoints = void(__fastcall*)
			(
				void* thisptr,
				void* edx
			);

			using MapFaceAllocatePoints = size_t(__fastcall*)
			(
				void* thisptr,
				void* edx,
				int count
			);

			using MapFaceCreateFace = void(__fastcall*)
			(
				void* thisptr,
				void* edx,
				Vector3* points,
				int count,
				bool iscordonface
			);
		}

		Types::MapFaceCalcPlaneFromFacePoints MapFaceCalcPlaneFromFacePoints;
		Types::MapFaceAllocatePoints MapFaceAllocatePoints;
		Types::MapFaceCreateFace MapFaceCreateFace;

		template <typename T>
		void SetFromAddress(T& type, void* address)
		{
			type = (T)(address);
		}

		bool Init()
		{
			/*
				MapFaceCalcPlaneFromFacePoints
			*/
			{
				/*
					0x1012F540 static Hammer IDA address May 8 2017
				*/
				HAP::AddressFinder address
				(
					"hammer_dll.dll",
					HAP::MemoryPattern
					(
						"\x8B\xC1\x83\xB8\x00\x00\x00\x00\x00\x7C\x23\x56"
						"\x8B\xB0\x00\x00\x00\x00\x85\xF6\x74\x17\x57\x8D"
						"\xB8\x00\x00\x00\x00\xB9\x00\x00\x00\x00\xF3\xA5"
						"\x5F\x8B\xC8\x5E\xE9\x00\x00\x00\x00\x5E\xC3"
					),
					"xxxx?????xxxxx????xxxxxxx????x????xxxxxxx????xx"
				);

				SetFromAddress
				(
					MapFaceCalcPlaneFromFacePoints,
					address.Get()
				);

				HAP::LogMessage
				(
					"HAP: SaveLoadInit: "
					"\"MapFaceCalcPlaneFromFacePoints\" -> "
					"hammer_dll.dll @ 0x%p\n",
					address.Get()
				);
			}

			/*
				MapFaceAllocatePoints
			*/
			{
				/*
					0x1012F370 static Hammer IDA address May 8 2017
				*/
				HAP::AddressFinder address
				(
					"hammer_dll.dll",
					HAP::MemoryPattern
					(
						"\x55\x8B\xEC\x64\xA1\x00\x00\x00\x00\x6A\xFF\x68"
						"\x00\x00\x00\x00\x50\x64\x89\x25\x00\x00\x00\x00"
						"\x56\x8B\xF1\x57\x8B\x7D\x08\x8B\x86\x00\x00\x00"
						"\x00"
					),
					"xxxxx????xxx????xxxx????xxxxxxxxx????"
				);

				SetFromAddress
				(
					MapFaceAllocatePoints,
					address.Get()
				);

				HAP::LogMessage
				(
					"HAP: SaveLoadInit: "
					"\"MapFaceAllocatePoints\" -> "
					"hammer_dll.dll @ 0x%p\n",
					address.Get()
				);
			}

			/*
				MapFaceCreateFace
			*/
			{
				/*
					0x10130490 static Hammer IDA address May 9 2017
				*/
				HAP::AddressFinder address
				(
					"hammer_dll.dll",
					HAP::MemoryPattern
					(
						"\x55\x8B\xEC\x53\x56\x6A\x00\x8B\xD9\xE8\x00\x00"
						"\x00\x00\x8B\x45\x0C\x83\xC4\x04\x8B\xCB\x85\xC0"
						"\x7E\x2A\x50\xE8\x00\x00\x00\x00\x8B\x83\x00\x00"
						"\x00\x00"
					),
					"xxxxxxxxxx????xxxxxxxxxxxxxx????xx????"
				);

				SetFromAddress
				(
					MapFaceCreateFace,
					address.Get()
				);

				HAP::LogMessage
				(
					"HAP: SaveLoadInit: "
					"\"MapFaceCreateFace\" -> "
					"hammer_dll.dll @ 0x%p\n",
					address.Get()
				);
			}

			return true;
		}

		HAP::StartupFunctionAdder Adder1
		(
			"SaveLoadInit",
			Init
		);
	}
}

namespace
{
	struct ScopedFile
	{
		ScopedFile(const char* path, const char* mode)
		{
			Assign(path, mode);
		}

		~ScopedFile()
		{
			Close();
		}

		void Close()
		{
			if (Handle)
			{
				fclose(Handle);
				Handle = nullptr;
			}
		}

		auto Get() const
		{
			return Handle;
		}

		explicit operator bool() const
		{
			return Get() != nullptr;
		}

		bool Assign(const char* path, const char* mode)
		{
			Handle = fopen(path, mode);
			return Handle != nullptr;
		}

		template <typename... Types>
		bool WriteSimple(Types&&... args)
		{
			size_t adder[] =
			{
				[&]()
				{
					return fwrite
					(
						&args,
						sizeof(args),
						1,
						Get()
					);
				}()...
			};

			for (auto value : adder)
			{
				if (value == 0)
				{
					return false;
				}
			}

			return true;
		}

		size_t WriteRegion
		(
			void* start,
			size_t size,
			int count = 1
		)
		{
			return fwrite
			(
				start,
				size,
				count,
				Get()
			);
		}

		template <typename... Types>
		bool ReadSimple(Types&... args)
		{
			size_t adder[] =
			{
				[&]()
				{
					return fread_s
					(
						&args,
						sizeof(args),
						sizeof(args),
						1,
						Get()
					);
				}()...
			};

			for (auto value : adder)
			{
				if (value == 0)
				{
					return false;
				}
			}

			return true;
		}

		template <typename T>
		size_t ReadRegion
		(
			std::vector<T>& vec,
			int count = 1
		)
		{
			return fread_s
			(
				&vec[0],
				vec.size() * sizeof(T),
				sizeof(T),
				count,
				Get()
			);
		}

		FILE* Handle = nullptr;
	};

	struct MapFace
	{
		static Vector3* GetPointsPtr(void* thisptr)
		{
			HAP::StructureWalker walker(thisptr);
			auto ret = *(Vector3**)walker.Advance(340);

			return ret;
		}

		static int GetPointCount(void* thisptr)
		{
			HAP::StructureWalker walker(thisptr);
			auto ret = *(int*)walker.Advance(344);

			return ret;
		}

		static int GetFaceID(void* thisptr)
		{
			HAP::StructureWalker walker(thisptr);
			auto ret = *(int*)walker.Advance(412);
			
			return ret;
		}

		static void CalcPlaneFromFacePoints(void* thisptr)
		{
			HammerFunctions::MapFaceCalcPlaneFromFacePoints
			(
				thisptr,
				nullptr
			);
		}

		static size_t AllocatePoints(void* thisptr, int count)
		{
			return HammerFunctions::MapFaceAllocatePoints
			(
				thisptr,
				nullptr,
				count
			);
		}

		static void CreateFace
		(
			void* thisptr,
			Vector3* points,
			int count,
			bool iscordonface
		)
		{
			HammerFunctions::MapFaceCreateFace
			(
				thisptr,
				nullptr,
				points,
				count,
				iscordonface
			);
		}

		int ParentSolidID;
		int ID;

		std::vector<Vector3> Points;
	};

	struct MapSolid
	{
		static int GetID(void* thisptr)
		{
			HAP::StructureWalker walker(thisptr);
			auto ret = *(int*)walker.Advance(164);

			return ret;
		}

		static int GetFaceCount(void* thisptr)
		{
			HAP::StructureWalker walker(thisptr);
			auto ret = *(short*)walker.Advance(556);

			return ret;
		}

		int ID;
		std::vector<MapFace> Faces;
	};

	struct
	{
		char VertexFileName[1024];

		ScopedFile* FilePtr;

		bool IsLoading = false;
		bool IsSaving = false;
	} SharedData;

	struct
	{
		
	} SaveData;

	struct
	{
		std::vector<MapSolid> Solids;
		MapSolid* CurrentSolid = nullptr;
	} LoadData;
}

namespace
{
	namespace Module_MapDocLoad
	{
		#pragma region Init

		/*
			0x10097AC0 static Hammer IDA address May 8 2017
		*/
		auto Pattern = HAP::MemoryPattern
		(
			"\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1"
			"\x00\x00\x00\x00\x50\x64\x89\x25\x00\x00\x00\x00"
			"\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x85"
			"\xC0\x53"
		);

		auto Mask =
		(
			"xxxxxx????xx????xxxx????xx????x????xxx"
		);

		bool __fastcall Override
		(
			void* thisptr,
			void* edx,
			const char* filename,
			bool unk
		);

		using ThisFunction = decltype(Override)*;

		HAP::HookModuleMask<ThisFunction> ThisHook
		{
			"hammer_dll.dll", "MapDocLoad", Override, Pattern, Mask
		};

		#pragma endregion

		bool __fastcall Override
		(
			void* thisptr,
			void* edx,
			const char* filename,
			bool unk
		)
		{
			SharedData.IsLoading = true;

			strcpy_s(SharedData.VertexFileName, filename);
			PathRenameExtensionA(SharedData.VertexFileName, ".hpverts");

			ScopedFile file(SharedData.VertexFileName, "rb");

			if (!file)
			{
				HAP::LogMessageText("HAP: Could not open vertex file\n");
			}

			SharedData.FilePtr = &file;

			auto ret = ThisHook.GetOriginal()
			(
				thisptr,
				edx,
				filename,
				unk
			);

			SharedData.FilePtr = nullptr;
			SharedData.IsLoading = false;

			return ret;
		}
	}

	namespace Module_MapDocSave
	{
		#pragma region Init

		/*
			0x100A36E0 static Hammer IDA address May 8 2017
		*/
		auto Pattern = HAP::MemoryPattern
		(
			"\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1"
			"\x00\x00\x00\x00\x50\x64\x89\x25\x00\x00\x00\x00"
			"\x81\xEC\x00\x00\x00\x00\x53\x56\x57\x8B\xF9\x8D"
			"\x8D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xC7\x45"
			"\x00\x00\x00\x00\x00\x8D\x8D\x00\x00\x00\x00\x8B"
			"\x5D\x08\x6A\x01\x53\xE8\x00\x00\x00\x00\x8B\xCF"
			"\x8B\xF0\xE8\x00\x00\x00\x00\x68\x00\x00\x00\x00"
			"\xE8\x00\x00\x00\x00"
		);

		auto Mask =
		(
			"xxxxxx????xx????xxxx????xx????xxxxxxx????x????xx"
			"?????xx????xxxxxxx????xxxxx????x????x????"
		);

		bool __fastcall Override
		(
			void* thisptr,
			void* edx,
			const char* filename,
			int saveflags
		);

		using ThisFunction = decltype(Override)*;

		HAP::HookModuleMask<ThisFunction> ThisHook
		{
			"hammer_dll.dll", "MapDocSave", Override, Pattern, Mask
		};

		#pragma endregion

		bool __fastcall Override
		(
			void* thisptr,
			void* edx,
			const char* filename,
			int saveflags
		)
		{
			SharedData.IsSaving = true;

			strcpy_s(SharedData.VertexFileName, filename);
			PathRenameExtensionA(SharedData.VertexFileName, ".hpverts");

			ScopedFile file(SharedData.VertexFileName, "wb");

			if (!file)
			{
				HAP::LogMessageText("HAP: Could not create vertex file\n");
			}

			SharedData.FilePtr = &file;

			auto ret = ThisHook.GetOriginal()
			(
				thisptr,
				edx,
				filename,
				saveflags
			);

			SharedData.FilePtr = nullptr;
			SharedData.IsSaving = false;

			return ret;
		}
	}

	namespace Module_MapSolidLoad
	{
		/*
			0x10148C10 static Hammer IDA address May 8 2017
		*/
		auto Pattern = HAP::MemoryPattern
		(
			"\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1"
			"\x00\x00\x00\x00\x50\x64\x89\x25\x00\x00\x00\x00"
			"\x83\xEC\x10\x53\x56\x8B\xD9\x8D\x4D\xE4"
		);

		auto Mask =
		(
			"xxxxxx????xx????xxxx????xxxxxxxxxx"
		);

		int __fastcall Override
		(
			void* thisptr,
			void* edx,
			void* file,
			bool& valid
		);

		using ThisFunction = decltype(Override)*;

		HAP::HookModuleMask<ThisFunction> ThisHook
		{
			"hammer_dll.dll", "MapSolidLoad", Override, Pattern, Mask
		};

		int __fastcall Override
		(
			void* thisptr,
			void* edx,
			void* file,
			bool& valid
		)
		{
			auto ret = ThisHook.GetOriginal()
			(
				thisptr,
				edx,
				file,
				valid
			);

			return ret;
		}
	}

	namespace Module_MapWorldLoadSolidCallback
	{
		#pragma region Init

		/*
			0x101508D0 static Hammer IDA address May 8 2017
		*/
		auto Pattern = HAP::MemoryPattern
		(
			"\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1"
			"\x00\x00\x00\x00\x50\x64\x89\x25\x00\x00\x00\x00"
			"\x83\xEC\x08\x56\x57\x68\x00\x00\x00\x00\xE8\x00"
			"\x00\x00\x00\x83\xC4\x04\x89\x45\xEC\xC7\x45\x00"
			"\x00\x00\x00\x00\x85\xC0\x74\x0D\x6A\x00\x8B\xC8"
			"\xE8\x00\x00\x00\x00\x8B\xF0\xEB\x02\x33\xF6\xC7"
			"\x45\x00\x00\x00\x00\x00\x8D\x45\xF3\x50\xFF\x75"
			"\x08\x8B\xCE\xE8\x00\x00\x00\x00\x8B\xF8\x85\xFF"
			"\x75\x30\x38\x45\xF3\x74\x2B\x68\x00\x00\x00\x00"
		);

		auto Mask =
		(
			"xxxxxx????xx????xxxx????xxxxxx????x????xxxxxxxx?"
			"????xxxxxxxxx????xxxxxxxx?????xxxxxxxxxx????xxxx"
			"xxxxxxxx????"
		);

		int __cdecl Override
		(
			void* file,
			void* mapworld
		);

		using ThisFunction = decltype(Override)*;

		HAP::HookModuleMask<ThisFunction> ThisHook
		{
			"hammer_dll.dll", "MapWorldLoadSolidCallback", Override, Pattern, Mask
		};

		#pragma endregion

		int __cdecl Override
		(
			void* file,
			void* mapworld
		)
		{
			if (SharedData.FilePtr)
			{
				LoadData.Solids.emplace_back();
				LoadData.CurrentSolid = &LoadData.Solids.back();

				int facecount;

				SharedData.FilePtr->ReadSimple
				(
					LoadData.CurrentSolid->ID,
					facecount
				);

				LoadData.CurrentSolid->Faces.resize(facecount);

				for (auto& face : LoadData.CurrentSolid->Faces)
				{
					MapFace entry;
					int pointscount;

					SharedData.FilePtr->ReadSimple
					(
						entry.ID,
						pointscount
					);

					entry.Points.resize(pointscount);

					SharedData.FilePtr->ReadRegion
					(
						entry.Points,
						pointscount
					);

					entry.ParentSolidID = LoadData.CurrentSolid->ID;

					face = std::move(entry);
				}
			}

			auto ret = ThisHook.GetOriginal()
			(
				file,
				mapworld
			);

			LoadData.CurrentSolid = nullptr;

			return ret;
		}
	}

	namespace Module_MapSolidLoadSideCallback
	{
		#pragma region Init

		/*
			0x10148B60 static Hammer IDA address May 8 2017
		*/
		auto Pattern = HAP::MemoryPattern
		(
			"\x55\x8B\xEC\x53\x56\x57\x8B\x7D\x0C\x0F\xBF\xB7"
			"\x00\x00\x00\x00\x8D\x8F\x00\x00\x00\x00\x8D\x46"
			"\x01\x50\xE8\x00\x00\x00\x00"
		);

		auto Mask =
		(
			"xxxxxxxxxxxx????xx????xxxxx????"
		);

		int __cdecl Override
		(
			void* file,
			void* solidptr
		);

		using ThisFunction = decltype(Override)*;

		HAP::HookModuleMask<ThisFunction> ThisHook
		{
			"hammer_dll.dll", "MapSolidLoadSideCallback", Override, Pattern, Mask
		};

		#pragma endregion

		int __cdecl Override
		(
			void* file,
			void* solidptr
		)
		{
			auto ret = ThisHook.GetOriginal()
			(
				file,
				solidptr
			);

			return ret;
		}
	}

	namespace Module_MapSolidSave
	{
		#pragma region Init

		/*
			0x10149C90 static Hammer IDA address May 8 2017
		*/
		auto Pattern = HAP::MemoryPattern
		(
			"\x55\x8B\xEC\x83\xEC\x08\x57\x8B\xF9\x8B\x4D\x0C"
			"\x57\x89\x7D\xFC\xE8\x00\x00\x00\x00\x84\xC0"
		);

		auto Mask =
		(
			"xxxxxxxxxxxxxxxxx????xx"
		);

		int __fastcall Override
		(
			void* thisptr,
			void* edx,
			void* file,
			void* saveinfo
		);

		using ThisFunction = decltype(Override)*;

		HAP::HookModuleMask<ThisFunction> ThisHook
		{
			"hammer_dll.dll", "MapSolidSave", Override, Pattern, Mask
		};

		#pragma endregion

		int __fastcall Override
		(
			void* thisptr,
			void* edx,
			void* file,
			void* saveinfo
		)
		{
			if (SharedData.FilePtr)
			{
				auto id = MapSolid::GetID(thisptr);
				auto facecount = MapSolid::GetFaceCount(thisptr);

				SharedData.FilePtr->WriteSimple
				(
					id,
					facecount
				);
			}

			auto ret = ThisHook.GetOriginal()
			(
				thisptr,
				edx,
				file,
				saveinfo
			);

			return ret;
		}
	}

	#if 0
	namespace Module_Scanf
	{
		#pragma region Init

		HAP::RelativeJumpFunctionFinder Address
		{
			HAP::AddressFinder
			(
				/*
					0x101336E3 static Hammer IDA address May 7 2017
				*/
				"hammer_dll.dll",
				HAP::MemoryPattern
				(
					"\xE8\x00\x00\x00\x00\x83\xC4\x2C\x83\xF8\x09\x0F"
					"\x84\x00\x00\x00\x00\x5F\x5E\xB8\x00\x00\x00\x00"
					"\x5B\x5D\xC3"
				),
				"x????xxxxxxxx????xxx????xxx"
			)
		};

		int __cdecl Override
		(
			const char* buffer,
			const char* format,
			...
		);

		using ThisFunction = decltype(Override)*;

		HAP::HookModuleStaticAddress<ThisFunction> ThisHook
		{
			"hammer_dll.dll", "Scanf", Override, Address.Get()
		};

		#pragma endregion

		int __cdecl Override
		(
			const char* buffer,
			const char* format,
			...
		)
		{
			if (SharedData.IsLoading && LoadData.ScanfEntryCounter == 0)
			{
				/*SaveLoadData.ScanfEntryCounter++;

				int64_t values[9];

				auto ret = ThisHook.GetOriginal()
				(
					buffer,
					"(%lld %lld %lld) (%lld %lld %lld) (%lld %lld %lld)",
					&values[0],
					&values[1],
					&values[2],
					&values[3],
					&values[4],
					&values[5],
					&values[6],
					&values[7],
					&values[8]
				);

				for (size_t i = 0; i < 9; i++)
				{
					auto address = va_arg(args, float*);
					*address = *(double*)&values[i];
				}*/

				return 9;
			}

			va_list args;
			va_start(args, format);

			auto ret = vsscanf(buffer, format, args);

			va_end(args);

			return ret;
		}
	}

	namespace Module_Sprintf
	{
		#pragma region Init

		/*
			0x10339CC9 static Hammer IDA address May 7 2017
		*/
		auto Pattern = HAP::MemoryPattern
		(
			"\x55\x8B\xEC\x83\xEC\x20\x53\x57\x33\xDB\x8D\x7D"
			"\xE4\x6A\x07\x33\xC0\x89\x5D\xE0\x59\xF3\xAB\x39"
			"\x45\x0C\x75\x15\xE8\x00\x00\x00\x00\xC7\x00\x00"
			"\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC8\xFF\xEB"
			"\x4D\x8B\x45\x08"
		);

		auto Mask =
		(
			"xxxxxxxxxxxxxxxxxxxxxxxxxxxxx????xx????x????xxxx"
			"xxxx"
		);

		int __cdecl Override
		(
			char* buffer,
			const char* format,
			...
		);

		using ThisFunction = decltype(Override)*;

		HAP::HookModuleMask<ThisFunction> ThisHook
		{
			"hammer_dll.dll", "Sprintf", Override, Pattern, Mask
		};

		#pragma endregion

		int __cdecl Override
		(
			char* buffer,
			const char* format,
			...
		)
		{
			if (SharedData.IsSaving && SaveData.SprintfEntryCounter == 0)
			{
				SaveData.SprintfEntryCounter++;

				return ThisHook.GetOriginal()(buffer, "");
			}

			va_list args;
			va_start(args, format);

			auto ret = vsprintf(buffer, format, args);

			va_end(args);

			return ret;
		}
	}
	#endif

	namespace Module_MapFaceLoadKeyCallback
	{
		#pragma region Init

		/*
			0x10133620 static Hammer IDA address May 7 2017
		*/
		auto Pattern = HAP::MemoryPattern
		(
			"\x55\x8B\xEC\x53\x56\x57\x6A\x00\xE8\x00\x00\x00"
			"\x00\x8B\x5D\x10\x8B\x7D\x08\x68\x00\x00\x00\x00"
			"\x57\x8B\x33\xE8\x00\x00\x00\x00"
		);

		auto Mask =
		(
			"xxxxxxxxx????xxxxxxx????xxxx????"
		);

		int __cdecl Override
		(
			const char* key,
			const char* value,
			void* loadface
		);

		using ThisFunction = decltype(Override)*;

		HAP::HookModuleMask<ThisFunction> ThisHook
		{
			"hammer_dll.dll", "MapFaceLoadKeyCallback", Override, Pattern, Mask
		};

		#pragma endregion

		int __cdecl Override
		(
			const char* key,
			const char* value,
			void* loadface
		)
		{
			auto ret = ThisHook.GetOriginal()
			(
				key,
				value,
				loadface
			);

			return ret;
		}
	}

	namespace Module_MapFaceSave
	{
		#pragma region Init

		/*
			0x10135C30 static Hammer IDA address May 7 2017
		*/
		auto Pattern = HAP::MemoryPattern
		(
			"\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x56\x57\x8B"
			"\xF1\xE8\x00\x00\x00\x00\x6A\x00\x8B\xCE\xE8\x00"
			"\x00\x00\x00"
		);

		auto Mask =
		(
			"xxxxx????xxxxx????xxxxx????"
		);

		int __fastcall Override
		(
			void* thisptr,
			void* edx,
			void* file,
			void* saveinfo
		);

		using ThisFunction = decltype(Override)*;

		HAP::HookModuleMask<ThisFunction> ThisHook
		{
			"hammer_dll.dll", "MapFaceSave", Override, Pattern, Mask
		};

		#pragma endregion

		int __fastcall Override
		(
			void* thisptr,
			void* edx,
			void* file,
			void* saveinfo
		)
		{
			if (SharedData.FilePtr)
			{				
				auto pointsaddr = MapFace::GetPointsPtr(thisptr);
				auto pointscount = MapFace::GetPointCount(thisptr);
				auto faceid = MapFace::GetFaceID(thisptr);

				SharedData.FilePtr->WriteSimple
				(
					faceid,
					pointscount
				);

				SharedData.FilePtr->WriteRegion
				(
					pointsaddr,
					sizeof(Vector3),
					pointscount
				);
			}

			auto ret = ThisHook.GetOriginal()
			(
				thisptr,
				edx,
				file,
				saveinfo
			);

			return ret;
		}
	}
}

#include <iostream>
#include <AOB_PatchManager.h>
/*
First time pressing OP:

CalcViewModel.dll+10EFB5:
7FFC8D54EFA9 - 48 8D 8E 60010000  - lea rcx,[rsi+00000160]
7FFC8D54EFB0 - E8 4BF7FEFF - call CalcViewModel.dll+FE700
7FFC8D54EFB5 - 44 89 76 18  - mov [rsi+18],r14d <<
7FFC8D54EFB9 - 48 8D 8E 700F0000  - lea rcx,[rsi+00000F70]
7FFC8D54EFC0 - 41 B1 01 - mov r9b,01

On each OP change:

CalcViewModel.dll+10EFE0:
7FFC8D54EFD4 - C6 86 220E0000 01 - mov byte ptr [rsi+00000E22],01
7FFC8D54EFDB - E9 5E1F0000 - jmp CalcViewModel.dll+110F3E
7FFC8D54EFE0 - 44 89 76 18  - mov [rsi+18],r14d <<
7FFC8D54EFE4 - 44 38 3E  - cmp [rsi],r15b
7FFC8D54EFE7 - 74 50 - je CalcViewModel.dll+10F039
*/
#define TEST

extern "C" void patch_CalcMulAsAdd_first();
extern "C" void patch_CalcMulAsAdd_second();

int main()
{
#ifdef TEST
	AOB_PatchManager pm(L"CalculatorApp.exe");
	pm.createNewPatch<PatchWithTrampoline>("Calc_MulAsAdd_Set")
		.setCodeCaveInfo({ L"CalcViewModel.dll", 0x1623E0 })
		.setOrigCodeInfo({ L"CalcViewModel.dll", 0x10EFB5 }, 11)
		.setNewCodeInfo(reinterpret_cast<std::uint8_t*>(patch_CalcMulAsAdd_first))
		.formatNextRelative({ L"CalcViewModel.dll", 0x10EFC0 });

	pm.createNewPatch<PatchWithTrampoline>("Calc_MulAsAss_Chg")
		.setCodeCaveInfo({ L"CalcViewModel.dll", 0x162420 })
		.setOrigCodeInfo({ L"CalcViewModel.dll", 0x10EFE0 }, 7)
		.setNewCodeInfo(reinterpret_cast<std::uint8_t*>(patch_CalcMulAsAdd_second))
		.formatNextRelative({ L"CalcViewModel.dll", 0x10EFE7 });

	pm.applyPatch("Calc_MulAsAdd_Set");
	pm.applyPatch("Calc_MulAsAss_Chg");
	

	//printf("%llx\n", &pt);
	system("pause");

	pm.revertPatch("Calc_MulAsAdd_Set");
	pm.revertPatch("Calc_MulAsAss_Chg");

#else

	std::shared_ptr<ProcMem>procMem(new ProcMem(L"CalculatorApp.exe"));
	printf("PID: %x\n", procMem->getProcessID());
	if (!procMem || !procMem->getConnectedState())
	{
		printf("Failed to connect to the process!\n");
		return 0;
	}
	printf("Successfully connected to the process!\n");
	system("pause");
	std::uintptr_t CalcViewModel = procMem->getModuleBaseAddress(L"CalcViewModel.dll");
	if (CalcViewModel == 0)
	{
		printf("Failed to find CalcViewModel.dll module!\n");
		return 0;
	}
	printf("Successfully found CalcViewModel.dll module!\n");

	// Code cave [100 bytes]: CalcViewModel.dll + 0x1623E0
	PatchWithTrampoline pt("Calc_MulAsAdd_Set", procMem, CalcViewModel + 0x10EFB5, 11, reinterpret_cast<std::uintptr_t*>(patch_CalcMulAsAdd_first), CalcViewModel + 0x1623E0);
	pt.formatNextRelative(CalcViewModel + 0x10EFC0);
	pt.patch();
	// Code cave [100 bytes]: CalcViewModel.dll + 0x162420
	PatchWithTrampoline pt2("Calc_MulAsAdd_Change", procMem, CalcViewModel + 0x10EFE0, 7, reinterpret_cast<std::uintptr_t*>(patch_CalcMulAsAdd_second), CalcViewModel + 0x162420);
	pt2.formatNextRelative(CalcViewModel + 0x10EFE7);
	pt2.patch();

	system("pause");
	pt.unpatch();
	pt2.unpatch();
#endif // TEST
	return 0;
}
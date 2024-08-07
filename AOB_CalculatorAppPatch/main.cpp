#include <iostream>
#include <AOB_PatchManager.h>

#define LOG(x) std::cout<< x << std::endl;

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

extern "C" void patch_CalcMulAsAdd_first();
extern "C" void patch_CalcMulAsAdd_second();

int main()
{
	AOB_PatchManager pm(L"CalculatorApp.exe");
	if (pm.lastError != eLastError::OK)
	{
		LOG("An error occured while creating AOB_PatchManager class!");
		return;
	}

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
	
	system("pause");

	pm.revertPatch("Calc_MulAsAdd_Set");
	pm.revertPatch("Calc_MulAsAss_Chg");

	return 0;
}
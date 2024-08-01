#include "AOB_PatchClasses.h"

PatchClass::PatchClass(
	const std::wstring& pName,
	std::shared_ptr<ProcMem> tProc_Mem,
	std::uintptr_t tProc_origCodeAddr,
	size_t tProc_origCodeSize,
	std::uintptr_t* patch_newCodeAOBAddr)
	:
	patchName(pName),
	tProcess_Mem(tProc_Mem),
	tProcess_OrigCodeAddr(tProc_origCodeAddr),
	tProcess_OrigCodeSize(tProc_origCodeSize),
	patch_NewCodeAddr(patch_newCodeAOBAddr),
	isActive(false)
{
	patch_OrigCodeRawCopy = tProc_Mem->readMemoryArray<std::uint8_t>(tProc_origCodeAddr, tProcess_OrigCodeSize);
	patch_NewCodeSize = asmPatchUtils_findPattern32(patch_NewCodeAddr, 0xDEADC0DE); // 0xDEADC0DE = end of func pattern
}

PatchBasic::PatchBasic(
	const std::wstring& pName,
	std::shared_ptr<ProcMem> tProc_Mem,
	std::uintptr_t tProc_origCodeAddr,
	size_t tProc_origCodeSize,
	std::uintptr_t* patch_newCodeAOBAddr)
	:
	PatchClass(pName, tProc_Mem, tProc_origCodeAddr, tProc_origCodeSize, patch_newCodeAOBAddr) {}

void PatchBasic::patch()
{
	std::cout << patchName.c_str() << ": PatchBasic patch() called" << std::endl;
	tProcess_Mem->writeMemoryArray<std::uint8_t>(tProcess_OrigCodeAddr, reinterpret_cast<std::uint8_t*>(patch_NewCodeAddr), patch_NewCodeSize);
	isActive = true;
}

void PatchBasic::unpatch()
{
	std::cout << patchName.c_str() << ": PatchBasic unpatch() called" << std::endl;
	tProcess_Mem->writeMemoryArray<std::uint8_t>(tProcess_OrigCodeAddr, patch_OrigCodeRawCopy.data(), tProcess_OrigCodeSize);
	isActive = false;
}

PatchWithTrampoline::PatchWithTrampoline(
	const std::wstring& pName,
	std::shared_ptr<ProcMem> tProc_Mem,
	std::uintptr_t tProc_origCodeAddr,
	size_t tProc_origCodeSize,
	std::uintptr_t* patch_newCodeAOBAddr,
	std::uintptr_t tProc_CodeCaveAddr)
	:
	PatchClass(pName, tProc_Mem, tProc_origCodeAddr, tProc_origCodeSize, patch_newCodeAOBAddr), tProcess_CodeCaveAddr(tProc_CodeCaveAddr)
{
	if (tProc_origCodeSize < 5)
	{
		printf("%s: tPatch_origCodeSize is less then 5! Aborting.", pName.c_str());
		return;
	}
	tProcess_patchTrampolineCode.resize(tProc_origCodeSize);
	tProcess_patchTrampolineCode.assign(tProcess_OrigCodeSize, 0x90);
	std::uint32_t jumpDist = tProcess_CodeCaveAddr - tProcess_OrigCodeAddr - 5;
	tProcess_patchTrampolineCode[0] = 0xE9;
	std::memcpy(&tProcess_patchTrampolineCode[1], &jumpDist, 4);
}

bool PatchWithTrampoline::formatNextRelative(std::uintptr_t valueToInsert)
{
	int result = asmPatchUtils_formatNextRelative(patch_NewCodeAddr, tProcess_CodeCaveAddr, valueToInsert);
	printf("formatNextRelative func with param: %llx, result: %d\n", valueToInsert, result);
	return result == 1;
}

bool PatchWithTrampoline::hasUnassignedRelatives()
{
	return asmPatchUtils_findPattern32(patch_NewCodeAddr, 0xDEADBEEF) != -1;
}

void PatchWithTrampoline::patch()
{
	if (hasUnassignedRelatives())
	{
		printf("The patch has unassigned relatives left! Aborting patch().\n");
		return;
	}
	std::cout << "PatchWithTrampoline patch() called" << std::endl;
	tProcess_Mem->writeMemoryArray<std::uint8_t>(tProcess_CodeCaveAddr, reinterpret_cast<std::uint8_t*>(patch_NewCodeAddr), patch_NewCodeSize); // insert patch to code cave
	printf("Assigned code cave addr: 0x%llx\n", tProcess_CodeCaveAddr);

	tProcess_Mem->writeMemoryArray<std::uint8_t>(tProcess_OrigCodeAddr, tProcess_patchTrampolineCode.data(), tProcess_OrigCodeSize); // patch orig code with trampoline

	isActive = true;
}

void PatchWithTrampoline::unpatch()
{
	std::cout << "PatchWithTrampoline unpatch() called" << std::endl;
	tProcess_Mem->writeMemoryArray<std::uint8_t>(tProcess_OrigCodeAddr, patch_OrigCodeRawCopy.data(), tProcess_OrigCodeSize); // orig code

	std::vector<std::uint8_t> cleanup(patch_NewCodeSize, 0);
	tProcess_Mem->writeMemoryArray<std::uint8_t>(tProcess_CodeCaveAddr, cleanup.data(), patch_NewCodeSize); // clear code cave
	isActive = false;
}

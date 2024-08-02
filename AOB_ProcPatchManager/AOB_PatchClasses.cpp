#include "AOB_PatchClasses.h"

PatchClass::PatchClass(
	const std::string& pName,
	std::shared_ptr<ProcMem> tProc_Mem,
	std::uintptr_t tProc_origCodeAddr,
	size_t tProc_origCodeSize,
	std::uintptr_t* patch_newCodeAOBAddr)
	:
	patchName(pName),
	m_tProcess_Mem(tProc_Mem),
	m_tProcess_OrigCodeAddr(tProc_origCodeAddr),
	m_tProcess_OrigCodeSize(tProc_origCodeSize),
	m_patch_NewCodeAddr(patch_newCodeAOBAddr),
	isActive(false)
{
	m_patch_OrigCodeRawCopy = tProc_Mem->readMemoryArray<std::uint8_t>(tProc_origCodeAddr, m_tProcess_OrigCodeSize);
	m_patch_NewCodeSize = asmPatchUtils_findPattern32(m_patch_NewCodeAddr, 0xDEADC0DE); // 0xDEADC0DE = end of func pattern
}

PatchBasic::PatchBasic(
	const std::string& pName,
	std::shared_ptr<ProcMem> tProc_Mem,
	std::uintptr_t tProc_origCodeAddr,
	size_t tProc_origCodeSize,
	std::uintptr_t* patch_newCodeAOBAddr)
	:
	PatchClass(pName, tProc_Mem, tProc_origCodeAddr, tProc_origCodeSize, patch_newCodeAOBAddr) {}

void PatchBasic::patch()
{
	std::cout << patchName.c_str() << ": PatchBasic patch() called" << std::endl;
	m_tProcess_Mem->writeMemoryArray<std::uint8_t>(m_tProcess_OrigCodeAddr, reinterpret_cast<std::uint8_t*>(m_patch_NewCodeAddr), m_patch_NewCodeSize);
	isActive = true;
}

void PatchBasic::unpatch()
{
	std::cout << patchName.c_str() << ": PatchBasic unpatch() called" << std::endl;
	m_tProcess_Mem->writeMemoryArray<std::uint8_t>(m_tProcess_OrigCodeAddr, m_patch_OrigCodeRawCopy.data(), m_tProcess_OrigCodeSize);
	isActive = false;
}

PatchWithTrampoline::PatchWithTrampoline(
	const std::string& pName,
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
	tProcess_patchTrampolineCode.assign(m_tProcess_OrigCodeSize, 0x90);
	std::uint32_t jumpDist = tProcess_CodeCaveAddr - m_tProcess_OrigCodeAddr - 5;
	tProcess_patchTrampolineCode[0] = 0xE9;
	std::memcpy(&tProcess_patchTrampolineCode[1], &jumpDist, 4);
}

bool PatchWithTrampoline::formatNextRelative(std::uintptr_t valueToInsert)
{
	int result = asmPatchUtils_formatNextRelative(m_patch_NewCodeAddr, tProcess_CodeCaveAddr, valueToInsert);
	printf("formatNextRelative func with param: %llx, result: %d\n", valueToInsert, result);
	return result == 1;
}

bool PatchWithTrampoline::hasUnassignedRelatives()
{
	return asmPatchUtils_findPattern32(m_patch_NewCodeAddr, 0xDEADBEEF) <= m_patch_NewCodeSize;
}

void PatchWithTrampoline::patch()
{
	if (hasUnassignedRelatives())
	{
		printf("The patch %s has unassigned relatives left! Aborting patch().\n", patchName.c_str());
		return;
	}
	std::cout << patchName.c_str() << "PatchWithTrampoline.patch() called" << std::endl;
	m_tProcess_Mem->writeMemoryArray<std::uint8_t>(tProcess_CodeCaveAddr, reinterpret_cast<std::uint8_t*>(m_patch_NewCodeAddr), m_patch_NewCodeSize); // insert patch to code cave
	printf("Assigned code cave addr: 0x%llx\n", tProcess_CodeCaveAddr);

	m_tProcess_Mem->writeMemoryArray<std::uint8_t>(m_tProcess_OrigCodeAddr, tProcess_patchTrampolineCode.data(), m_tProcess_OrigCodeSize); // patch orig code with trampoline

	isActive = true;
}

void PatchWithTrampoline::unpatch()
{
	std::cout << "PatchWithTrampoline unpatch() called" << std::endl;
	m_tProcess_Mem->writeMemoryArray<std::uint8_t>(m_tProcess_OrigCodeAddr, m_patch_OrigCodeRawCopy.data(), m_tProcess_OrigCodeSize); // orig code

	std::vector<std::uint8_t> cleanup(m_patch_NewCodeSize, 0);
	m_tProcess_Mem->writeMemoryArray<std::uint8_t>(tProcess_CodeCaveAddr, cleanup.data(), m_patch_NewCodeSize); // clear code cave
	isActive = false;
}

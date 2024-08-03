#pragma once
#include <memory>
#include <iostream>
#include <ProcMem.h>


extern "C" int asmPatchUtils_findPattern32(std::uintptr_t * func, std::uintptr_t pattern);
extern "C" int asmPatchUtils_formatNextRelative(std::uintptr_t * funcToInject, std::uintptr_t funcInjectionAddr, std::uintptr_t valueToCalc);

class PatchClass
{
protected:
	std::shared_ptr<ProcMem>	m_tProcess_Mem;
	std::uintptr_t				m_tProcess_OrigCodeAddr;
	size_t						m_tProcess_OrigCodeSize;
	std::vector<uint8_t>		m_patch_OrigCodeRawCopy;
	std::uintptr_t*				m_patch_NewCodeAddr;
	size_t						m_patch_NewCodeSize;
	bool						isActive;
public:
	std::string					patchName;
	
public:
	PatchClass(
		const std::string& pName,
		std::shared_ptr<ProcMem> tProc_Mem,
		std::uintptr_t tProc_origCodeAddr,
		size_t tProc_origCodeSize,
		std::uintptr_t* patch_newCodeAOBAddr);

	virtual ~PatchClass() = default;

	virtual void patch() = 0;
	virtual void unpatch() = 0;
};

class PatchBasic : public PatchClass
{
public:
	PatchBasic(
		const std::string& pName,
		std::shared_ptr<ProcMem> tProc_Mem,
		std::uintptr_t tProc_origCodeAddr,
		size_t tProc_origCodeSize,
		std::uintptr_t* patch_newCodeAOBAddr);

	void patch() override;
	void unpatch() override;
};

class PatchWithTrampoline : public PatchClass
{
public:
	std::uintptr_t tProcess_CodeCaveAddr;
	std::vector<uint8_t> tProcess_patchTrampolineCode;
public:
	PatchWithTrampoline(
		const std::string& pName,
		std::shared_ptr<ProcMem> tProc_Mem,
		std::uintptr_t tProc_origCodeAddr,
		size_t tProc_origCodeSize,
		std::uintptr_t* patch_newCodeAOBAddr,
		std::uintptr_t tProc_CodeCaveAddr);

	bool formatNextRelative(std::uintptr_t valueToInsert);
	bool hasUnassignedRelatives();

	void patch() override;
	void unpatch() override;

};
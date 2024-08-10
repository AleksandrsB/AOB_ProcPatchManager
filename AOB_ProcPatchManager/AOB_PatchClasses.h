#pragma once
#include <memory>
#include <iostream>
#include <ProcMem.h>


extern "C" int asmPatchUtils_findPattern32(std::uint8_t * func, std::uintptr_t pattern);
extern "C" int asmPatchUtils_formatNextRelative(std::uint8_t * funcToInject, std::uintptr_t funcInjectionAddr, std::uintptr_t valueToCalc);
extern "C" int asmPatchUtils_formatNextAbsolute(std::uint8_t * funcToInject, std::uintptr_t funcInjectionAddr, std::uintptr_t valueToCalc);

class PatchClass
{
protected:
	std::shared_ptr<ProcMem>	remote_procMem;
	std::uintptr_t				remote_origCodeAddr;
	std::size_t					remote_origCodeSize;
	std::vector<std::uint8_t>	local_origCodeRawCopy;
	std::uint8_t*				local_newCodeAddr;
	std::size_t					local_newCodeSize;

	bool						isActive;
public:
	std::string					patchName;

public:
	PatchClass(const std::string& pName, std::shared_ptr<ProcMem> pMem);
	virtual ~PatchClass() = default;

	virtual PatchClass& setOrigCodeInfo(const ModuleOffset& remote_addr, size_t remote_size);
	virtual PatchClass& setNewCodeInfo(std::uint8_t* local_newCodeAddr);

	virtual void patch() = 0;
	virtual void testPatch() = 0;
	virtual void unpatch() = 0;
};

class PatchBasic : public PatchClass
{
public:
	PatchBasic(const std::string& pName, std::shared_ptr<ProcMem> pMem) : PatchClass(pName, pMem) {}

	void patch() override;
	void unpatch() override;
};

class PatchWithTrampoline : public PatchClass
{
public:
	std::uintptr_t remote_CodeCaveAddr = 0;
	std::vector<uint8_t> remote_patchTrampolineCode;
public:
	PatchWithTrampoline(const std::string& pName, std::shared_ptr<ProcMem> pMem) : PatchClass(pName, pMem) {}

	PatchWithTrampoline& setOrigCodeInfo(const ModuleOffset& remote_addr, size_t remote_size) override;
	PatchWithTrampoline& setNewCodeInfo(std::uint8_t* local_newCodeAddr) override;
	PatchWithTrampoline& setCodeCaveInfo(const ModuleOffset& remote_addr);
	

	PatchWithTrampoline& formatNextRelative(const ModuleOffset& remote_addrToInsert);
	PatchWithTrampoline& formatNextAbsolute(const ModuleOffset& remote_addrToInsert);

	bool hasUnassignedRelatives();

	void patch() override;
	void testPatch();
	void unpatch() override;

};
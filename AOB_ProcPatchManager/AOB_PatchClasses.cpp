#include "AOB_PatchClasses.h"

PatchClass::PatchClass(const std::string& pName, std::shared_ptr<ProcMem> pMem)
{
	patchName = pName;
	remote_procMem = pMem;
	isActive = false;
}

PatchClass& PatchClass::setOrigCodeInfo(const ModuleOffset& remote_addr, size_t remote_size)
{
	// get absolute remote proc address from (moduleName + offset)
	remote_origCodeAddr = remote_procMem->getModuleBaseAddress(remote_addr.moduleName) + remote_addr.moduleOffset;
	remote_origCodeSize = remote_size;

	// copy original code from remote proc to local
	local_origCodeRawCopy = remote_procMem->readMemoryArray<std::uint8_t>(remote_origCodeAddr, remote_size);
	return *this;
}

PatchClass& PatchClass::setNewCodeInfo(std::uint8_t* newCodeAddr)
{
	local_newCodeAddr = newCodeAddr;
	local_newCodeSize = asmPatchUtils_findPattern32(local_newCodeAddr, 0xDEADC0DE);
	return *this;
}

void PatchBasic::patch()
{
	remote_procMem->
		writeMemoryArray<std::uint8_t>(
			remote_origCodeAddr,
			local_newCodeAddr,
			local_newCodeSize
			);

	isActive = true;
}

void PatchBasic::unpatch()
{
	remote_procMem->
		writeMemoryArray<std::uint8_t>(
			remote_origCodeAddr,
			local_origCodeRawCopy.data(),
			remote_origCodeSize
			);

	isActive = false;
}

PatchWithTrampoline& PatchWithTrampoline::setOrigCodeInfo(const ModuleOffset& remote_addr, size_t remote_size)
{

	// remote_origCodeAddr = calc absolute address
	PatchClass::setOrigCodeInfo(remote_addr, remote_size);

	if (remote_size < 5)
	{
		throw std::runtime_error(std::string("Failed: The patch") + patchName + std::string(" has origCodeSize less then 5!"));
	}

	// build jumpToCodeCave jmp instruction: (5 bytes) + NOPs to cover remote original instruction
	remote_patchTrampolineCode.assign(remote_size, 0x90);
	remote_patchTrampolineCode[0] = 0xE9;

	std::uint32_t jumpDist = remote_CodeCaveAddr - remote_origCodeAddr - 5;
	std::memcpy(&remote_patchTrampolineCode[1], &jumpDist, 4);
	return *this;
}

PatchWithTrampoline& PatchWithTrampoline::setNewCodeInfo(std::uint8_t* local_newCodeAddr)
{
	PatchClass::setNewCodeInfo(local_newCodeAddr);
	return *this;
}

PatchWithTrampoline& PatchWithTrampoline::setCodeCaveInfo(const ModuleOffset& remote_addr)
{
	remote_CodeCaveAddr = remote_procMem->getModuleBaseAddress(remote_addr.moduleName) + remote_addr.moduleOffset;
	return *this;
}

PatchWithTrampoline& PatchWithTrampoline::formatNextRelative(const ModuleOffset& remote_addrToInsert)
{
	std::uintptr_t valueToInsert = remote_procMem->getModuleBaseAddress(remote_addrToInsert.moduleName) + remote_addrToInsert.moduleOffset;

	int result = asmPatchUtils_formatNextRelative(local_newCodeAddr, remote_CodeCaveAddr, valueToInsert);
	if(result==-1)
		throw std::runtime_error(std::string("Failed: The patch") + patchName + std::string(".formatNextRelative returned -1!"));
	return *this;
}

bool PatchWithTrampoline::hasUnassignedRelatives()
{
	return asmPatchUtils_findPattern32(local_newCodeAddr, 0xDEADBEEF) <= local_newCodeSize;
}

void PatchWithTrampoline::patch()
{
	if (hasUnassignedRelatives())
	{
		throw std::runtime_error(std::string("Failed: The patch") + patchName+ std::string(" has unassigned relatives left!"));
	}
	if (!remote_CodeCaveAddr || !local_newCodeAddr || !local_newCodeSize)
	{
		throw std::runtime_error(std::string("Failed: The patch") + patchName + std::string(" has unassigned data!"));
	}
	// insert patch to code cave
	remote_procMem->
		writeMemoryArray<std::uint8_t>(
			remote_CodeCaveAddr,
			local_newCodeAddr,
			local_newCodeSize);


	// patch orig code with trampoline to code cave
	remote_procMem->
		writeMemoryArray<std::uint8_t>(
			remote_origCodeAddr,
			remote_patchTrampolineCode.data(),
			remote_patchTrampolineCode.size());

	isActive = true;
}

void PatchWithTrampoline::unpatch()
{
	// orig code
	remote_procMem->
		writeMemoryArray<std::uint8_t>(
			remote_origCodeAddr,
			local_origCodeRawCopy.data(),
			local_origCodeRawCopy.size());

	// clear code cave
	std::vector<std::uint8_t> cleanup(local_newCodeSize, 0);
	remote_procMem->
		writeMemoryArray<std::uint8_t>(
			remote_CodeCaveAddr,
			cleanup.data(),
			local_newCodeSize);

	isActive = false;
}
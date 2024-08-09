#pragma once
#include "AOB_PatchClasses.h"
/*
* TODO:
* 1) Chain functions while building patch - make them independent or not?
* 2) Remove all console lines
* 3) Make eLastError more usable
* 
*/
#define not nameConstructor

enum class eLastError
{
	OK,
	TARGET_PROC_NOT_FOUND,
	TARGET_MODULE_NOT_FOUND
};

class AOB_PatchManager
{
private:
	std::shared_ptr<ProcMem> m_targetProcess;
	std::vector<std::unique_ptr<PatchClass>> m_patches;

public:
	eLastError lastError;
public:

#ifdef nameConstructor
	AOB_PatchManager(const std::wstring& procExeName)
	{
		m_targetProcess = std::make_shared<ProcMem>(procExeName);
		if (!m_targetProcess || !m_targetProcess->getConnectedState())
		{
			lastError = eLastError::TARGET_PROC_NOT_FOUND;
			return;
		}
		lastError = eLastError::OK;
	}
#else
	AOB_PatchManager(std::shared_ptr<ProcMem> procMem)
	{
		m_targetProcess = procMem;
		if (!m_targetProcess || !m_targetProcess->getConnectedState())
		{
			lastError = eLastError::TARGET_PROC_NOT_FOUND;
			return;
		}
		lastError = eLastError::OK;
	}
#endif // nameConstructor
	virtual ~AOB_PatchManager() = default;

	template<typename T>
	_NODISCARD T& createNewPatch(const std::string& patchName)
	{
		static_assert(std::is_base_of<PatchClass, T>::value, "T must be derived from PatchClass");

		// check already existed name
		auto patchCheck = findPatchByName(patchName);
		if (patchCheck)
		{
			return (T&)(*patchCheck); // return reference for next chain functions
		}

		// create new patch and return reference for next chain functions
		std::unique_ptr<T> patch = std::make_unique<T>(patchName, m_targetProcess);
		T& ref = *patch;
		m_patches.push_back(std::move(patch));
		return ref;
	}

	// Your method to find the patch by name
	PatchClass* findPatchByName(const std::string& name) 
	{
		auto it = std::find_if(
			m_patches.begin(),
			m_patches.end(),
			[&name](const std::unique_ptr<PatchClass>& patch) 
			{
				return patch->patchName == name;
			});

		if (it != m_patches.end())
		{
			return it->get();
		}

		return nullptr; // Return nullptr if not found
	}

	void applyPatch(const std::string& patchName)
	{
		PatchClass* patch = findPatchByName(patchName);
		if (patch) 
		{
			patch->patch();
		}
		else throw std::runtime_error("Patch Not Found!");
	}

	void revertPatch(const std::string& patchName)
	{
		PatchClass* patch = findPatchByName(patchName);
		if (patch)
		{
			patch->unpatch();
		}
		else throw std::runtime_error("Patch Not Found!");
	}
};


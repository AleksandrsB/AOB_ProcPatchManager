#pragma once
#include "AOB_PatchClasses.h"
#include <unordered_map>

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
	std::unordered_map<std::string, std::unique_ptr<PatchClass>> m_patches;

public:
	eLastError lastError;
public:

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
	virtual ~AOB_PatchManager() = default;

	template<typename T>
	_NODISCARD T& createNewPatch(const std::string& patchName)
	{
		static_assert(std::is_base_of<PatchClass, T>::value, "T must be derived from PatchClass");

		// check already existed name
		auto it = m_patches.find(patchName);
		if (it != m_patches.end())
		{
			return dynamic_cast<T&>(*it->second); // return reference for next chain functions
		}

		// create new patch and return reference for next chain functions
		std::unique_ptr<T> patch = std::make_unique<T>(patchName, m_targetProcess);
		T& ref = *patch;
		m_patches.emplace(patchName, std::move(patch));
		return ref;
	}


	void applyPatch(const std::string& patchName)
	{
		if (m_patches.find(patchName) != m_patches.end())
		{
			m_patches[patchName]->patch();
		}
	}

	void revertPatch(const std::string& patchName)
	{
		if (m_patches.find(patchName) != m_patches.end())
		{
			m_patches[patchName]->unpatch();
		}
	}
};


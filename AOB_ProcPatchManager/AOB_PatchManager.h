#pragma once
#include "AOB_PatchClasses.h"

class AOB_PatchManager
{
private:
	std::shared_ptr<ProcMem> m_targetProcess;
public:
	AOB_PatchManager(const std::wstring& procExeName)
	{
		m_targetProcess = std::make_shared<ProcMem>(procExeName);
	}
	virtual ~AOB_PatchManager() = default;


};


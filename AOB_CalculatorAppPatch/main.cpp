#include "main.h"


int main()
{
	std::shared_ptr<ProcMem>procMem(new ProcMem(L"Tutorial-x86_64.exe"));
	return 0;
}

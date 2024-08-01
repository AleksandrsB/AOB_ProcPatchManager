#include <iostream>
#include "../ProcMem/ProcMem/ProcMem.cpp"

int main()
{
	std::shared_ptr<ProcMem>procMem(new ProcMem(L"CalculatorApp.exe"));
	std::cout << "Entry" << std::endl;
	return 0;
}
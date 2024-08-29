# AOB Injection Patch Tool

## Overview

This project demonstrates an advanced technique for patching external Windows processes using **AOB (Array of Bytes) Injection**. The goal is to modify the behavior of external applications at runtime by replacing specific code patterns in their memory with custom instructions.

As an example, this project includes a demonstration on how to modify the built-in **CalculatorApp.exe** in Windows 10 to change the **MUL (multiplication)** operation to **ADD (addition)**.

## Example

- **Target Application**: `CalculatorApp.exe` (built-in Windows 10 Calculator).
- **Original Operation**: Multiplication (MUL).
- **Modified Operation**: Addition (ADD).

This example demonstrates how to patch the byte pattern corresponding to the multiplication operation and replace it with a custom pattern for addition.

## Disclaimer

This tool is intended for educational purposes only. Unauthorized modification of software is illegal and unethical. Please use this tool responsibly and only with applications for which you have the legal right to modify.

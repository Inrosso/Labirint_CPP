#include "console.h"

#include <clocale>
#include <cstdlib>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

void InitConsole() {
#ifdef _WIN32
	// локаль Windows
	// Перевод и ввода, и вывода консоли на одну кодировку CP1251.
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	std::setlocale(LC_ALL, "Russian");
#else
	// Локаль на Linux/macOS достаточно включить системную.
	std::setlocale(LC_ALL, "");
#endif
}

void ClearScreen() {
	// На Windows очистка делается командой "cls", на других системах — "clear".
#ifdef _WIN32
	std::system("cls");
#else
	std::system("clear");
#endif
}

void PauseForEnter() {
	std::cout << "\nНажмите Enter, чтобы продолжить...";
	// Перед вызовом этой функции буфер ввода всегда очищен от лишнего,
	// поэтому getline дождётся ровно одного нажатия Enter.
	std::string dummy;
	std::getline(std::cin, dummy);
}
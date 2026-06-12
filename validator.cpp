#include "validator.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "console.h"
#include "exceptions.h"

std::string Validator::Trim(const std::string& text) {
    std::size_t start = 0;
    std::size_t end = text.size();
    while (start < end && (text[start] == ' ' || text[start] == '\t' ||
        text[start] == '\r' || text[start] == '\n')) {
        ++start;
    }
    while (end > start && (text[end - 1] == ' ' || text[end - 1] == '\t' ||
        text[end - 1] == '\r' || text[end - 1] == '\n')) {
        --end;
    }
    return text.substr(start, end - start);
}

int Validator::ReadMenuChoice(const std::string& menu_text, int min, int max) {
    std::string error;
    while (true) {
        ClearScreen();           // перерисовываем экран заново, чтобы меню не уезжало
        std::cout << menu_text;
        if (!error.empty()) {
            std::cout << "\n[!] " << error << "\n";
        }
        std::cout << "Ваш выбор: ";

        std::string line;
        if (!std::getline(std::cin, line)) {
            return min;  // ввод закончился — выходим со значением по умолчанию
        }
        line = Trim(line);

        if (line.empty()) {
            error = "Пустой ввод. Введите число от " + std::to_string(min) + " до " +
                std::to_string(max) + ".";
            continue;
        }
        // Проверяем, что в строке ровно одно целое число и ничего лишнего.
        std::istringstream stream(line);
        int value = 0;
        char extra = 0;
        if (!(stream >> value) || (stream >> extra)) {
            error = "Нужно одно число от " + std::to_string(min) + " до " +
                std::to_string(max) + ".";
            continue;
        }
        if (value < min || value > max) {
            error = "Число должно быть от " + std::to_string(min) + " до " +
                std::to_string(max) + ".";
            continue;
        }
        return value;
    }
}

bool Validator::ReadYesNo(const std::string& question) {
    std::string error;
    while (true) {
        ClearScreen();  // перерисовываем экран, чтобы вопрос не уезжал вниз
        std::cout << question << "\n";
        if (!error.empty()) {
            std::cout << "[!] " << error << "\n";
        }
        std::cout << "Ваш ответ (Y — да, N — нет): ";

        std::string line;
        if (!std::getline(std::cin, line)) {
            return false;  // ввод закончился — считаем ответ «нет»
        }
        line = Trim(line);
        if (line.size() == 1) {
            char answer = static_cast<char>(
                std::tolower(static_cast<unsigned char>(line[0])));
            if (answer == 'y') return true;
            if (answer == 'n') return false;
        }
        error = "Введите ровно одну букву: Y (да) или N (нет).";
    }
}

std::string Validator::ReadLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

bool Validator::FileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

void Validator::ValidateMaze(const Maze& maze, const Point& player,
    const Point& treasure, const Point& exit) {
    if (maze.GetRows() < 3 || maze.GetCols() < 3) {
        throw InvalidMazeException("Лабиринт слишком маленький.");
    }
    if (player.GetRow() < 0) {
        throw InvalidMazeException("В лабиринте нет стартовой клетки игрока (P).");
    }
    if (treasure.GetRow() < 0) {
        throw InvalidMazeException("В лабиринте нет сокровища (T).");
    }
    if (exit.GetRow() < 0) {
        throw InvalidMazeException("В лабиринте нет выхода (E).");
    }
    if (player == treasure || player == exit || treasure == exit) {
        throw InvalidMazeException("Игрок, сокровище и выход должны быть в разных клетках.");
    }
    if (!maze.PathExists(player, treasure)) {
        throw InvalidMazeException("До сокровища нельзя добраться.");
    }
    if (!maze.PathExists(player, exit)) {
        throw InvalidMazeException("До выхода нельзя добраться.");
    }
}
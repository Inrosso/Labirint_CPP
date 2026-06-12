#pragma once

#include <stdexcept>
#include <string>

// Базовый класс исключений игры. Наследуется от стандартного std::runtime_error,
class MazeException : public std::runtime_error {
public:
    explicit MazeException(const std::string& message)
        : std::runtime_error(message) {}
};

// Попытка взять элемент из пустой структуры (стек/очередь/список).
class EmptyStructureException : public MazeException {
public:
    explicit EmptyStructureException(const std::string& message)
        : MazeException(message) {
    }
};

// Некорректный ввод пользователя или выход за границы.
class InvalidInputException : public MazeException {
public:
    explicit InvalidInputException(const std::string& message)
        : MazeException(message) {
    }
};

// Ошибка при работе с файлом (не открылся, не найден и т. п.).
class FileException : public MazeException {
public:
    explicit FileException(const std::string& message)
        : MazeException(message) {
    }
};

// Некорректная карта лабиринта (нет игрока, нет пути и т. п.).
class InvalidMazeException : public MazeException {
public:
    explicit InvalidMazeException(const std::string& message)
        : MazeException(message) {
    }
};
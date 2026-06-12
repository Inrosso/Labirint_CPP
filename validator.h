#pragma once

#include <string>

#include "maze.h"
#include "point.h"

// Класс проверок и безопасного ввода. Все методы статические,

// объект создавать не нужно — это набор инструментов-помощников!
class Validator {
public:
    // Показать меню (menu_text) и прочитать корректный номер пункта [min; max].
    // Экран каждый раз очищается заново, поэтому меню не «уезжает» вниз,
    // а при ошибке снизу выводится пояснение. Пустой ввод тоже считается ошибкой.
    static int ReadMenuChoice(const std::string& menu_text, int min, int max);

    // Задать вопрос «да/нет». Принимает только Y или N (в любом регистре),
    // на всё остальное ругается и спрашивает заново. true = Y.
    static bool ReadYesNo(const std::string& question);

    // Прочитать строку целиком (например, имя файла или игрока).
    static std::string ReadLine(const std::string& prompt);

    // Убрать пробелы и переводы строк по краям строки.
    static std::string Trim(const std::string& text);

    // Проверить, существует ли файл.
    static bool FileExists(const std::string& filename);

    // Проверить корректность лабиринта. Бросает InvalidMazeException,
    // если что-то не так (нет игрока/сокровища/выхода, нет пути и т. п.).
    static void ValidateMaze(const Maze& maze, const Point& player,
        const Point& treasure, const Point& exit);
};
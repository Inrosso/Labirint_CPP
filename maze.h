#pragma once

#include <string>

#include "point.h"

class PointStack;  // предварительное объявление (полное определение — в containers.h)

// Класс лабиринта: хранит двумерную сетку символов и умеет с ней работать.
// Символы сетки: '#' — стена, ' ' — проход, 'E' — выход (по нему можно ходить).

// Игрок и сокровище в сетке не хранятся — это отдельные координаты в классе Game!
class Maze {
public:
    Maze();
    Maze(const Maze& other); // конструктор копирования
    Maze& operator=(const Maze& other); // оператор присваивания
    ~Maze(); // деструктор

    int GetRows() const { return rows_; }
    int GetCols() const { return cols_; }

    char GetCell(int row, int col) const; // бросает InvalidInputException за границей
    void SetCell(int row, int col, char symbol); // бросает InvalidInputException за границей
    bool InBounds(int row, int col) const;
    bool IsWall(int row, int col) const;  // клетка за границей считается стеной

    // Есть ли путь между двумя клетками (поиск в ширину на собственной очереди).
    bool PathExists(const Point& from, const Point& to) const;
    // Кратчайший путь: заполняет path клетками маршрута. false, если пути нет.
    bool FindPath(const Point& from, const Point& to, PointStack& path) const;

    // Построить лабиринт из текста; попутно найти позиции игрока (P), сокровища (T), выхода (E).
    void LoadFromText(const std::string& text, Point& player, Point& treasure, Point& exit);
    // Прочитать лабиринт из файла. Бросает FileException, если файл не открылся.
    void LoadFromFile(const std::string& filename, Point& player, Point& treasure,
        Point& exit);
    // Сгенерировать случайный проходимый лабиринт.
    void GenerateRandom(int rows, int cols, Point& player, Point& treasure, Point& exit);

private:
    char** grid_; // двумерный массив символов (собственное управление памятью)
    int rows_;
    int cols_;

    void Allocate(int rows, int cols); // выделить сетку
    void Free(); // освободить сетку
    void CopyFrom(const Maze& other); // глубокое копирование сетки

    // Расстояния от клетки from до всех остальных (BFS). -1 означает «недостижимо».
    // Возвращает выделенный массив; освобождать его нужно через FreeDistances.
    int** ComputeDistances(const Point& from) const;
    void FreeDistances(int** distances) const;
};
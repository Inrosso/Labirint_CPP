#include "maze.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

#include "containers.h"
#include "exceptions.h"

// Конструктор по умолчанию

Maze::Maze() : grid_(nullptr), rows_(0), cols_(0) {}

// Конструктор копирования

Maze::Maze(const Maze& other) : grid_(nullptr), rows_(0), cols_(0) {
    CopyFrom(other);
}

// Копирующее присваивание

Maze& Maze::operator=(const Maze& other) {
    if (this != &other) {
        Free();
        CopyFrom(other);
    }
    return *this;
}

Maze::~Maze() { Free(); }

void Maze::Allocate(int rows, int cols) {
    rows_ = rows;
    cols_ = cols;
    grid_ = new char* [rows_];
    for (int r = 0; r < rows_; ++r) {
        grid_[r] = new char[cols_];
        for (int c = 0; c < cols_; ++c) {
            grid_[r][c] = ' ';
        }
    }
}

void Maze::Free() {
    if (grid_ != nullptr) {
        for (int r = 0; r < rows_; ++r) {
            delete[] grid_[r];
        }
        delete[] grid_;
        grid_ = nullptr;
    }
    rows_ = 0;
    cols_ = 0;
}

void Maze::CopyFrom(const Maze& other) {
    if (other.grid_ == nullptr) {
        grid_ = nullptr;
        rows_ = 0;
        cols_ = 0;
        return;
    }
    Allocate(other.rows_, other.cols_);
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            grid_[r][c] = other.grid_[r][c];
        }
    }
}

bool Maze::InBounds(int row, int col) const {
    return row >= 0 && row < rows_ && col >= 0 && col < cols_;
}

char Maze::GetCell(int row, int col) const {
    if (!InBounds(row, col)) {
        throw InvalidInputException("Обращение к клетке за пределами лабиринта.");
    }
    return grid_[row][col];
}

void Maze::SetCell(int row, int col, char symbol) {
    if (!InBounds(row, col)) {
        throw InvalidInputException("Запись в клетку за пределами лабиринта.");
    }
    grid_[row][col] = symbol;
}

bool Maze::IsWall(int row, int col) const {
    if (!InBounds(row, col)) return true;  // за стенами лабиринта — тоже «стена»
    return grid_[row][col] == '#';
}

int** Maze::ComputeDistances(const Point& from) const {
    // Выделяем матрицу расстояний и заполняем её -1 (пока никуда не дошли).
    int** distances = new int* [rows_];
    for (int r = 0; r < rows_; ++r) {
        distances[r] = new int[cols_];
        for (int c = 0; c < cols_; ++c) {
            distances[r][c] = -1;
        }
    }

    // Поиск в ширину (BFS) с использованием собственной очереди координат.
    PointQueue queue;
    if (InBounds(from.GetRow(), from.GetCol()) && !IsWall(from.GetRow(), from.GetCol())) {
        distances[from.GetRow()][from.GetCol()] = 0;
        queue.Enqueue(from);
    }
    const int kDeltaRow[4] = { -1, 1, 0, 0 };  // вверх, вниз, влево, вправо
    const int kDeltaCol[4] = { 0, 0, -1, 1 };
    while (!queue.IsEmpty()) {
        Point current = queue.Dequeue();
        int r = current.GetRow();
        int c = current.GetCol();
        for (int i = 0; i < 4; ++i) {
            int next_row = r + kDeltaRow[i];
            int next_col = c + kDeltaCol[i];
            // Идём в соседа, если он в границах, не стена и ещё не посещён.
            if (InBounds(next_row, next_col) && !IsWall(next_row, next_col) &&
                distances[next_row][next_col] == -1) {
                distances[next_row][next_col] = distances[r][c] + 1;
                queue.Enqueue(Point(next_row, next_col));
            }
        }
    }
    return distances;
}

void Maze::FreeDistances(int** distances) const {
    if (distances == nullptr) return;
    for (int r = 0; r < rows_; ++r) {
        delete[] distances[r];
    }
    delete[] distances;
}

bool Maze::PathExists(const Point& from, const Point& to) const {
    if (!InBounds(to.GetRow(), to.GetCol())) return false;
    int** distances = ComputeDistances(from);
    bool reachable = distances[to.GetRow()][to.GetCol()] >= 0;
    FreeDistances(distances);
    return reachable;
}

bool Maze::FindPath(const Point& from, const Point& to, PointStack& path) const {
    path.Clear();
    if (!InBounds(to.GetRow(), to.GetCol())) return false;
    int** distances = ComputeDistances(from);
    if (distances[to.GetRow()][to.GetCol()] < 0) {
        FreeDistances(distances);
        return false;  // пути нет
    }
    // Восстанавливаем путь: идём от выхода к старту, каждый раз переходя
    // в соседнюю клетку, расстояние до которой на 1 меньше.
    const int kDeltaRow[4] = { -1, 1, 0, 0 };
    const int kDeltaCol[4] = { 0, 0, -1, 1 };
    Point current = to;
    while (current != from) {
        path.Push(current);
        int r = current.GetRow();
        int c = current.GetCol();
        for (int i = 0; i < 4; ++i) {
            int next_row = r + kDeltaRow[i];
            int next_col = c + kDeltaCol[i];
            if (InBounds(next_row, next_col) &&
                distances[next_row][next_col] == distances[r][c] - 1) {
                current = Point(next_row, next_col);
                break;
            }
        }
    }
    path.Push(from);
    FreeDistances(distances);
    return true;
}

void Maze::LoadFromText(const std::string& text, Point& player, Point& treasure,
    Point& exit) {
    Free();
    // Первый проход: считаем количество строк и максимальную длину строки.
    std::istringstream first_pass(text);
    std::string line;
    int rows = 0;
    int cols = 0;
    while (std::getline(first_pass, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();  // файлы из Windows
        if (line.empty()) continue;
        ++rows;
        if (static_cast<int>(line.size()) > cols) cols = static_cast<int>(line.size());
    }
    if (rows < 3 || cols < 3) {
        throw InvalidMazeException("Лабиринт слишком маленький (нужно минимум 3x3).");
    }

    Allocate(rows, cols);
    player = Point();
    treasure = Point();
    exit = Point();

    // Второй проход: заполняем сетку и запоминаем позиции P, T, E.
    std::istringstream second_pass(text);
    int r = 0;
    while (std::getline(second_pass, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        for (int c = 0; c < cols_; ++c) {
            // Если строка короче — недостающие клетки считаем стеной.
            char symbol = (c < static_cast<int>(line.size())) ? line[c] : '#';
            switch (symbol) {
            case '#':
                grid_[r][c] = '#';
                break;
            case 'P':
            case 'p':
                if (player.GetRow() >= 0) {
                    throw InvalidMazeException(
                        "В лабиринте больше одной стартовой клетки игрока (P).");
                }
                grid_[r][c] = ' ';
                player = Point(r, c);
                break;
            case 'T':
            case 't':
                if (treasure.GetRow() >= 0) {
                    throw InvalidMazeException("В лабиринте больше одного сокровища (T).");
                }
                grid_[r][c] = ' ';
                treasure = Point(r, c);
                break;
            case 'E':
            case 'e':
                if (exit.GetRow() >= 0) {
                    throw InvalidMazeException("В лабиринте больше одного выхода (E).");
                }
                grid_[r][c] = 'E';
                exit = Point(r, c);
                break;
            case '.':
            case ' ':
                grid_[r][c] = ' ';  // проход
                break;
            default:
                // Любой другой символ (цифры, посторонние буквы и т. п.) — ошибка.
                throw InvalidMazeException(
                    std::string("Недопустимый символ '") + symbol + "' в строке " +
                    std::to_string(r + 1) + ", позиция " + std::to_string(c + 1) +
                    ". Разрешены: # . пробел P T E.");
            }
        }
        ++r;
    }
}

void Maze::LoadFromFile(const std::string& filename, Point& player, Point& treasure,
    Point& exit) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw FileException("Не удалось открыть файл лабиринта: " + filename);
    }
    // Читаем файл построчно и складываем строки в один текст.
    std::string text;
    std::string line;
    while (std::getline(file, line)) {
        text += line;
        text += '\n';
    }
    LoadFromText(text, player, treasure, exit);
}

void Maze::GenerateRandom(int rows, int cols, Point& player, Point& treasure,
    Point& exit) {
    if (rows < 5) rows = 5;
    if (cols < 5) cols = 5;
    const int kMaxAttempts = 300;

    for (int attempt = 0; attempt < kMaxAttempts; ++attempt) {
        Free();
        Allocate(rows, cols);
        // Внешняя рамка — стены, внутри случайно расставляем стены и проходы.
        for (int r = 0; r < rows_; ++r) {
            for (int c = 0; c < cols_; ++c) {
                bool border = (r == 0 || c == 0 || r == rows_ - 1 || c == cols_ - 1);
                if (border) {
                    grid_[r][c] = '#';
                }
                else {
                    grid_[r][c] = (std::rand() % 100 < 28) ? '#' : ' ';  // примерно 28% стен
                }
            }
        }
        player = Point(1, 1);
        exit = Point(rows_ - 2, cols_ - 2);
        grid_[1][1] = ' ';
        grid_[rows_ - 2][cols_ - 2] = 'E';

        // Сокровище ставим в случайную свободную клетку, не совпадающую со стартом/выходом.
        bool placed = false;
        for (int tries = 0; tries < 200 && !placed; ++tries) {
            int rr = 1 + std::rand() % (rows_ - 2);
            int cc = 1 + std::rand() % (cols_ - 2);
            if (grid_[rr][cc] == ' ' && !(rr == 1 && cc == 1) &&
                !(rr == rows_ - 2 && cc == cols_ - 2)) {
                treasure = Point(rr, cc);
                placed = true;
            }
        }
        if (!placed) continue;

        // Проверяем, что лабиринт проходим: от старта есть путь и к сокровищу, и к выходу.
        if (PathExists(player, treasure) && PathExists(player, exit)) {
            return;  // подходящий лабиринт найден
        }
    }

    // Запасной вариант, если случайно не вышло: полностью открытое поле.
    Free();
    Allocate(rows, cols);
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            bool border = (r == 0 || c == 0 || r == rows_ - 1 || c == cols_ - 1);
            grid_[r][c] = border ? '#' : ' ';
        }
    }
    player = Point(1, 1);
    exit = Point(rows_ - 2, cols_ - 2);
    grid_[rows_ - 2][cols_ - 2] = 'E';
    treasure = Point(1, cols_ - 2);
}
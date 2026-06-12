#include "game.h"

#include <cctype>
#include <iostream>
#include <string>

#include "console.h"
#include "exceptions.h"
#include "validator.h"

Game::Game()
    : has_treasure_(false),
    steps_(0),
    level_(0),
    levels_completed_(0),
    total_steps_(0) {
}

void Game::SetupLevel(const Maze& maze, const Point& player, const Point& treasure,
    const Point& exit) {
    maze_ = maze;  // глубокое копирование (работает правило трёх в Maze)
    player_ = player;
    treasure_ = treasure;
    exit_point_ = exit;
    has_treasure_ = false;
    steps_ = 0;
    history_.Clear();
    ++level_;
}

bool Game::IsWon() const {
    // Уровень пройден, когда собрано сокровище и игрок стоит на выходе.
    return has_treasure_ && player_ == exit_point_;
}

bool Game::IsWallCell(int row, int col) const {
    // Клетка за пределами сетки стеной для отрисовки НЕ считается,
    // чтобы рамка рисовалась аккуратно.
    return maze_.InBounds(row, col) && maze_.GetCell(row, col) == '#';
}

bool Game::IsVisibleWall(int row, int col) const {
    // Рисуем стену, только если она граничит со свободным пространством

    // "Внутренности" сплошных стенных массивов не рисуем — иначе вместо
    if (!IsWallCell(row, col)) return false;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int nr = row + dr;
            int nc = col + dc;
            if (maze_.InBounds(nr, nc) && maze_.GetCell(nr, nc) != '#') {
                return true;  // рядом есть проход или выход — стену видно
            }
        }
    }
    return false;
}

char Game::WallGlyph(int row, int col) const {
    // Подбираем «палочку» по соседним ВИДИМЫМ стенам: получается рамка из | - +.
    bool up = IsVisibleWall(row - 1, col);
    bool down = IsVisibleWall(row + 1, col);
    bool left = IsVisibleWall(row, col - 1);
    bool right = IsVisibleWall(row, col + 1);
    bool vertical = up || down;
    bool horizontal = left || right;
    if (vertical && horizontal) return '+';  // угол или перекрёсток
    if (vertical) return '|'; // вертикальная стена
    if (horizontal) return '-'; // горизонтальная стена
    return '+'; // одиночная стена
}

void Game::PrintCell(int row, int col, bool on_path) const {
    Point cell(row, col);
    char symbol = maze_.GetCell(row, col);
    if (cell == player_) {
        std::cout << '@'; // игрок
    }
    else if (!has_treasure_ && cell == treasure_) {
        std::cout << '$'; // сокровище (пока не собрано)
    }
    else if (symbol == 'E') {
        std::cout << 'E'; // выход
    }
    else if (symbol == '#') {
        if (IsVisibleWall(row, col)) {
            std::cout << WallGlyph(row, col); // стена в виде палочки
        }
        else {
            std::cout << ' '; // «внутренность» стенного массива не рисуем
        }
    }
    else if (on_path) {
        std::cout << '*'; // клетка подсказанного пути
    }
    else {
        std::cout << ' '; // свободный проход
    }
}

void Game::Draw(const std::string& message) const {
    ClearScreen();
    std::cout << "=== ЛАБИРИНТ ===   Уровень: " << level_ << "   Шаги: " << steps_
        << "\n";
    std::cout << (has_treasure_ ? "Сокровище собрано! Идите к выходу (E)."
        : "Сначала найдите сокровище ($).")
        << "\n\n";

    for (int r = 0; r < maze_.GetRows(); ++r) {
        for (int c = 0; c < maze_.GetCols(); ++c) {
            PrintCell(r, c, false);
        }
        std::cout << "\n";
    }

    std::cout << "\n@ — вы    $ — сокровище    E — выход\n\n";
    std::cout << "Ход: W/A/S/D    Z — назад    H — подсказка    Q — меню\n\n";
    if (!message.empty()) {
        std::cout << ">> " << message << "\n";
    }
}

void Game::DrawWithPath(const PointStack& path) const {
    int rows = maze_.GetRows();
    int cols = maze_.GetCols();

    // Временная матрица пометок: какие клетки входят в подсказанный путь.
    bool** on_path = new bool* [rows];
    for (int r = 0; r < rows; ++r) {
        on_path[r] = new bool[cols];
        for (int c = 0; c < cols; ++c) on_path[r][c] = false;
    }

    // Копируем путь, чтобы пометить клетки, не разрушая исходный стек.
    PointStack copy = path;
    while (!copy.IsEmpty()) {
        Point p = copy.Pop();
        if (maze_.InBounds(p.GetRow(), p.GetCol())) {
            on_path[p.GetRow()][p.GetCol()] = true;
        }
    }

    ClearScreen();
    std::cout << "=== ПОДСКАЗКА (кратчайший путь отмечен '*') ===\n\n";
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            PrintCell(r, c, on_path[r][c]);
        }
        std::cout << "\n";
    }

    for (int r = 0; r < rows; ++r) delete[] on_path[r];
    delete[] on_path;
}

void Game::TryMove(int delta_row, int delta_col, std::string& message) {
    int next_row = player_.GetRow() + delta_row;
    int next_col = player_.GetCol() + delta_col;
    if (maze_.IsWall(next_row, next_col)) {
        message = "Там стена.";
        return;
    }
    history_.Push(player_); // запоминаем текущую позицию для отмены
    player_ = Point(next_row, next_col);
    ++steps_;
    if (!has_treasure_ && player_ == treasure_) {
        has_treasure_ = true;
        message = "Сокровище найдено! Теперь к выходу.";
    }
}

void Game::Undo(std::string& message) {
    // Пытаемся снять последнюю позицию со стека истории.
    // Если стек пуст, собственное исключение будет поймано здесь.
    try {
        player_ = history_.Pop();
        if (steps_ > 0) --steps_;
        message = "Шаг отменён.";
    }
    catch (const EmptyStructureException& error) {
        message = error.what();  // "Стек пуст: нечего извлекать."
    }
}

void Game::ShowHint(std::string& message) {
    Point target = has_treasure_ ? exit_point_ : treasure_;
    PointStack path;
    if (!maze_.FindPath(player_, target, path)) {
        message = "Путь не найден.";
        return;
    }
    DrawWithPath(path);
    PauseForEnter();
    message = "Подсказка показана.";
}

bool Game::Play() {
    std::string message = "Удачи!";
    while (true) {
        Draw(message);
        message.clear();

        std::cout << "Ваш ход: ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            return false;  // ввод закончился — выходим в меню
        }
        line = Validator::Trim(line);

        if (line.empty()) {
            message = "Пустой ввод. Введите одну букву: W/A/S/D/Z/H/Q.";
            continue;
        }
        if (line.size() > 1) {
            message = "Слишком длинный ввод. Нужна ровно одна команда (одна буква).";
            continue;
        }
        char command = static_cast<char>(std::tolower(static_cast<unsigned char>(line[0])));

        switch (command) {
        case 'w':
            TryMove(-1, 0, message);
            break;
        case 's':
            TryMove(1, 0, message);
            break;
        case 'a':
            TryMove(0, -1, message);
            break;
        case 'd':
            TryMove(0, 1, message);
            break;
        case 'z':
            Undo(message);
            break;
        case 'h':
            ShowHint(message);
            break;
        case 'q':
            return false;  // выход в меню
        default:
            message = "Неизвестная команда. Доступны: W/A/S/D, Z, H, Q.";
            break;
        }

        if (IsWon()) {
            ++levels_completed_;
            total_steps_ += steps_;
            Draw("Уровень пройден! Поздравляем!");
            return true;
        }
    }
}
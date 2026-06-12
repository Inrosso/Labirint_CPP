#pragma once

#include <string>

#include "containers.h"
#include "maze.h"
#include "point.h"

//  ласс игры. ’ранит состо€ние одного прохождени€ (композици€: внутри лежат
// лабиринт Maze и стек истории ходов PointStack) и управл€ет игровым циклом.
class Game {
public:
    Game();

    // ѕодготовить новый уровень: скопировать лабиринт и расставить объекты.
    void SetupLevel(const Maze& maze, const Point& player, const Point& treasure,
        const Point& exit);

    // —ыграть один уровень. true Ч уровень пройден, false Ч игрок вышел в меню.
    bool Play();

    int GetLevelsCompleted() const { return levels_completed_; }
    int GetTotalSteps() const { return total_steps_; }

private:
    Maze maze_; // карта (хранитс€ по значению Ч глубока€ копи€)
    Point player_; // текуща€ позици€ игрока
    Point treasure_;  // позици€ сокровища
    Point exit_point_;  // позици€ выхода
    bool has_treasure_;  // собрано ли сокровище
    int steps_;  // шагов на текущем уровне
    int level_;  // номер текущего уровн€
    int levels_completed_; // сколько уровней пройдено за сессию
    int total_steps_; // суммарно шагов за сессию
    PointStack history_; // истори€ позиций дл€ кнопки Ђшаг назадї

    void Draw(const std::string& message) const;
    void DrawWithPath(const PointStack& path) const;
    void PrintCell(int row, int col, bool on_path) const; // нарисовать одну клетку
    bool IsWallCell(int row, int col) const;  // стена ли клетка (внутри сетки)
    bool IsVisibleWall(int row, int col) const; // рисуем ли эту стену
    char WallGlyph(int row, int col) const; // подобрать символ стены
    void TryMove(int delta_row, int delta_col, std::string& message);
    void Undo(std::string& message);
    void ShowHint(std::string& message);
    bool IsWon() const;
};
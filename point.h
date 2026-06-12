#pragma once

// Координата одной клетки лабиринта: строка (row) и столбец (col).
// Методы очень короткие, поэтому описаны прямо в классе.
class Point {
public:
    Point() : row_(-1), col_(-1) {}
    Point(int row, int col) : row_(row), col_(col) {}

    int GetRow() const { return row_; }
    int GetCol() const { return col_; }
    void SetRow(int row) { row_ = row; }
    void SetCol(int col) { col_ = col; }

    // Перегрузка операторов сравнения — удобно проверять,
    // дошёл ли игрок до сокровища или до выхода.
    bool operator==(const Point& other) const {
        return row_ == other.row_ && col_ == other.col_;
    }
    bool operator!=(const Point& other) const { return !(*this == other); }

private:
    int row_;
    int col_;
};
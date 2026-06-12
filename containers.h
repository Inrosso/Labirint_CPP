#pragma once

#include <string>

#include "point.h"

// В этом файле — собственные динамические структуры данных.

//  Стек координат (LIFO — последним пришёл, первым ушёл) 
// Применение: история ходов игрока (кнопка «шаг назад»).
class PointStack {
public:
    PointStack();
    PointStack(const PointStack& other); // конструктор копирования
    PointStack& operator=(const PointStack& other); // оператор присваивания
    ~PointStack(); // деструктор

    void Push(const Point& value); // положить элемент на вершину
    Point Pop(); // снять с вершины (бросает EmptyStructureException)
    bool IsEmpty() const;
    int Size() const;
    void Clear();

private:
    // Узел односвязного списка. Используем вложенный class (не struct).
    class Node {
    public:
        Node(const Point& value, Node* next) : value(value), next(next) {}
        Point value;
        Node* next;
    };

    Node* head_; // вершина стека
    int size_;

    void CopyFrom(const PointStack& other);
};

// Очередь координат (FIFO — первым пришёл, первым ушёл)
// Применение: поиск в ширину (BFS) для проверки проходимости и подсказки.
class PointQueue {
public:
    PointQueue();
    PointQueue(const PointQueue& other);
    PointQueue& operator=(const PointQueue& other);
    ~PointQueue();

    void Enqueue(const Point& value); // добавить в конец
    Point Dequeue(); // взять из начала (бросает EmptyStructureException)
    bool IsEmpty() const;
    void Clear();

private:
    class Node {
    public:
        Node(const Point& value, Node* next) : value(value), next(next) {}
        Point value;
        Node* next;
    };

    Node* head_; // начало очереди
    Node* tail_; // конец очереди
    int size_;

    void CopyFrom(const PointQueue& other);
};

// Запись о пройденной игре
class Record {
public:
    Record();
    Record(const std::string& name, int levels, int steps, const std::string& mode);

    std::string GetName() const { return name_; }
    int GetLevels() const { return levels_; }
    int GetSteps() const { return steps_; }
    std::string GetMode() const { return mode_; } // как был выбран первый уровень

private:
    std::string name_;
    int levels_;
    int steps_;
    std::string mode_;
};

// Список прохождений (односвязный список) 
// Хранит последние N записей; самая свежая — в начале списка.
// Умеет сохраняться в файл и читаться из файла.
class RecordList {
public:
    explicit RecordList(int capacity = 5);
    RecordList(const RecordList& other);
    RecordList& operator=(const RecordList& other);
    ~RecordList();

    void Add(const Record& record); // добавить (старые вытесняются)
    void SaveToFile(const std::string& filename) const; // бросает FileException
    void LoadFromFile(const std::string& filename); // если файла нет — просто пусто
    void Print() const;
    bool IsEmpty() const;

private:
    class Node {
    public:
        Node(const Record& value, Node* next) : value(value), next(next) {}
        Record value;
        Node* next;
    };

    Node* head_;
    int size_;
    int capacity_;

    void Clear();
    void CopyFrom(const RecordList& other);
};
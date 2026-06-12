#include "containers.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "exceptions.h"

// Стэк

// Конструктор по умолчанию

PointStack::PointStack() : head_(nullptr), size_(0) {}

// Конструктор копирования

PointStack::PointStack(const PointStack& other) : head_(nullptr), size_(0) {
    CopyFrom(other);
}

// Копирующее присваивание

PointStack& PointStack::operator=(const PointStack& other) {
    if (this != &other) {  // защита от самоприсваивания
        Clear();
        CopyFrom(other);
    }
    return *this;
}

// Деструктор

PointStack::~PointStack() { Clear(); }

void PointStack::Push(const Point& value) {
    head_ = new Node(value, head_);  // новый узел становится вершиной
    ++size_;
}

Point PointStack::Pop() {
    if (head_ == nullptr) {
        throw EmptyStructureException("Стек пуст: нечего извлекать.");
    }
    Node* node = head_;
    Point value = node->value;
    head_ = head_->next;
    delete node;
    --size_;
    return value;
}

// Проврка не пустоту
bool PointStack::IsEmpty() const { return head_ == nullptr; }

int PointStack::Size() const { return size_; }

void PointStack::Clear() {
    while (head_ != nullptr) {
        Node* node = head_;
        head_ = head_->next;
        delete node;
    }
    size_ = 0;
}

void PointStack::CopyFrom(const PointStack& other) {
    // Копируем узлы по порядку, чтобы вершина исходного стека
    // осталась вершиной и в копии.
    if (other.head_ == nullptr) return;
    head_ = new Node(other.head_->value, nullptr);
    Node* source = other.head_->next;
    Node* current = head_;
    while (source != nullptr) {
        current->next = new Node(source->value, nullptr);
        current = current->next;
        source = source->next;
    }
    size_ = other.size_;
}

// Очередь

// Конструктор по умолчанию

PointQueue::PointQueue() : head_(nullptr), tail_(nullptr), size_(0) {}

// Конструктор копирования

PointQueue::PointQueue(const PointQueue& other)
    : head_(nullptr), tail_(nullptr), size_(0) {
    CopyFrom(other);
}

// Копирующее присваивание

PointQueue& PointQueue::operator=(const PointQueue& other) {
    if (this != &other) {
        Clear();
        CopyFrom(other);
    }
    return *this;
}

PointQueue::~PointQueue() { Clear(); }

void PointQueue::Enqueue(const Point& value) {
    Node* node = new Node(value, nullptr);
    if (tail_ == nullptr) {
        head_ = node;  // очередь была пуста
        tail_ = node;
    }
    else {
        tail_->next = node;  // добавляем в конец
        tail_ = node;
    }
    ++size_;
}

Point PointQueue::Dequeue() {
    if (head_ == nullptr) {
        throw EmptyStructureException("Очередь пуста: нечего извлекать.");
    }
    Node* node = head_;
    Point value = node->value;
    head_ = head_->next;
    if (head_ == nullptr) {
        tail_ = nullptr;  // очередь опустела
    }
    delete node;
    --size_;
    return value;
}

bool PointQueue::IsEmpty() const { return head_ == nullptr; }

void PointQueue::Clear() {
    while (head_ != nullptr) {
        Node* node = head_;
        head_ = head_->next;
        delete node;
    }
    tail_ = nullptr;
    size_ = 0;
}

void PointQueue::CopyFrom(const PointQueue& other) {
    // Проходим исходную очередь от начала к концу и добавляем в свою.
    Node* source = other.head_;
    while (source != nullptr) {
        Enqueue(source->value);
        source = source->next;
    }
}

// Прохождения

Record::Record() : name_(""), levels_(0), steps_(0), mode_("") {}

Record::Record(const std::string& name, int levels, int steps,
    const std::string& mode)
    : name_(name), levels_(levels), steps_(steps), mode_(mode) {
}

// Таблица прохождений

RecordList::RecordList(int capacity) : head_(nullptr), size_(0), capacity_(capacity) {
    if (capacity_ < 1) capacity_ = 1;
}

RecordList::RecordList(const RecordList& other)
    : head_(nullptr), size_(0), capacity_(other.capacity_) {
    CopyFrom(other);
}

RecordList& RecordList::operator=(const RecordList& other) {
    if (this != &other) {
        Clear();
        capacity_ = other.capacity_;
        CopyFrom(other);
    }
    return *this;
}

RecordList::~RecordList() { Clear(); }

void RecordList::Add(const Record& record) {
    head_ = new Node(record, head_);  // новая запись — в начало
    ++size_;
    // Если записей стало больше вместимости, удаляем самую старую (она в конце).
    if (size_ > capacity_) {
        Node* current = head_;
        while (current->next->next != nullptr) {  // доходим до предпоследнего узла
            current = current->next;
        }
        delete current->next;
        current->next = nullptr;
        --size_;
    }
}

void RecordList::SaveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw FileException("Не удалось открыть файл для сохранения прохождений.");
    }
    // Каждая запись хранится в четырёх строках: имя, уровни, шаги, режим.
    // Так в имени и режиме можно использовать пробелы и русские буквы.
    Node* current = head_;
    while (current != nullptr) {
        file << current->value.GetName() << "\n"
            << current->value.GetLevels() << "\n"
            << current->value.GetSteps() << "\n"
            << current->value.GetMode() << "\n";
        current = current->next;
    }
}

void RecordList::LoadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return;  // файла ещё нет — это не ошибка, просто список пуст
    }
    Clear();
    Node* last = nullptr;
    std::string name;
    std::string levels_line;
    std::string steps_line;
    std::string mode;
    // Читаем записи по четыре строки.
    while (std::getline(file, name) && std::getline(file, levels_line) &&
        std::getline(file, steps_line) && std::getline(file, mode)) {
        int levels = std::atoi(levels_line.c_str());
        int steps = std::atoi(steps_line.c_str());
        // Добавляем в конец, чтобы сохранить порядок из файла (свежие записи — сверху).
        Node* node = new Node(Record(name, levels, steps, mode), nullptr);
        if (head_ == nullptr) {
            head_ = node;
        }
        else {
            last->next = node;
        }
        last = node;
        ++size_;
        if (size_ >= capacity_) break;  // больше вместимости не читаем
    }
}

void RecordList::Print() const {
    if (head_ == nullptr) {
        std::cout << "Прохождений пока нет.\n";
        return;
    }
    Node* current = head_;
    int index = 1;
    while (current != nullptr) {
        std::cout << index << ". " << current->value.GetName() << "\n";
        std::cout << "   режим: " << current->value.GetMode()
            << ";  уровней: " << current->value.GetLevels()
            << ";  шагов: " << current->value.GetSteps() << "\n";
        if (current->next != nullptr) {
            std::cout << "\n";  // пустая строка между записями для читаемости
        }
        current = current->next;
        ++index;
    }
}

// Проверка что не пуст

bool RecordList::IsEmpty() const { return head_ == nullptr; }

// Отчистка

void RecordList::Clear() {
    while (head_ != nullptr) {
        Node* node = head_;
        head_ = head_->next;
        delete node;
    }
    size_ = 0;
}

void RecordList::CopyFrom(const RecordList& other) {
    Node* source = other.head_;
    Node* last = nullptr;
    while (source != nullptr) {
        Node* node = new Node(source->value, nullptr);
        if (head_ == nullptr) {
            head_ = node;
        }
        else {
            last->next = node;
        }
        last = node;
        ++size_;
        source = source->next;
    }
}
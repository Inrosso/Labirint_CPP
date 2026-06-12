#include <clocale>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include "console.h"
#include "containers.h"
#include "exceptions.h"
#include "game.h"
#include "maze.h"
#include "point.h"
#include "validator.h"

namespace {

    const char* const kRecordsFile = "records.txt";
    const int kMinMazeSize = 5; // меньше — рамка займёт почти всё поле
    const int kMaxMazeSize = 30;// больше — не помещается на экран
    const std::size_t kMaxNameLength = 15;// лимит длины имени в прохождениях

    // Готовый (заранее построенный) уровень — «змейка».
    // Сокровище (T) лежит прямо на пути от старта (P) к выходу (E),
    // поэтому маршрут понятный и точно проходимый.
    // '#' — стена, '.' — проход, P — игрок, T — сокровище, E — выход.
    std::string PrebuiltMaze() {
        return
            "###########\n"
            "#P........#\n"
            "#########.#\n"
            "#T........#\n"
            "#.#########\n"
            "#.........#\n"
            "#########.#\n"
            "#........E#\n"
            "###########\n";
    }

    // На прощание показываем одного из пяти случайных котиков.
    void PrintRandomCat() {
        int cat = std::rand() % 5;
        switch (cat) {
        case 0:
            std::cout << "  /\\_/\\\n"
                " ( o.o )\n"
                "  > ^ <\n";
            break;
        case 1:
            std::cout << "  /\\_/\\\n"
                " ( ^.^ )\n"
                " (\")_(\")\n";
            break;
        case 2:
            std::cout << " |\\__/|\n"
                " (=-.-=)  zZz\n"
                " (\")_(\")\n";
            break;
        case 3:
            std::cout << "  /\\_/\\\n"
                " ( o o )\n"
                " ==_Y_==\n"
                "   `-'\n";
            break;
        default:
            std::cout << " (\\_/)\n"
                " ( o.o )\n"
                " (>   <)\n";
            break;
        }
    }

    void ShowInstructions() {
        ClearScreen();
        std::cout << "==============================================\n";
        std::cout << "                 ИНСТРУКЦИЯ\n";
        std::cout << "==============================================\n\n";

        std::cout << "ЦЕЛЬ ИГРЫ\n\n";
        std::cout << "  Сначала собрать сокровище ($), затем дойти до выхода (E).\n";
        std::cout << "  Пока сокровище не собрано, выход не засчитывается.\n\n";

        std::cout << "----------------------------------------------\n";
        std::cout << "ОБОЗНАЧЕНИЯ НА ПОЛЕ\n\n";
        std::cout << "  @        игрок (вы)\n";
        std::cout << "  $        сокровище\n";
        std::cout << "  E        выход\n";
        std::cout << "  | - +    стены\n";
        std::cout << "  пробел   свободный проход\n\n";

        std::cout << "----------------------------------------------\n";
        std::cout << "УПРАВЛЕНИЕ (одна буква + Enter)\n\n";
        std::cout << "  W  вверх        A  влево\n";
        std::cout << "  S  вниз         D  вправо\n";
        std::cout << "  Z  отменить последний шаг\n";
        std::cout << "  H  подсказка (кратчайший путь отметится '*')\n";
        std::cout << "  Q  выйти в главное меню\n\n";

        std::cout << "----------------------------------------------\n";
        std::cout << "ИСТОЧНИКИ УРОВНЕЙ\n\n";
        std::cout << "  1) готовый уровень;\n";
        std::cout << "  2) случайный: вы задаёте размер от " << kMinMazeSize << " до "
            << kMaxMazeSize << " и выбираете (Y/N),\n";
        std::cout << "     делать ли следующие уровни такого же размера;\n";
        std::cout << "  3) из текстового файла (.txt).\n\n";

        std::cout << "----------------------------------------------\n";
        std::cout << "ФОРМАТ ФАЙЛА С ЛАБИРИНТОМ\n\n";
        std::cout << "  Каждая строка файла — один ряд лабиринта.\n\n";
        std::cout << "  Разрешённые символы:\n";
        std::cout << "    #            стена\n";
        std::cout << "    .  (пробел)  проход\n";
        std::cout << "    P            старт игрока  — ровно один\n";
        std::cout << "    T            сокровище     — ровно одно\n";
        std::cout << "    E            выход         — ровно один\n\n";
        std::cout << "  Дубликаты P/T/E и любые другие символы — ошибка.\n";
        std::cout << "  До сокровища и выхода должен существовать путь.\n";
        std::cout << "  Лабиринт лучше окружить стенами по краям.\n";
        PauseForEnter();
    }

    void ShowRecords(const RecordList& records) {
        ClearScreen();
        std::cout << "=== ТАБЛИЦА ПРОХОЖДЕНИЙ (последние 5) ===\n\n";
        records.Print();
        PauseForEnter();
    }

    // Спросить имя игрока. Пустое имя или имя длиннее kMaxNameLength символов
    // не принимается — спрашиваем заново. 
    // Русские буквы и пробелы разрешены.
    std::string AskPlayerName() {
        std::string error;
        while (true) {
            ClearScreen();
            std::cout << "=== СОХРАНЕНИЕ ПРОХОЖДЕНИЯ ===\n\n";
            if (!error.empty()) {
                std::cout << "[!] " << error << "\n";
            }
            std::string name = Validator::Trim(
                Validator::ReadLine("Введите ваше имя (до " +
                    std::to_string(kMaxNameLength) + " символов): "));
            if (name.empty()) {
                error = "Имя не может быть пустым. Введите хотя бы один символ.";
                continue;
            }
            if (name.size() > kMaxNameLength) {
                error = "Имя слишком длинное (" + std::to_string(name.size()) +
                    " символов). Максимум — " + std::to_string(kMaxNameLength) + ".";
                continue;
            }
            return name;
        }
    }

    // Выбрать источник уровня и построить лабиринт.
    // true — лабиринт готов и корректен; в mode записывается, как он выбран.
    // Для случайного режима: chosen_size — выбранный размер,
    // keep_size — делать ли ВСЕ следующие уровни такого же размера.
    bool BuildLevel(Maze& maze, Point& player, Point& treasure, Point& exit,
        std::string& mode, int& chosen_size, bool& keep_size) {
        chosen_size = 0;
        keep_size = false;
        std::string menu =
            "=== ВЫБОР УРОВНЯ ===\n\n"
            "1. Готовый уровень\n"
            "2. Случайный уровень\n"
            "3. Загрузить из файла\n"
            "0. Назад\n";
        int choice = Validator::ReadMenuChoice(menu, 0, 3);
        if (choice == 0) return false;

        try {
            if (choice == 1) {
                maze.LoadFromText(PrebuiltMaze(), player, treasure, exit);
                mode = "заготовленный";
            }
            else if (choice == 2) {
                // Спрашиваем размер поля. Меньше 5 лабиринт не имеет смысла
                // (рамка займёт почти всё), больше 30 — не помещается на экран.
                chosen_size = Validator::ReadMenuChoice(
                    "=== СЛУЧАЙНЫЙ УРОВЕНЬ ===\n\n"
                    "Введите размер лабиринта (одно число N, поле будет N x N).\n"
                    "Допустимо от " + std::to_string(kMinMazeSize) + " до " +
                    std::to_string(kMaxMazeSize) + ".\n",
                    kMinMazeSize, kMaxMazeSize);
                keep_size = Validator::ReadYesNo(
                    "Делать все следующие уровни такого же размера (" +
                    std::to_string(chosen_size) + " x " + std::to_string(chosen_size) +
                    ")?\nY — да, все уровни одинаковые.\n"
                    "N — нет, такой только первый, дальше уровни будут расти.");
                maze.GenerateRandom(chosen_size, chosen_size, player, treasure, exit);
                mode = "случайный";
            }
            else {
                std::string filename = Validator::ReadLine("Имя файла: ");
                if (!Validator::FileExists(filename)) {
                    throw FileException("Файл не найден: " + filename);
                }
                maze.LoadFromFile(filename, player, treasure, exit);
                mode = "из файла (" + filename + ")";
            }
            // Проверяем корректность лабиринта (есть игрок/сокровище/выход и путь к ним).
            Validator::ValidateMaze(maze, player, treasure, exit);
        }
        catch (const MazeException& error) {
            std::cout << "Ошибка: " << error.what() << "\n";
            PauseForEnter();
            return false;
        }
        return true;
    }

    void StartGame(RecordList& records) {
        Maze maze;
        Point player;
        Point treasure;
        Point exit;
        std::string mode;
        int chosen_size = 0;
        bool keep_size = false;
        if (!BuildLevel(maze, player, treasure, exit, mode, chosen_size, keep_size)) return;

        Game game;
        game.SetupLevel(maze, player, treasure, exit);
        bool won = game.Play();

        // Если уровень пройден — предлагаем перейти на следующий (случайный, побольше).
        while (won) {
            std::string menu =
                "Уровень пройден!\n\n"
                "1. Следующий уровень\n"
                "0. Закончить и сохранить результат\n";
            int next = Validator::ReadMenuChoice(menu, 0, 1);
            if (next == 0) break;

            Maze next_maze;
            Point next_player;
            Point next_treasure;
            Point next_exit;
            // Размер следующего уровня: если игрок выбрал «все одинаковые» —
            // используем выбранный размер; иначе уровни постепенно растут.
            int size = 0;
            if (keep_size && chosen_size > 0) {
                size = chosen_size;
            }
            else {
                size = maze.GetRows() + game.GetLevelsCompleted() * 2;
                if (size > kMaxMazeSize) size = kMaxMazeSize;
                if (size < kMinMazeSize) size = kMinMazeSize;
            }
            next_maze.GenerateRandom(size, size, next_player, next_treasure, next_exit);
            try {
                Validator::ValidateMaze(next_maze, next_player, next_treasure, next_exit);
            }
            catch (const MazeException& error) {
                std::cout << "Не удалось создать уровень: " << error.what() << "\n";
                break;
            }
            game.SetupLevel(next_maze, next_player, next_treasure, next_exit);
            won = game.Play();
        }

        // Если игрок прошёл хотя бы один уровень — предлагаем сохранить прохождение,
        // но даём и возможность вернуться в меню без сохранения.
        if (game.GetLevelsCompleted() > 0) {
            std::string finish_menu =
                "=== ИГРА ОКОНЧЕНА ===\n\n"
                "Пройдено уровней: " + std::to_string(game.GetLevelsCompleted()) +
                "\nВсего шагов: " + std::to_string(game.GetTotalSteps()) +
                "\n\n"
                "1. Сохранить результат в таблицу прохождений\n"
                "0. Вернуться в меню без сохранения\n";
            int save = Validator::ReadMenuChoice(finish_menu, 0, 1);
            if (save == 1) {
                std::string name = AskPlayerName();
                records.Add(Record(name, game.GetLevelsCompleted(), game.GetTotalSteps(), mode));
                try {
                    records.SaveToFile(kRecordsFile);
                    std::cout << "Прохождение сохранено.\n";
                }
                catch (const FileException& error) {
                    std::cout << "Не удалось сохранить прохождение: " << error.what() << "\n";
                }
                PauseForEnter();
            }
        }
    }

}

int main() {
    InitConsole(); // настройка консоли под русские буквы (ввод и вывод)
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Верхний уровень обработки исключений: что бы ни случилось внутри,
    // программа не «упадёт», а сообщит об ошибке и корректно завершится.
    try {
        RecordList records(5);
        records.LoadFromFile(kRecordsFile); // подгружаем сохранённые прохождения (если есть)

        std::string menu =
            "==============================\n"
            "           ЛАБИРИНТ\n"
            "==============================\n"
            "1. Начать игру\n"
            "2. Таблица прохождений\n"
            "3. Инструкция\n"
            "0. Выход\n";

        while (true) {
            int choice = Validator::ReadMenuChoice(menu, 0, 3);
            switch (choice) {
            case 1:
                StartGame(records);
                break;
            case 2:
                ShowRecords(records);
                break;
            case 3:
                ShowInstructions();
                break;
            case 0:
                ClearScreen();
                std::cout << "До встречи!\n\n";
                PrintRandomCat();
                return 0;
            }
        }
    }
    catch (const std::exception& error) {
        std::cout << "\nКритическая ошибка: " << error.what() << "\n";
        return 1;
    }
}
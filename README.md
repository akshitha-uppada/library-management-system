# Library Management System

A console-based Library Management System written in C++ (object-oriented),
covering:

- **Book & Member classes** — structured storage of book and member details
- **Issue / Return** — borrow records track book, member, issue date, and return date
- **Search** — find books by title or author (partial, case-insensitive match)

Data is saved to plain text files (`books.txt`, `members.txt`, `records.txt`)
in the same folder as the program, so your library persists between runs.

## Project Structure

```
LibraryManagementSystem/
├── src/
│   ├── main.cpp          # Dev-C++ compatible version (C++98/03 style) - USE THIS in Dev-C++
│   └── main_cpp17.cpp    # Modern C++17 version (for g++/clang/VS on any OS)
└── README.md
```

## Running in Dev-C++ (Bloodshed / Orwell Dev-C++)

1. Open Dev-C++
2. **File -> New -> Project -> Console Application -> C++** -> name it, click OK
3. Dev-C++ creates a default `main.cpp` with a "Hello World" template — select
   all of it (Ctrl+A) and delete it
4. Open `src/main.cpp` from this zip, copy all its contents, and paste it into
   the empty Dev-C++ editor
5. Press **F9** (or Execute -> Compile & Run)

No compiler flags or settings changes are needed — `src/main.cpp` uses only
classic C++98/03 features (no `auto`, no range-based `for`, no `nullptr`,
no `stoi`/`to_string`, no in-class member initializers), which is exactly what
Dev-C++'s bundled compiler expects by default. This was the cause of the
"errors" — the original version used modern C++17 syntax that Dev-C++
doesn't understand out of the box.

If you'd rather use the modern version (`main_cpp17.cpp`) in Dev-C++, you can,
but you must first enable C++11 support: **Tools -> Compiler Options ->
Settings -> Code Generation -> Language Standard -> ISO C++11**, then rebuild.

## Building outside Dev-C++ (g++ on Windows/Mac/Linux)

```bash
g++ -o library src/main.cpp
./library
```

## Menu

```
1. Add Book
2. Add Member
3. Issue Book
4. Return Book
5. Search Books (title/author)
6. List All Books
7. List All Members
8. List Active Borrow Records
0. Exit
```

## Notes / Ideas for Extension

- Add due dates and overdue fines
- Add password-protected admin login
- Export reports to CSV
- Replace text-file storage with SQLite

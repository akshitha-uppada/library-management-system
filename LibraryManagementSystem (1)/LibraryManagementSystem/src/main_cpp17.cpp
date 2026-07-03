/*
 * Library Management System
 * -----------------------------------------
 * A console-based application to manage books, members, and borrowing
 * records using object-oriented programming in C++.
 *
 * Features:
 *   1. Classes for Book and Member details
 *   2. Book issue and return functionality
 *   3. Search functionality by title or author
 *
 * Data is persisted to simple text files (books.txt, members.txt,
 * records.txt) so information survives between runs.
 *
 * Compile:  g++ -std=c++17 -o library main.cpp
 * Run:      ./library
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <ctime>

using namespace std;

// ------------------------- Utility helpers -------------------------

static string toLower(const string& s) {
    string out = s;
    transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

static string currentDate() {
    time_t now = time(nullptr);
    tm* ltm = localtime(&now);
    ostringstream oss;
    oss << (1900 + ltm->tm_year) << "-"
        << (ltm->tm_mon + 1 < 10 ? "0" : "") << (ltm->tm_mon + 1) << "-"
        << (ltm->tm_mday < 10 ? "0" : "") << ltm->tm_mday;
    return oss.str();
}

static int readIntInRange(const string& prompt, int lo, int hi) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= lo && value <= hi) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Invalid input. Please try again.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

static string readLine(const string& prompt) {
    string line;
    cout << prompt;
    getline(cin, line);
    return line;
}

// ------------------------- Data Classes -------------------------

class Book {
public:
    int id;
    string title;
    string author;
    int totalCopies;
    int availableCopies;

    Book() : id(0), totalCopies(0), availableCopies(0) {}
    Book(int id_, string title_, string author_, int copies_)
        : id(id_), title(move(title_)), author(move(author_)),
          totalCopies(copies_), availableCopies(copies_) {}

    string serialize() const {
        ostringstream oss;
        oss << id << "|" << title << "|" << author << "|"
            << totalCopies << "|" << availableCopies;
        return oss.str();
    }

    static Book deserialize(const string& line) {
        Book b;
        stringstream ss(line);
        string token;
        getline(ss, token, '|'); b.id = stoi(token);
        getline(ss, token, '|'); b.title = token;
        getline(ss, token, '|'); b.author = token;
        getline(ss, token, '|'); b.totalCopies = stoi(token);
        getline(ss, token, '|'); b.availableCopies = stoi(token);
        return b;
    }

    void print() const {
        cout << left
             << "  ID: " << id
             << " | Title: " << title
             << " | Author: " << author
             << " | Available: " << availableCopies << "/" << totalCopies
             << "\n";
    }
};

class Member {
public:
    int id;
    string name;
    string email;

    Member() : id(0) {}
    Member(int id_, string name_, string email_)
        : id(id_), name(move(name_)), email(move(email_)) {}

    string serialize() const {
        ostringstream oss;
        oss << id << "|" << name << "|" << email;
        return oss.str();
    }

    static Member deserialize(const string& line) {
        Member m;
        stringstream ss(line);
        string token;
        getline(ss, token, '|'); m.id = stoi(token);
        getline(ss, token, '|'); m.name = token;
        getline(ss, token, '|'); m.email = token;
        return m;
    }

    void print() const {
        cout << "  ID: " << id << " | Name: " << name
             << " | Email: " << email << "\n";
    }
};

class BorrowRecord {
public:
    int recordId;
    int bookId;
    int memberId;
    string issueDate;
    string returnDate; // empty if not yet returned

    BorrowRecord() : recordId(0), bookId(0), memberId(0) {}
    BorrowRecord(int recordId_, int bookId_, int memberId_,
                 string issueDate_, string returnDate_ = "")
        : recordId(recordId_), bookId(bookId_), memberId(memberId_),
          issueDate(move(issueDate_)), returnDate(move(returnDate_)) {}

    bool isReturned() const { return !returnDate.empty(); }

    string serialize() const {
        ostringstream oss;
        oss << recordId << "|" << bookId << "|" << memberId << "|"
            << issueDate << "|" << returnDate;
        return oss.str();
    }

    static BorrowRecord deserialize(const string& line) {
        BorrowRecord r;
        stringstream ss(line);
        string token;
        getline(ss, token, '|'); r.recordId = stoi(token);
        getline(ss, token, '|'); r.bookId = stoi(token);
        getline(ss, token, '|'); r.memberId = stoi(token);
        getline(ss, token, '|'); r.issueDate = token;
        getline(ss, r.returnDate, '|');
        return r;
    }
};

// ------------------------- Library System -------------------------

class Library {
private:
    vector<Book> books;
    vector<Member> members;
    vector<BorrowRecord> records;

    int nextBookId = 1;
    int nextMemberId = 1;
    int nextRecordId = 1;

    const string booksFile = "books.txt";
    const string membersFile = "members.txt";
    const string recordsFile = "records.txt";

public:
    Library() { loadAll(); }

    ~Library() { saveAll(); }

    // ---------- Persistence ----------
    void loadAll() {
        loadBooks();
        loadMembers();
        loadRecords();
    }

    void saveAll() {
        saveBooks();
        saveMembers();
        saveRecords();
    }

    void loadBooks() {
        ifstream in(booksFile);
        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;
            Book b = Book::deserialize(line);
            books.push_back(b);
            nextBookId = max(nextBookId, b.id + 1);
        }
    }

    void loadMembers() {
        ifstream in(membersFile);
        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;
            Member m = Member::deserialize(line);
            members.push_back(m);
            nextMemberId = max(nextMemberId, m.id + 1);
        }
    }

    void loadRecords() {
        ifstream in(recordsFile);
        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;
            BorrowRecord r = BorrowRecord::deserialize(line);
            records.push_back(r);
            nextRecordId = max(nextRecordId, r.recordId + 1);
        }
    }

    void saveBooks() {
        ofstream out(booksFile, ios::trunc);
        for (const auto& b : books) out << b.serialize() << "\n";
    }

    void saveMembers() {
        ofstream out(membersFile, ios::trunc);
        for (const auto& m : members) out << m.serialize() << "\n";
    }

    void saveRecords() {
        ofstream out(recordsFile, ios::trunc);
        for (const auto& r : records) out << r.serialize() << "\n";
    }

    // ---------- Book management ----------
    void addBook() {
        string title = readLine("Enter book title: ");
        string author = readLine("Enter author name: ");
        int copies = readIntInRange("Enter number of copies: ", 1, 10000);

        Book b(nextBookId++, title, author, copies);
        books.push_back(b);
        saveBooks();
        cout << "Book added successfully. (Book ID: " << b.id << ")\n";
    }

    Book* findBookById(int id) {
        for (auto& b : books) if (b.id == id) return &b;
        return nullptr;
    }

    Member* findMemberById(int id) {
        for (auto& m : members) if (m.id == id) return &m;
        return nullptr;
    }

    void listBooks() const {
        if (books.empty()) { cout << "No books in the library yet.\n"; return; }
        cout << "\n--- All Books ---\n";
        for (const auto& b : books) b.print();
    }

    // ---------- Member management ----------
    void addMember() {
        string name = readLine("Enter member name: ");
        string email = readLine("Enter member email: ");

        Member m(nextMemberId++, name, email);
        members.push_back(m);
        saveMembers();
        cout << "Member added successfully. (Member ID: " << m.id << ")\n";
    }

    void listMembers() const {
        if (members.empty()) { cout << "No members registered yet.\n"; return; }
        cout << "\n--- All Members ---\n";
        for (const auto& m : members) m.print();
    }

    // ---------- Borrow / Return ----------
    void issueBook() {
        if (books.empty() || members.empty()) {
            cout << "You need at least one book and one member first.\n";
            return;
        }
        int bookId = readIntInRange("Enter Book ID to issue: ", 1, 1000000);
        int memberId = readIntInRange("Enter Member ID: ", 1, 1000000);

        Book* b = findBookById(bookId);
        Member* m = findMemberById(memberId);

        if (!b) { cout << "Book not found.\n"; return; }
        if (!m) { cout << "Member not found.\n"; return; }
        if (b->availableCopies <= 0) {
            cout << "No available copies of \"" << b->title << "\" right now.\n";
            return;
        }

        b->availableCopies--;
        BorrowRecord r(nextRecordId++, bookId, memberId, currentDate());
        records.push_back(r);

        saveBooks();
        saveRecords();

        cout << "Book \"" << b->title << "\" issued to " << m->name
             << " on " << r.issueDate << ". (Record ID: " << r.recordId << ")\n";
    }

    void returnBook() {
        int recordId = readIntInRange("Enter Record ID to return: ", 1, 1000000);

        for (auto& r : records) {
            if (r.recordId == recordId) {
                if (r.isReturned()) {
                    cout << "This record is already marked as returned.\n";
                    return;
                }
                r.returnDate = currentDate();
                Book* b = findBookById(r.bookId);
                if (b) b->availableCopies++;

                saveBooks();
                saveRecords();

                cout << "Book returned successfully on " << r.returnDate << ".\n";
                return;
            }
        }
        cout << "Record not found.\n";
    }

    void listActiveBorrows() const {
        bool any = false;
        cout << "\n--- Active (Unreturned) Borrow Records ---\n";
        for (const auto& r : records) {
            if (r.isReturned()) continue;
            any = true;
            cout << "  Record ID: " << r.recordId
                 << " | Book ID: " << r.bookId
                 << " | Member ID: " << r.memberId
                 << " | Issued: " << r.issueDate << "\n";
        }
        if (!any) cout << "  No active borrow records.\n";
    }

    // ---------- Search ----------
    void searchBooks() const {
        cout << "Search by (1) Title  (2) Author: ";
        int choice = readIntInRange("", 1, 2);
        string query = toLower(readLine("Enter search text: "));

        vector<Book> results;
        for (const auto& b : books) {
            string field = toLower(choice == 1 ? b.title : b.author);
            if (field.find(query) != string::npos) results.push_back(b);
        }

        if (results.empty()) {
            cout << "No matching books found.\n";
        } else {
            cout << "\n--- Search Results (" << results.size() << ") ---\n";
            for (const auto& b : results) b.print();
        }
    }

    // ---------- Menu ----------
    void run() {
        while (true) {
            cout << "\n============================\n";
            cout << "   LIBRARY MANAGEMENT SYSTEM\n";
            cout << "============================\n";
            cout << "1. Add Book\n";
            cout << "2. Add Member\n";
            cout << "3. Issue Book\n";
            cout << "4. Return Book\n";
            cout << "5. Search Books (title/author)\n";
            cout << "6. List All Books\n";
            cout << "7. List All Members\n";
            cout << "8. List Active Borrow Records\n";
            cout << "0. Exit\n";

            int choice = readIntInRange("Choose an option: ", 0, 8);

            switch (choice) {
                case 1: addBook(); break;
                case 2: addMember(); break;
                case 3: issueBook(); break;
                case 4: returnBook(); break;
                case 5: searchBooks(); break;
                case 6: listBooks(); break;
                case 7: listMembers(); break;
                case 8: listActiveBorrows(); break;
                case 0:
                    cout << "Saving data and exiting. Goodbye!\n";
                    saveAll();
                    return;
            }
        }
    }
};

int main() {
    Library library;
    library.run();
    return 0;
}

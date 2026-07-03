/*
 * Library Management System
 * -----------------------------------------
 * A console-based application to manage books, members, and borrowing
 * records using object-oriented programming in C++.
 *
 * Written in classic C++ (C++98/03) style so it compiles cleanly in
 * Dev-C++ with NO extra compiler settings (no -std=c++11/17 needed).
 *
 * Features:
 *   1. Classes for Book and Member details
 *   2. Book issue and return functionality
 *   3. Search functionality by title or author
 *
 * Data is saved to text files (books.txt, members.txt, records.txt)
 * so information survives between runs.
 *
 * HOW TO RUN IN DEV-C++:
 *   1. Open Dev-C++
 *   2. File -> New -> Project -> Console Application -> C++ -> OK
 *   3. Delete the default code in main.cpp and paste this file's contents
 *   4. Press F9 (Compile & Run)
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <limits>

using namespace std;

// ------------------------- Utility helpers -------------------------

string toLowerStr(const string& s) {
    string out = s;
    for (size_t i = 0; i < out.size(); i++) {
        out[i] = (char)tolower((unsigned char)out[i]);
    }
    return out;
}

string currentDate() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    ostringstream oss;
    int year = 1900 + ltm->tm_year;
    int mon = ltm->tm_mon + 1;
    int day = ltm->tm_mday;
    oss << year << "-";
    if (mon < 10) oss << "0";
    oss << mon << "-";
    if (day < 10) oss << "0";
    oss << day;
    return oss.str();
}

int readIntInRange(const string& prompt, int lo, int hi) {
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

string readLine(const string& prompt) {
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

    Book() {
        id = 0;
        totalCopies = 0;
        availableCopies = 0;
    }

    Book(int id_, string title_, string author_, int copies_) {
        id = id_;
        title = title_;
        author = author_;
        totalCopies = copies_;
        availableCopies = copies_;
    }

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
        getline(ss, token, '|'); b.id = atoi(token.c_str());
        getline(ss, token, '|'); b.title = token;
        getline(ss, token, '|'); b.author = token;
        getline(ss, token, '|'); b.totalCopies = atoi(token.c_str());
        getline(ss, token, '|'); b.availableCopies = atoi(token.c_str());
        return b;
    }

    void print() const {
        cout << "  ID: " << id
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

    Member() {
        id = 0;
    }

    Member(int id_, string name_, string email_) {
        id = id_;
        name = name_;
        email = email_;
    }

    string serialize() const {
        ostringstream oss;
        oss << id << "|" << name << "|" << email;
        return oss.str();
    }

    static Member deserialize(const string& line) {
        Member m;
        stringstream ss(line);
        string token;
        getline(ss, token, '|'); m.id = atoi(token.c_str());
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

    BorrowRecord() {
        recordId = 0;
        bookId = 0;
        memberId = 0;
    }

    BorrowRecord(int recordId_, int bookId_, int memberId_,
                 string issueDate_, string returnDate_ = "") {
        recordId = recordId_;
        bookId = bookId_;
        memberId = memberId_;
        issueDate = issueDate_;
        returnDate = returnDate_;
    }

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
        getline(ss, token, '|'); r.recordId = atoi(token.c_str());
        getline(ss, token, '|'); r.bookId = atoi(token.c_str());
        getline(ss, token, '|'); r.memberId = atoi(token.c_str());
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

    int nextBookId;
    int nextMemberId;
    int nextRecordId;

    string booksFile;
    string membersFile;
    string recordsFile;

public:
    Library() {
        nextBookId = 1;
        nextMemberId = 1;
        nextRecordId = 1;
        booksFile = "books.txt";
        membersFile = "members.txt";
        recordsFile = "records.txt";
        loadAll();
    }

    ~Library() {
        saveAll();
    }

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
        ifstream in(booksFile.c_str());
        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;
            Book b = Book::deserialize(line);
            books.push_back(b);
            if (b.id + 1 > nextBookId) nextBookId = b.id + 1;
        }
    }

    void loadMembers() {
        ifstream in(membersFile.c_str());
        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;
            Member m = Member::deserialize(line);
            members.push_back(m);
            if (m.id + 1 > nextMemberId) nextMemberId = m.id + 1;
        }
    }

    void loadRecords() {
        ifstream in(recordsFile.c_str());
        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;
            BorrowRecord r = BorrowRecord::deserialize(line);
            records.push_back(r);
            if (r.recordId + 1 > nextRecordId) nextRecordId = r.recordId + 1;
        }
    }

    void saveBooks() {
        ofstream out(booksFile.c_str(), ios::trunc);
        for (size_t i = 0; i < books.size(); i++) {
            out << books[i].serialize() << "\n";
        }
    }

    void saveMembers() {
        ofstream out(membersFile.c_str(), ios::trunc);
        for (size_t i = 0; i < members.size(); i++) {
            out << members[i].serialize() << "\n";
        }
    }

    void saveRecords() {
        ofstream out(recordsFile.c_str(), ios::trunc);
        for (size_t i = 0; i < records.size(); i++) {
            out << records[i].serialize() << "\n";
        }
    }

    // ---------- Book management ----------
    void addBook() {
        string title = readLine("Enter book title: ");
        string author = readLine("Enter author name: ");
        int copies = readIntInRange("Enter number of copies: ", 1, 10000);

        Book b(nextBookId, title, author, copies);
        nextBookId++;
        books.push_back(b);
        saveBooks();
        cout << "Book added successfully. (Book ID: " << b.id << ")\n";
    }

    Book* findBookById(int id) {
        for (size_t i = 0; i < books.size(); i++) {
            if (books[i].id == id) return &books[i];
        }
        return NULL;
    }

    Member* findMemberById(int id) {
        for (size_t i = 0; i < members.size(); i++) {
            if (members[i].id == id) return &members[i];
        }
        return NULL;
    }

    void listBooks() const {
        if (books.empty()) { cout << "No books in the library yet.\n"; return; }
        cout << "\n--- All Books ---\n";
        for (size_t i = 0; i < books.size(); i++) books[i].print();
    }

    // ---------- Member management ----------
    void addMember() {
        string name = readLine("Enter member name: ");
        string email = readLine("Enter member email: ");

        Member m(nextMemberId, name, email);
        nextMemberId++;
        members.push_back(m);
        saveMembers();
        cout << "Member added successfully. (Member ID: " << m.id << ")\n";
    }

    void listMembers() const {
        if (members.empty()) { cout << "No members registered yet.\n"; return; }
        cout << "\n--- All Members ---\n";
        for (size_t i = 0; i < members.size(); i++) members[i].print();
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

        if (b == NULL) { cout << "Book not found.\n"; return; }
        if (m == NULL) { cout << "Member not found.\n"; return; }
        if (b->availableCopies <= 0) {
            cout << "No available copies of \"" << b->title << "\" right now.\n";
            return;
        }

        b->availableCopies--;
        BorrowRecord r(nextRecordId, bookId, memberId, currentDate());
        nextRecordId++;
        records.push_back(r);

        saveBooks();
        saveRecords();

        cout << "Book \"" << b->title << "\" issued to " << m->name
             << " on " << r.issueDate << ". (Record ID: " << r.recordId << ")\n";
    }

    void returnBook() {
        int recordId = readIntInRange("Enter Record ID to return: ", 1, 1000000);

        for (size_t i = 0; i < records.size(); i++) {
            if (records[i].recordId == recordId) {
                if (records[i].isReturned()) {
                    cout << "This record is already marked as returned.\n";
                    return;
                }
                records[i].returnDate = currentDate();
                Book* b = findBookById(records[i].bookId);
                if (b != NULL) b->availableCopies++;

                saveBooks();
                saveRecords();

                cout << "Book returned successfully on " << records[i].returnDate << ".\n";
                return;
            }
        }
        cout << "Record not found.\n";
    }

    void listActiveBorrows() const {
        bool any = false;
        cout << "\n--- Active (Unreturned) Borrow Records ---\n";
        for (size_t i = 0; i < records.size(); i++) {
            if (records[i].isReturned()) continue;
            any = true;
            cout << "  Record ID: " << records[i].recordId
                 << " | Book ID: " << records[i].bookId
                 << " | Member ID: " << records[i].memberId
                 << " | Issued: " << records[i].issueDate << "\n";
        }
        if (!any) cout << "  No active borrow records.\n";
    }

    // ---------- Search ----------
    void searchBooks() const {
        cout << "Search by (1) Title  (2) Author: ";
        int choice = readIntInRange("", 1, 2);
        string query = toLowerStr(readLine("Enter search text: "));

        vector<Book> results;
        for (size_t i = 0; i < books.size(); i++) {
            string field = toLowerStr(choice == 1 ? books[i].title : books[i].author);
            if (field.find(query) != string::npos) results.push_back(books[i]);
        }

        if (results.empty()) {
            cout << "No matching books found.\n";
        } else {
            cout << "\n--- Search Results (" << results.size() << ") ---\n";
            for (size_t i = 0; i < results.size(); i++) results[i].print();
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

            if (choice == 1) addBook();
            else if (choice == 2) addMember();
            else if (choice == 3) issueBook();
            else if (choice == 4) returnBook();
            else if (choice == 5) searchBooks();
            else if (choice == 6) listBooks();
            else if (choice == 7) listMembers();
            else if (choice == 8) listActiveBorrows();
            else if (choice == 0) {
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

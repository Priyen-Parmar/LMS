#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <iomanip>
using namespace std;

class FileManager
{
private:
    vector<vector<string>> fileData;

public:
    void loadFile(const string &filename)
    {
        ifstream file(filename);
        fileData.clear();
        if (file)
        {
            string line;
            while (getline(file, line))
            {
                vector<string> row;
                stringstream ss(line);
                string field;
                while (getline(ss, field, ','))
                    row.push_back(field);
                fileData.push_back(row);
            }
        }
    }

    void saveFile(const string &filename)
    {
        ofstream file(filename);
        for (auto &row : fileData)
        {
            for (size_t i = 0; i < row.size(); ++i)
            {
                file << row[i];
                if (i != row.size() - 1)
                    file << ",";
            }
            file << "\n";
        }
        fileData.clear();
    }

    void appendRecord(const vector<string> &record, const string &filename)
    {
        ofstream file(filename, ios::app);
        for (size_t i = 0; i < record.size(); ++i)
        {
            file << record[i];
            if (i != record.size() - 1)
                file << ",";
        }
        file << "\n";
    }

    vector<vector<string>> &getData() { return fileData; }
};

class LibraryMember
{
protected:
    string memberId;
    string memberName;
    string memberPassword;
    int memberType;

    bool hasOverdueItems(int maxDays)
    {
        FileManager fm;
        fm.loadFile("transactions.csv");
        time_t now = time(0);

        for (auto &trans : fm.getData())
        {
            if (trans[0] == memberId)
            {
                time_t issueDate = stol(trans[4]);
                int daysOverdue = (now - issueDate) / 86400;
                if (daysOverdue > maxDays)
                    return true;
            }
        }
        return false;
    }

public:
    virtual void displayMainMenu() = 0;
    virtual ~LibraryMember() = default;
};

class Student : public LibraryMember
{
    const int MAX_BORROW = 3;
    const int LOAN_DAYS = 15;
    const float DAILY_FINE = 10.0;

public:
    Student(const string &id, const string &name, const string &pwd)
    {
        memberId = id;
        memberName = name;
        memberPassword = pwd;
        memberType = 1;
    }

    void displayMainMenu() override
    {
        while (true)
        {
            cout << "\nStudent Portal - " << memberName << "\n"
                 << "1. View Available Books\n"
                 << "2. View Current Loans\n"
                 << "3. Borrow Book\n"
                 << "4. Return Book\n"
                 << "5. Check Fines\n"
                 << "6. Reserve Book\n"
                 << "7. Logout\n"
                 << "Choice: ";

            int choice;
            cin >> choice;

            try
            {
                switch (choice)
                {
                case 1:
                    showAvailableBooks();
                    break;
                case 2:
                    showCurrentLoans();
                    break;
                case 3:
                    borrowBook();
                    break;
                case 4:
                    returnBook();
                    break;
                case 5:
                    calculateFines();
                    break;
                case 6:
                    reserveBook();
                    break;
                case 7:
                    return;
                default:
                    throw runtime_error("Invalid choice");
                }
            }
            catch (const exception &e)
            {
                cerr << "Error: " << e.what() << endl;
            }
        }
    }

private:
    void showAvailableBooks()
    {
        FileManager fm;
        fm.loadFile("books.csv");
        cout << "\nAvailable Books:\n";
        int count = 1;
        for (auto &book : fm.getData())
        {
            if (book[4] == "0")
            {
                cout << count++ << ". " << book[0]
                     << " by " << book[1] << " (ISBN: " << book[2] << ")\n";
            }
        }
    }

    void borrowBook()
    {
        if (hasOverdueItems(LOAN_DAYS))
        {
            cout << "You have overdue books. Return them first.\n";
            return;
        }

        FileManager fm;
        fm.loadFile("transactions.csv");
        int activeLoans = count_if(fm.getData().begin(), fm.getData().end(),
                                   [this](const vector<string> &t)
                                   { return t[0] == memberId && t[5] == "0"; });

        if (activeLoans >= MAX_BORROW)
        {
            cout << "Maximum borrowing limit reached!\n";
            return;
        }

        string isbn;
        cout << "Enter ISBN: ";
        cin >> isbn;

        fm.loadFile("books.csv");
        auto bookIt = find_if(fm.getData().begin(), fm.getData().end(),
                              [&isbn](const vector<string> &b)
                              { return b[2] == isbn && b[4] == "0"; });

        if (bookIt != fm.getData().end())
        {
            (*bookIt)[4] = "1";
            fm.saveFile("books.csv");

            time_t dueDate = time(0) + LOAN_DAYS * 86400;
            vector<string> transaction = {
                memberId,
                (*bookIt)[0],
                isbn,
                to_string(time(0)),
                to_string(dueDate),
                "0"};
            fm.appendRecord(transaction, "transactions.csv");
            cout << "Book borrowed successfully!\n";
        }
        else
        {
            cout << "Book not available!\n";
        }
    }

    void returnBook()
    {
        string isbn;
        cout << "Enter ISBN to return: ";
        cin >> isbn;

        FileManager fm;
        fm.loadFile("transactions.csv");
        bool found = false;

        for (auto &trans : fm.getData())
        {
            if (trans[0] == memberId && trans[2] == isbn && trans[5] == "0")
            {
                trans[5] = "1";
                found = true;
                break;
            }
        }

        if (found)
        {
            fm.saveFile("transactions.csv");

            fm.loadFile("books.csv");
            for (auto &book : fm.getData())
            {
                if (book[2] == isbn)
                    book[4] = "0";
            }
            fm.saveFile("books.csv");

            cout << "Book returned successfully!\n";
        }
        else
        {
            cout << "No active loan found for this book!\n";
        }
    }

    void calculateFines()
    {
        FileManager fm;
        fm.loadFile("transactions.csv");
        float total = 0.0;

        for (auto &trans : fm.getData())
        {
            if (trans[0] == memberId && trans[5] == "0")
            {
                time_t dueDate = stol(trans[4]);
                int daysOverdue = (time(0) - dueDate) / 86400;
                if (daysOverdue > 0)
                    total += daysOverdue * DAILY_FINE;
            }
        }
        cout << "Outstanding fines: ₹" << fixed << setprecision(2) << total << "\n";
    }

    void reserveBook()
    {
        string isbn;
        cout << "Enter ISBN to reserve: ";
        cin >> isbn;

        FileManager fm;
        fm.loadFile("books.csv");
        auto bookIt = find_if(fm.getData().begin(), fm.getData().end(),
                              [&isbn](const vector<string> &b)
                              {
                                  return b[2] == isbn && b[4] == "0" && b[5] == "0";
                              });

        if (bookIt != fm.getData().end())
        {
            (*bookIt)[5] = "1";
            fm.saveFile("books.csv");

            vector<string> reservation = {
                memberId,
                (*bookIt)[0],
                isbn,
                to_string(time(0))};
            fm.appendRecord(reservation, "reservations.csv");
            cout << "Book reserved successfully!\n";
        }
        else
        {
            cout << "Book not available for reservation!\n";
        }
    }

    void showCurrentLoans()
    {
        FileManager fm;
        fm.loadFile("transactions.csv");
        cout << "\nCurrent Loans:\n";
        for (auto &trans : fm.getData())
        {
            if (trans[0] == memberId && trans[5] == "0")
            {
                time_t dueDate = stol(trans[4]);
                tm *dt = localtime(&dueDate);
                cout << "- " << trans[1] << " (ISBN: " << trans[2]
                     << ") Due: " << put_time(dt, "%d/%m/%Y") << "\n";
            }
        }
    }
};

class Faculty : public LibraryMember
{
    const int MAX_BORROW = 10;
    const int LOAN_DAYS = 60;

public:
    Faculty(const string &id, const string &name, const string &pwd)
    {
        memberId = id;
        memberName = name;
        memberPassword = pwd;
        memberType = 2;
    }

    void displayMainMenu() override
    {
        while (true)
        {
            cout << "\nFaculty Portal - " << memberName << "\n"
                 << "1. View Available Books\n"
                 << "2. View Current Loans\n"
                 << "3. Borrow Book\n"
                 << "4. Return Book\n"
                 << "5. Reserve Book\n"
                 << "6. Logout\n"
                 << "Choice: ";

            int choice;
            cin >> choice;

            try
            {
                switch (choice)
                {
                case 1:
                    showAvailableBooks();
                    break;
                case 2:
                    showCurrentLoans();
                    break;
                case 3:
                    borrowBook();
                    break;
                case 4:
                    returnBook();
                    break;
                case 5:
                    reserveBook();
                    break;
                case 6:
                    return;
                default:
                    throw runtime_error("Invalid choice");
                }
            }
            catch (const exception &e)
            {
                cerr << "Error: " << e.what() << endl;
            }
        }
    }

private:
    void showAvailableBooks()
    {
        FileManager fm;
        fm.loadFile("books.csv");
        cout << "\nAvailable Books:\n";
        int count = 1;
        for (auto &book : fm.getData())
        {
            if (book[4] == "0")
            {
                cout << count++ << ". " << book[0]
                     << " by " << book[1] << " (ISBN: " << book[2] << ")\n";
            }
        }
    }

    void borrowBook()
    {
        FileManager fm;
        fm.loadFile("transactions.csv");
        int activeLoans = count_if(fm.getData().begin(), fm.getData().end(),
                                   [this](const vector<string> &t)
                                   { return t[0] == memberId && t[5] == "0"; });

        if (activeLoans >= MAX_BORROW)
        {
            cout << "Maximum borrowing limit reached!\n";
            return;
        }

        string isbn;
        cout << "Enter ISBN: ";
        cin >> isbn;

        fm.loadFile("books.csv");
        auto bookIt = find_if(fm.getData().begin(), fm.getData().end(),
                              [&isbn](const vector<string> &b)
                              { return b[2] == isbn && b[4] == "0"; });

        if (bookIt != fm.getData().end())
        {
            (*bookIt)[4] = "1";
            fm.saveFile("books.csv");

            time_t dueDate = time(0) + LOAN_DAYS * 86400;
            vector<string> transaction = {
                memberId,
                (*bookIt)[0],
                isbn,
                to_string(time(0)),
                to_string(dueDate),
                "0"};
            fm.appendRecord(transaction, "transactions.csv");
            cout << "Book borrowed successfully!\n";
        }
        else
        {
            cout << "Book not available!\n";
        }
    }

    void returnBook()
    {
        string isbn;
        cout << "Enter ISBN to return: ";
        cin >> isbn;

        FileManager fm;
        fm.loadFile("transactions.csv");
        bool found = false;

        for (auto &trans : fm.getData())
        {
            if (trans[0] == memberId && trans[2] == isbn && trans[5] == "0")
            {
                trans[5] = "1";
                found = true;
                break;
            }
        }

        if (found)
        {
            fm.saveFile("transactions.csv");

            fm.loadFile("books.csv");
            for (auto &book : fm.getData())
            {
                if (book[2] == isbn)
                    book[4] = "0";
            }
            fm.saveFile("books.csv");

            cout << "Book returned successfully!\n";
        }
        else
        {
            cout << "No active loan found for this book!\n";
        }
    }

    void reserveBook()
    {
        string isbn;
        cout << "Enter ISBN to reserve: ";
        cin >> isbn;

        FileManager fm;
        fm.loadFile("books.csv");
        auto bookIt = find_if(fm.getData().begin(), fm.getData().end(),
                              [&isbn](const vector<string> &b)
                              {
                                  return b[2] == isbn && b[4] == "0" && b[5] == "0";
                              });

        if (bookIt != fm.getData().end())
        {
            (*bookIt)[5] = "1";
            fm.saveFile("books.csv");

            vector<string> reservation = {
                memberId,
                (*bookIt)[0],
                isbn,
                to_string(time(0))};
            fm.appendRecord(reservation, "reservations.csv");
            cout << "Book reserved successfully!\n";
        }
        else
        {
            cout << "Book not available for reservation!\n";
        }
    }

    void showCurrentLoans()
    {
        FileManager fm;
        fm.loadFile("transactions.csv");
        cout << "\nCurrent Loans:\n";
        for (auto &trans : fm.getData())
        {
            if (trans[0] == memberId && trans[5] == "0")
            {
                time_t dueDate = stol(trans[4]);
                tm *dt = localtime(&dueDate);
                cout << "- " << trans[1] << " (ISBN: " << trans[2]
                     << ") Due: " << put_time(dt, "%d/%m/%Y") << "\n";
            }
        }
    }
};

class Librarian : public LibraryMember
{
public:
    Librarian(const string &id, const string &name, const string &pwd)
    {
        memberId = id;
        memberName = name;
        memberPassword = pwd;
        memberType = 3;
    }

    void displayMainMenu() override
    {
        while (true)
        {
            cout << "\nLibrarian Portal - " << memberName << "\n"
                 << "1. Add User\n"
                 << "2. Update User\n"
                 << "3. Remove User\n"
                 << "4. Add Book\n"
                 << "5. Update Book\n"
                 << "6. Remove Book\n"
                 << "7. View All Loans\n"
                 << "8. View Reservations\n"
                 << "9. Generate Reports\n"
                 << "10. View User Loans\n"
                 << "0. Logout\n"
                 << "Choice: ";

            int choice;
            cin >> choice;

            try
            {
                switch (choice)
                {
                case 1:
                    addUser();
                    break;
                case 2:
                    updateUser();
                    break;
                case 3:
                    removeUser();
                    break;
                case 4:
                    addBook();
                    break;
                case 5:
                    updateBook();
                    break;
                case 6:
                    removeBook();
                    break;
                case 7:
                    viewAllLoans();
                    break;
                case 8:
                    viewReservations();
                    break;
                case 9:
                    generateReports();
                    break;
                case 10:
                    viewUserLoans();
                    break;
                case 0:
                    return;
                default:
                    throw runtime_error("Invalid choice");
                }
            }
            catch (const exception &e)
            {
                cerr << "Error: " << e.what() << endl;
                cin.clear();
                // cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }
    }

private:
    void addUser()
    {
        vector<string> newUser(4);
        cout << "Enter User Type (1-Student, 2-Faculty, 3-Librarian): ";
        cin >> newUser[3];
        cout << "Enter Name: ";
        cin.ignore();
        getline(cin, newUser[0]);
        cout << "Enter User ID: ";
        cin >> newUser[1];
        cout << "Enter Password: ";
        cin >> newUser[2];

        FileManager fm;
        fm.appendRecord(newUser, "users.csv");
        cout << "User added successfully!\n";
    }

    void updateUser()
    {
        string userId;
        cout << "Enter User ID to update: ";
        cin >> userId;

        FileManager fm;
        fm.loadFile("users.csv");
        auto userIt = find_if(fm.getData().begin(), fm.getData().end(),
                              [&userId](const vector<string> &u)
                              { return u[1] == userId; });

        if (userIt != fm.getData().end())
        {
            cout << "Select field to update:\n"
                 << "1. Name\n2. Password\nChoice: ";
            int field;
            cin >> field;

            cout << "Enter new value: ";
            string value;
            cin.ignore();
            getline(cin, value);

            switch (field)
            {
            case 1:
                (*userIt)[0] = value;
                break;
            case 2:
                (*userIt)[2] = value;
                break;
            default:
                throw runtime_error("Invalid field");
            }
            fm.saveFile("users.csv");
            cout << "User updated successfully!\n";
        }
        else
        {
            throw runtime_error("User not found");
        }
    }

    void removeUser()
    {
        string userId;
        cout << "Enter User ID to remove: ";
        cin >> userId;

        FileManager fm;

        // Remove from users.csv
        fm.loadFile("users.csv");
        auto userIt = remove_if(fm.getData().begin(), fm.getData().end(),
                                [&userId](const vector<string> &u)
                                { return u[1] == userId; });

        if (userIt != fm.getData().end())
        {
            fm.getData().erase(userIt, fm.getData().end());
            fm.saveFile("users.csv");
            cout << "User removed from registry.\n";

            // Handle related transactions
            fm.loadFile("transactions.csv");
            vector<string> booksToReturn;
            for (auto &trans : fm.getData())
            {
                if (trans[0] == userId && trans[5] == "0")
                {
                    trans[5] = "1";
                    booksToReturn.push_back(trans[2]);
                }
            }
            fm.saveFile("transactions.csv");

            // Update book availability
            if (!booksToReturn.empty())
            {
                fm.loadFile("books.csv");
                for (auto &book : fm.getData())
                {
                    if (find(booksToReturn.begin(), booksToReturn.end(), book[2]) != booksToReturn.end())
                    {
                        book[4] = "0";
                    }
                }
                fm.saveFile("books.csv");
                cout << "Associated books returned to inventory.\n";
            }

            // Remove reservations
            fm.loadFile("reservations.csv");
            fm.getData().erase(remove_if(fm.getData().begin(), fm.getData().end(),
                                         [&userId](const vector<string> &r)
                                         { return r[0] == userId; }),
                               fm.getData().end());
            fm.saveFile("reservations.csv");

            cout << "User removed successfully!\n";
        }
        else
        {
            throw runtime_error("User not found!");
        }
    }

    void addBook()
    {
        vector<string> newBook(6);
        cout << "Enter Book Title: ";
        cin.ignore();
        getline(cin, newBook[0]);
        cout << "Enter Author: ";
        getline(cin, newBook[1]);
        cout << "Enter ISBN: ";
        getline(cin, newBook[2]);
        cout << "Enter Publisher: ";
        getline(cin, newBook[3]);
        newBook[4] = "0";
        newBook[5] = "0";

        FileManager fm;
        fm.appendRecord(newBook, "books.csv");
        cout << "Book added successfully!\n";
    }

    void updateBook()
    {
        string isbn;
        cout << "Enter ISBN to update: ";
        cin >> isbn;

        FileManager fm;
        fm.loadFile("books.csv");
        auto bookIt = find_if(fm.getData().begin(), fm.getData().end(),
                              [&isbn](const vector<string> &b)
                              { return b[2] == isbn; });

        if (bookIt != fm.getData().end())
        {
            cout << "Current Details:\n"
                 << "1. Title: " << (*bookIt)[0] << "\n"
                 << "2. Author: " << (*bookIt)[1] << "\n"
                 << "3. Publisher: " << (*bookIt)[3] << "\n"
                 << "Enter field number to update (1-3): ";

            int field;
            cin >> field;
            cin.ignore();

            if (field < 1 || field > 3)
            {
                throw runtime_error("Invalid field selection");
            }

            cout << "Enter new value: ";
            string value;
            getline(cin, value);

            switch (field)
            {
            case 1:
                (*bookIt)[0] = value;
                break;
            case 2:
                (*bookIt)[1] = value;
                break;
            case 3:
                (*bookIt)[3] = value;
                break;
            }

            fm.saveFile("books.csv");

            if (field == 1)
            {
                fm.loadFile("transactions.csv");
                for (auto &trans : fm.getData())
                {
                    if (trans[2] == isbn)
                    {
                        trans[1] = value;
                    }
                }
                fm.saveFile("transactions.csv");
            }

            cout << "Book updated successfully!\n";
        }
        else
        {
            throw runtime_error("Book not found!");
        }
    }

    void removeBook()
    {
        string isbn;
        cout << "Enter ISBN to remove: ";
        cin >> isbn;

        FileManager fm;

        fm.loadFile("books.csv");
        auto bookIt = remove_if(fm.getData().begin(), fm.getData().end(),
                                [&isbn](const vector<string> &b)
                                { return b[2] == isbn; });

        if (bookIt != fm.getData().end())
        {
            fm.getData().erase(bookIt, fm.getData().end());
            fm.saveFile("books.csv");
            cout << "Book removed from catalog.\n";

            fm.loadFile("transactions.csv");
            fm.getData().erase(remove_if(fm.getData().begin(), fm.getData().end(),
                                         [&isbn](const vector<string> &t)
                                         { return t[2] == isbn; }),
                               fm.getData().end());
            fm.saveFile("transactions.csv");

            fm.loadFile("reservations.csv");
            fm.getData().erase(remove_if(fm.getData().begin(), fm.getData().end(),
                                         [&isbn](const vector<string> &r)
                                         { return r[2] == isbn; }),
                               fm.getData().end());
            fm.saveFile("reservations.csv");

            cout << "Associated records cleaned up.\n";
        }
        else
        {
            throw runtime_error("Book not found!");
        }
    }

    void viewAllLoans()
    {
        FileManager fm;
        fm.loadFile("transactions.csv");
        cout << "\nAll Active Loans:\n";
        for (auto &trans : fm.getData())
        {
            if (trans[5] == "0")
            {
                time_t dueDate = stol(trans[4]);
                tm *dt = localtime(&dueDate);
                cout << "User: " << trans[0] << " | Book: " << trans[1]
                     << " (ISBN: " << trans[2] << ") | Due: "
                     << put_time(dt, "%d/%m/%Y") << "\n";
            }
        }
    }

    void viewReservations()
    {
        FileManager fm;
        fm.loadFile("reservations.csv");

        if (fm.getData().empty())
        {
            cout << "No active reservations.\n";
            return;
        }

        cout << "\nActive Reservations:\n";
        for (auto &res : fm.getData())
        {
            time_t resDate = stol(res[3]);
            tm *dt = localtime(&resDate);
            cout << "User: " << res[0]
                 << " | Book: " << res[1]
                 << " (ISBN: " << res[2] << ")"
                 << " | Reserved: " << put_time(dt, "%d/%m/%Y %H:%M") << "\n";
        }
    }

    void generateReports()
    {
        FileManager fm;

        fm.loadFile("users.csv");
        int totalUsers = fm.getData().size();

        fm.loadFile("books.csv");
        int totalBooks = fm.getData().size();
        int availableBooks = count_if(fm.getData().begin(), fm.getData().end(),
                                      [](const vector<string> &b)
                                      { return b[4] == "0"; });

        fm.loadFile("transactions.csv");
        int activeLoans = count_if(fm.getData().begin(), fm.getData().end(),
                                   [](const vector<string> &t)
                                   { return t[5] == "0"; });

        fm.loadFile("reservations.csv");
        int activeReservations = fm.getData().size();

        cout << "\n=== Library Status Report ===\n"
             << "Total Users: " << totalUsers << "\n"
             << "Total Books: " << totalBooks << "\n"
             << "Available Books: " << availableBooks << "\n"
             << "Active Loans: " << activeLoans << "\n"
             << "Active Reservations: " << activeReservations << "\n";

        float totalFines = 0.0;
        time_t now = time(0);
        for (auto &trans : fm.getData())
        {
            if (trans[5] == "0")
            {
                time_t dueDate = stol(trans[4]);
                int daysOverdue = (now - dueDate) / 86400;
                if (daysOverdue > 0)
                {
                    totalFines += daysOverdue * 10.0;
                }
            }
        }
        cout << "Estimated Outstanding Fines: ₹" << fixed << setprecision(2) << totalFines << "\n";
    }

    void viewUserLoans()
    {
        string userId;
        cout << "Enter User ID to view loans: ";
        cin >> userId;

        FileManager fm;
        fm.loadFile("transactions.csv");

        cout << "\nLoan History for User: " << userId << "\n";
        for (auto &trans : fm.getData())
        {
            if (trans[0] == userId)
            {
                time_t borrowDate = stol(trans[3]);
                time_t dueDate = stol(trans[4]);
                tm *bdt = localtime(&borrowDate);
                tm *ddt = localtime(&dueDate);

                cout << "- " << trans[1] << " (ISBN: " << trans[2] << ")\n"
                     << "  Borrowed: " << put_time(bdt, "%d/%m/%Y")
                     << " | Due: " << put_time(ddt, "%d/%m/%Y")
                     << " | Status: " << (trans[5] == "0" ? "Active" : "Returned") << "\n";
            }
        }
    }
};

class AuthService
{
public:
    static LibraryMember *authenticate()
    {
        string userId, password;
        cout << "User ID: ";
        cin >> userId;
        cout << "Password: ";
        cin >> password;

        FileManager fm;
        fm.loadFile("users.csv");

        for (auto &user : fm.getData())
        {
            if (user[1] == userId && user[2] == password)
            {
                if (user[3] == "1")
                    return new Student(user[1], user[0], user[2]);
                if (user[3] == "2")
                    return new Faculty(user[1], user[0], user[2]);
                if (user[3] == "3")
                    return new Librarian(user[1], user[0], user[2]);
            }
        }
        throw runtime_error("Authentication failed");
    }
};

int main()
{
    while (true)
    {
        try
        {
            cout << "\nLibrary Management System\n"
                 << "1. Login\n2. Exit\nChoice: ";
            int choice;
            cin >> choice;

            if (choice == 1)
            {
                LibraryMember *user = AuthService::authenticate();
                user->displayMainMenu();
                delete user;
            }
            else if (choice == 2)
            {
                break;
            }
        }
        catch (const exception &e)
        {
            cerr << "System Error: " << e.what() << endl;
        }
    }
    return 0;
}
# Library Management System

A C++ console application for managing library operations with role-based access control (Student, Faculty, Librarian).

## Features

### User Roles
- **Students**:
  - Borrow books (3 max, 15 days)
  - Return books with fine calculation (₹10/day)
  - Reserve available books
  - View loans and available books

- **Faculty**:
  - Borrow books (10 max, 60 days)
  - Reserve books
  - View loans and books

- **Librarians**:
  - Manage users 
  - Manage book catalog 
  - View all loans/reservations
  - Generate system reports
  - Manage book returns

## Installation

### Requirements
- C++11 compiler
- Makefile (optional)

### Setup
```bash
g++ -std=c++11 main.cpp -o library_system
```

### Data Files
Create these CSV files in the same directory:

#### users.csv
```
Name,UserID,Password,Type
John Doe,STU001,pass123,1
Jane Smith,FAC002,pass456,2
Admin,LIB003,admin789,3
```

#### books.csv
```
Title,Author,ISBN,Publisher,Available,Reserved
C++ Primer,Stanley Lippman,9780321714114,Addison-Wesley,0,0
```

#### transactions.csv and reservations.csv
(Empty files initially)

## Usage
Run the program:
```bash
./library_system
```

### Main Menu
```
1. Login
2. Exit
```

### Sample Workflow
1. Login as Librarian (LIB003/admin789)
2. Add books/users through the menu
3. Logout and login as Student
4. Borrow/reserve books
5. Return books with fines

## Data Structure

### CSV Formats
- **users.csv**: Columns: Name, UserID, Password, Type (1|2|3)
- **books.csv**: Columns: Title, Author, ISBN, Publisher, Available (0/1), Reserved (0/1)
- **transactions.csv**: Columns: UserID, BookTitle, ISBN, IssueDate, DueDate, ReturnStatus
- **reservations.csv**: Columns: UserID, BookTitle, ISBN, ReservationDate

## Class Diagram
```
FileManager -> Data Storage
LibraryMember (Abstract)
├── Student
├── Faculty
└── Librarian
AuthService
Main Application
```

## License
This project is unlicensed. Free for educational use.

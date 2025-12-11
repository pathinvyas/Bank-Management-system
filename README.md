# Sudbury Student Bank

This is a **fully code‑only Qt Widgets application** that simulates a modern banking experience.

It includes:

- Login & Signup screens
- Chequing and Savings accounts with **automatic monthly interest** on savings
- Deposits & withdrawals
- Internal (account‑to‑account) transfers
- Interac‑style email transfers between clients
- Credit card application flow (min limit 2000 CAD)
- Bill payments to sample payees
- Profile overview page
- Account statements (transaction history)
- FAQ section with sample questions/answers
- SQLite database created automatically on first run with **sample users & data**

## Tech stack

- **Qt 6** (Widgets + Sql)
- **C++17**
- **SQLite** via `QSQLITE`
- **CMake** build system
- 100% **code‑based UI** – no `.ui` files used

## Building (Qt Creator + CMake)

1. Open **Qt Creator**.
2. Choose **File → Open Project…** and select `CMakeLists.txt` in the root folder.
3. Configure a **Desktop Qt 6.x kit**.
4. Build and run the `SudburyStudentBank` target.

You must make sure the **Qt Sql** module is available in your kit (it usually is by default).

The app will create `bank.db` automatically in the working directory if it does not exist.

## Dummy login credentials

On first run the database is seeded with two demo clients:

1. **Alice Blue**
   - Email: `alice@example.com`
   - Password: `Password123!`
   - Accounts:
     - Chequing: ~3500 CAD
     - Savings: ~8200 CAD at **1.2% annual interest**
   - Interac email: `alice.interac@example.com`
   - 1 sample credit card with a limit of ~5000 CAD

2. **Bob Noir**
   - Email: `bob@example.com`
   - Password: `Password123!`
   - Accounts:
     - Chequing: ~900 CAD
   - Interac email: `bob.interac@example.com`

You can also **sign up** as a completely new client from the login screen. New users start with **0 accounts and 0 cards** – they can create accounts and apply for cards from within the main app.

## Features mapping

- **Login / Signup**
  - Implemented in `src/loginwindow.*`
  - Uses `users` table in SQLite via `DBManager`.

- **Accounts (Chequing & Savings)**
  - Tab: **Accounts**
  - Create accounts (type, initial deposit, optional interest rate).
  - Deposit / Withdraw operations.
  - Data: `accounts` and `transactions` tables.

- **Automatic monthly interest**
  - Implemented in `DBManager::applyMonthlyInterestForUser`.
  - Triggered when dashboard loads or overview refreshes.
  - Uses `interest_rate` and `last_interest_applied` columns in `accounts`.
  - No manual buttons – it simulates a scheduled monthly accrual.

- **Interac‑style email transfer**
  - Tab: **Transfers**
  - Uses `interac_registrations` and `transactions` tables.
  - Sample Interac emails:
    - `alice.interac@example.com`
    - `bob.interac@example.com`

- **Account‑to‑Account transfers**
  - Tab: **Transfers**
  - You can move money between any two of your own accounts in real time.

- **Credit cards**
  - Tab: **Credit Cards**
  - Apply for a new card (minimum 2000 CAD limit enforced in code).
  - Data: `credit_cards` table.

- **Bill payments**
  - Tab: **Bill Payments**
  - Pay sample payees:
    - Hydro One
    - Bell Canada
    - Netflix
    - City of Sudbury Property Tax
  - Data: `bill_payees`, `bill_payments`, `transactions` tables.

- **Profile page**
  - Shows username, email, DOB, and client since date.

- **Statements**
  - Tab: **Statements**
  - Choose any of your accounts and view the last 100 transactions.

- **FAQs**
  - Tab: **FAQs**
  - Loads Q/A pairs from the `faqs` table for display.

## Color theme & UI

- Primary background: **#050816** (deep navy/black)
- Accent blues: gradients from **#1E3C72 → #2A5298**
- Text: light blue/white for contrast
- All views styled globally via `QApplication::setStyleSheet` in `main.cpp`.


## Important note!!

- Passwords are stored in **plain text** for simplicity 
- There is **no real email sending** for Interac – it just simulates balance movement between clients.
- No external dependencies beyond Qt and SQLite.

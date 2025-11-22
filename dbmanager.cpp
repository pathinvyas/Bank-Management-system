#include "dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QDate>
#include <QDir>
#include <random>

QSqlDatabase DBManager::m_db;
int DBManager::m_nextAccountSeed = 9825;

QSqlDatabase DBManager::database() {
    return m_db;
}

bool DBManager::init(const QString &dbPath) {
    if (QSqlDatabase::contains("bluebank_connection")) {
        m_db = QSqlDatabase::database("bluebank_connection");
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "bluebank_connection");
        m_db.setDatabaseName(dbPath);
    }

    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    createTablesIfNeeded();
    createSampleDataIfEmpty();
    return true;
}

void DBManager::createTablesIfNeeded() {
    QSqlQuery q(m_db);

    q.exec("PRAGMA foreign_keys = ON");

    // users
    q.exec("CREATE TABLE IF NOT EXISTS users ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "email TEXT UNIQUE NOT NULL,"
           "password TEXT NOT NULL,"
           "username TEXT NOT NULL,"
           "dob TEXT,"
           "profile_pic TEXT,"
           "created_at TEXT DEFAULT CURRENT_TIMESTAMP"
           ")");

    // accounts
    q.exec("CREATE TABLE IF NOT EXISTS accounts ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "user_id INTEGER NOT NULL,"
           "account_number TEXT UNIQUE NOT NULL,"
           "type TEXT NOT NULL,"
           "balance REAL NOT NULL DEFAULT 0,"
           "interest_rate REAL NOT NULL DEFAULT 0,"
           "last_interest_applied TEXT,"
           "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE"
           ")");

    // credit cards
    q.exec("CREATE TABLE IF NOT EXISTS credit_cards ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "user_id INTEGER NOT NULL,"
           "card_number TEXT UNIQUE NOT NULL,"
           "cvv TEXT NOT NULL,"
           "expiry_month INTEGER NOT NULL,"
           "expiry_year INTEGER NOT NULL,"
           "credit_limit REAL NOT NULL,"
           "current_balance REAL NOT NULL DEFAULT 0,"
           "min_payment REAL NOT NULL DEFAULT 0,"
           "status TEXT NOT NULL DEFAULT 'Active',"
           "created_at TEXT DEFAULT CURRENT_TIMESTAMP,"
           "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE"
           ")");

    // transactions
    q.exec("CREATE TABLE IF NOT EXISTS transactions ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "account_id INTEGER NOT NULL,"
           "type TEXT NOT NULL,"
           "amount REAL NOT NULL,"
           "timestamp TEXT DEFAULT CURRENT_TIMESTAMP,"
           "description TEXT,"
           "related_account_id INTEGER,"
           "interac_email TEXT,"
           "FOREIGN KEY(account_id) REFERENCES accounts(id) ON DELETE CASCADE"
           ")");

    // interac registrations
    q.exec("CREATE TABLE IF NOT EXISTS interac_registrations ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "user_id INTEGER NOT NULL,"
           "account_id INTEGER NOT NULL,"
           "email TEXT UNIQUE NOT NULL,"
           "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE,"
           "FOREIGN KEY(account_id) REFERENCES accounts(id) ON DELETE CASCADE"
           ")");

    // bill payees
    q.exec("CREATE TABLE IF NOT EXISTS bill_payees ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "name TEXT NOT NULL,"
           "category TEXT"
           ")");

    // bill payments
    q.exec("CREATE TABLE IF NOT EXISTS bill_payments ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "user_id INTEGER NOT NULL,"
           "from_account_id INTEGER NOT NULL,"
           "payee_id INTEGER NOT NULL,"
           "amount REAL NOT NULL,"
           "timestamp TEXT DEFAULT CURRENT_TIMESTAMP,"
           "reference TEXT,"
           "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE,"
           "FOREIGN KEY(from_account_id) REFERENCES accounts(id) ON DELETE CASCADE,"
           "FOREIGN KEY(payee_id) REFERENCES bill_payees(id) ON DELETE CASCADE"
           ")");

    // FAQs
    q.exec("CREATE TABLE IF NOT EXISTS faqs ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "question TEXT NOT NULL,"
           "answer TEXT NOT NULL"
           ")");
}

void DBManager::createSampleDataIfEmpty() {
    QSqlQuery q(m_db);
    q.exec("SELECT COUNT(*) FROM users");
    if (q.next() && q.value(0).toInt() > 0) {
        return; // already seeded
    }

    // Sample users
    createUser("alice@example.com", "Password123!", "Alice Blue", QDate(2002, 1, 15));
    createUser("bob@example.com", "Password123!", "Bob Noir", QDate(2001, 4, 3));

    // Fetch IDs
    QSqlQuery q2(m_db);
    int aliceId = -1;
    int bobId = -1;

    q2.exec("SELECT id, email FROM users");
    while (q2.next()) {
        int id = q2.value(0).toInt();
        QString email = q2.value(1).toString();
        if (email == "alice@example.com") aliceId = id;
        if (email == "bob@example.com") bobId = id;
    }

    // Sample accounts
    int aliceChequing = createAccount(aliceId, "Chequing", 3500.0, 0.0);
    int aliceSavings = createAccount(aliceId, "Savings", 8200.0, 0.012); // 1.2% annually
    int bobChequing = createAccount(bobId, "Chequing", 900.0, 0.0);

    // Interac registrations
    registerInteracEmail(aliceId, aliceChequing, "alice.interac@example.com");
    registerInteracEmail(bobId, bobChequing, "bob.interac@example.com");

    // Credit card for Alice
    applyForCreditCard(aliceId, 5000.0);

    // Bill payees
    QSqlQuery bp(m_db);
    bp.prepare("INSERT INTO bill_payees (name, category) VALUES (?, ?)");
    QStringList names = {"Hydro One", "Bell Canada", "Netflix", "City of Sudbury Property Tax"};
    QStringList cats  = {"Utilities", "Telecom", "Streaming", "Municipal"};
    for (int i = 0; i < names.size(); ++i) {
        bp.bindValue(0, names[i]);
        bp.bindValue(1, cats[i]);
        bp.exec();
    }

    // FAQs
    QSqlQuery fq(m_db);
    fq.prepare("INSERT INTO faqs (question, answer) VALUES (?, ?)");
    fq.addBindValue("How do I open a new savings account?");
    fq.addBindValue("Go to Accounts → Create Account, choose 'Savings', and confirm your details.");
    fq.exec();

    fq.addBindValue("How is interest calculated on my savings account?");
    fq.addBindValue("Interest is calculated monthly based on your daily closing balance and credited automatically.");
    fq.exec();

    fq.addBindValue("How can I reset my password?");
    fq.addBindValue("For this demo app, passwords are fixed. In a real system, you'd use email-based password reset.");
    fq.exec();
}

bool DBManager::createUser(const QString &email,
                           const QString &password,
                           const QString &username,
                           const QDate &dob) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO users (email, password, username, dob) "
              "VALUES (:email, :password, :username, :dob)");
    q.bindValue(":email", email.trimmed());
    q.bindValue(":password", password); // NOTE: plain text for demo only
    q.bindValue(":username", username.trimmed());
    q.bindValue(":dob", dob.isValid() ? dob.toString("yyyy-MM-dd") : QString());
    if (!q.exec()) {
        qWarning() << "Failed to create user:" << q.lastError().text();
        return false;
    }
    return true;
}

int DBManager::authenticateUser(const QString &email,
                                const QString &password) {
    QSqlQuery q(m_db);
    q.prepare("SELECT id FROM users WHERE email = :email AND password = :password");
    q.bindValue(":email", email.trimmed());
    q.bindValue(":password", password);
    if (!q.exec()) {
        qWarning() << "Auth query failed:" << q.lastError().text();
        return -1;
    }
    if (q.next()) {
        return q.value(0).toInt();
    }
    return -1;
}

QString DBManager::generateAccountNumber() {
    // Start with 9825 and then random 6 digits (unsequenced feel)
    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(100000, 999999);
    int randPart = dist(rng);
    return QString("9825%1").arg(randPart);
}

QString DBManager::generateCardNumber() {
    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(10000000, 99999999);
    int p1 = dist(rng);
    int p2 = dist(rng);
    return QString("%1%2").arg(p1).arg(p2);
}

int DBManager::createAccount(int userId,
                             const QString &type,
                             double initialBalance,
                             double interestRate) {
    QString accNum = generateAccountNumber();

    QSqlQuery q(m_db);
    q.prepare("INSERT INTO accounts "
              "(user_id, account_number, type, balance, interest_rate, last_interest_applied) "
              "VALUES (:user_id, :acc, :type, :bal, :rate, :last)");
    q.bindValue(":user_id", userId);
    q.bindValue(":acc", accNum);
    q.bindValue(":type", type);
    q.bindValue(":bal", initialBalance);
    q.bindValue(":rate", interestRate);
    q.bindValue(":last", QDate::currentDate().toString("yyyy-MM-dd"));

    if (!q.exec()) {
        qWarning() << "Failed to create account:" << q.lastError().text();
        return -1;
    }
    return q.lastInsertId().toInt();
}

bool DBManager::deposit(int accountId, double amount) {
    if (amount <= 0) return false;
    QSqlQuery q(m_db);
    q.prepare("UPDATE accounts SET balance = balance + :amt WHERE id = :id");
    q.bindValue(":amt", amount);
    q.bindValue(":id", accountId);
    if (!q.exec()) return false;

    QSqlQuery t(m_db);
    t.prepare("INSERT INTO transactions (account_id, type, amount, description) "
              "VALUES (:acc, 'Deposit', :amt, 'Cash deposit')");
    t.bindValue(":acc", accountId);
    t.bindValue(":amt", amount);
    t.exec();
    return true;
}

bool DBManager::withdraw(int accountId, double amount) {
    if (amount <= 0) return false;

    QSqlQuery balQ(m_db);
    balQ.prepare("SELECT balance FROM accounts WHERE id = :id");
    balQ.bindValue(":id", accountId);
    if (!balQ.exec() || !balQ.next()) return false;
    double balance = balQ.value(0).toDouble();
    if (balance < amount) return false;

    QSqlQuery q(m_db);
    q.prepare("UPDATE accounts SET balance = balance - :amt WHERE id = :id");
    q.bindValue(":amt", amount);
    q.bindValue(":id", accountId);
    if (!q.exec()) return false;

    QSqlQuery t(m_db);
    t.prepare("INSERT INTO transactions (account_id, type, amount, description) "
              "VALUES (:acc, 'Withdrawal', :amt, 'Cash withdrawal')");
    t.bindValue(":acc", accountId);
    t.bindValue(":amt", amount);
    t.exec();

    return true;
}

bool DBManager::transferAccountToAccount(int fromAccountId, int toAccountId, double amount) {
    if (amount <= 0 || fromAccountId == toAccountId) return false;

    m_db.transaction();

    QSqlQuery balQ(m_db);
    balQ.prepare("SELECT balance FROM accounts WHERE id = :id");
    balQ.bindValue(":id", fromAccountId);
    if (!balQ.exec() || !balQ.next()) {
        m_db.rollback();
        return false;
    }
    double balance = balQ.value(0).toDouble();
    if (balance < amount) {
        m_db.rollback();
        return false;
    }

    QSqlQuery debit(m_db);
    debit.prepare("UPDATE accounts SET balance = balance - :amt WHERE id = :id");
    debit.bindValue(":amt", amount);
    debit.bindValue(":id", fromAccountId);
    if (!debit.exec()) { m_db.rollback(); return false; }

    QSqlQuery credit(m_db);
    credit.prepare("UPDATE accounts SET balance = balance + :amt WHERE id = :id");
    credit.bindValue(":amt", amount);
    credit.bindValue(":id", toAccountId);
    if (!credit.exec()) { m_db.rollback(); return false; }

    QSqlQuery t1(m_db);
    t1.prepare("INSERT INTO transactions (account_id, type, amount, description, related_account_id) "
               "VALUES (:acc, 'Transfer Out', :amt, 'Transfer to another account', :rel)");
    t1.bindValue(":acc", fromAccountId);
    t1.bindValue(":amt", amount);
    t1.bindValue(":rel", toAccountId);
    t1.exec();

    QSqlQuery t2(m_db);
    t2.prepare("INSERT INTO transactions (account_id, type, amount, description, related_account_id) "
               "VALUES (:acc, 'Transfer In', :amt, 'Transfer from another account', :rel)");
    t2.bindValue(":acc", toAccountId);
    t2.bindValue(":amt", amount);
    t2.bindValue(":rel", fromAccountId);
    t2.exec();

    m_db.commit();
    return true;
}

bool DBManager::registerInteracEmail(int userId, int accountId, const QString &email) {
    QSqlQuery q(m_db);
    q.prepare("INSERT OR REPLACE INTO interac_registrations (user_id, account_id, email) "
              "VALUES (:user, :acc, :email)");
    q.bindValue(":user", userId);
    q.bindValue(":acc", accountId);
    q.bindValue(":email", email.trimmed().toLower());
    if (!q.exec()) {
        qWarning() << "Failed to register Interac:" << q.lastError().text();
        return false;
    }
    return true;
}

bool DBManager::interacTransfer(int fromAccountId, const QString &toEmail, double amount) {
    if (amount <= 0) return false;

    QSqlQuery find(m_db);
    find.prepare("SELECT account_id FROM interac_registrations WHERE email = :email");
    find.bindValue(":email", toEmail.trimmed().toLower());
    if (!find.exec() || !find.next()) {
        return false; // recipient not registered
    }
    int destAccountId = find.value(0).toInt();

    if (!transferAccountToAccount(fromAccountId, destAccountId, amount)) {
        return false;
    }

    // Override descriptions for clarity
    QSqlQuery t(m_db);
    t.prepare("INSERT INTO transactions (account_id, type, amount, description, related_account_id, interac_email) "
              "VALUES (:acc, 'Interac Out', :amt, 'Interac e-Transfer sent', :rel, :email)");
    t.bindValue(":acc", fromAccountId);
    t.bindValue(":amt", amount);
    t.bindValue(":rel", destAccountId);
    t.bindValue(":email", toEmail.trimmed().toLower());
    t.exec();

    QSqlQuery t2(m_db);
    t2.prepare("INSERT INTO transactions (account_id, type, amount, description, related_account_id, interac_email) "
               "VALUES (:acc, 'Interac In', :amt, 'Interac e-Transfer received', :rel, :email)");
    t2.bindValue(":acc", destAccountId);
    t2.bindValue(":amt", amount);
    t2.bindValue(":rel", fromAccountId);
    t2.bindValue(":email", toEmail.trimmed().toLower());
    t2.exec();

    return true;
}

int DBManager::applyForCreditCard(int userId, double creditLimit) {
    if (creditLimit < 2000.0) creditLimit = 2000.0; // minimum limit

    QString cardNumber = generateCardNumber();

    // Simple CVV and expiry
    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> cvvDist(100, 999);
    int cvv = cvvDist(rng);

    QDate today = QDate::currentDate();
    int expiryMonth = today.month();
    int expiryYear = today.year() + 3;

    QSqlQuery q(m_db);
    q.prepare("INSERT INTO credit_cards "
              "(user_id, card_number, cvv, expiry_month, expiry_year, credit_limit, current_balance, min_payment) "
              "VALUES (:user, :card, :cvv, :mm, :yy, :limit, 0, 0)");
    q.bindValue(":user", userId);
    q.bindValue(":card", cardNumber);
    q.bindValue(":cvv", QString::number(cvv));
    q.bindValue(":mm", expiryMonth);
    q.bindValue(":yy", expiryYear);
    q.bindValue(":limit", creditLimit);

    if (!q.exec()) {
        qWarning() << "Failed to create credit card:" << q.lastError().text();
        return -1;
    }
    return q.lastInsertId().toInt();
}

bool DBManager::payBill(int userId, int fromAccountId, int payeeId, double amount) {
    if (amount <= 0) return false;

    m_db.transaction();

    QSqlQuery balQ(m_db);
    balQ.prepare("SELECT balance FROM accounts WHERE id = :id");
    balQ.bindValue(":id", fromAccountId);
    if (!balQ.exec() || !balQ.next()) {
        m_db.rollback();
        return false;
    }
    double balance = balQ.value(0).toDouble();
    if (balance < amount) {
        m_db.rollback();
        return false;
    }

    QSqlQuery upd(m_db);
    upd.prepare("UPDATE accounts SET balance = balance - :amt WHERE id = :id");
    upd.bindValue(":amt", amount);
    upd.bindValue(":id", fromAccountId);
    if (!upd.exec()) { m_db.rollback(); return false; }

    QSqlQuery bp(m_db);
    bp.prepare("INSERT INTO bill_payments (user_id, from_account_id, payee_id, amount, reference) "
               "VALUES (:user, :acc, :payee, :amt, :ref)");
    bp.bindValue(":user", userId);
    bp.bindValue(":acc", fromAccountId);
    bp.bindValue(":payee", payeeId);
    bp.bindValue(":amt", amount);
    bp.bindValue(":ref", QString("Online bill payment"));
    if (!bp.exec()) { m_db.rollback(); return false; }

    QSqlQuery t(m_db);
    t.prepare("INSERT INTO transactions (account_id, type, amount, description) "
              "VALUES (:acc, 'Bill Payment', :amt, 'Bill payment to registered payee')");
    t.bindValue(":acc", fromAccountId);
    t.bindValue(":amt", amount);
    t.exec();

    m_db.commit();
    return true;
}

bool DBManager::spendOnCard(int cardId, double amount) {
    if (amount <= 0) return false;

    // Check limit
    QSqlQuery q(m_db);
    q.prepare("SELECT credit_limit, current_balance FROM credit_cards WHERE id = :id");
    q.bindValue(":id", cardId);
    if (!q.exec() || !q.next()) return false;

    double limit = q.value(0).toDouble();
    double bal   = q.value(1).toDouble();

    if (bal + amount > limit) {
        return false; // would exceed limit
    }

    QSqlQuery upd(m_db);
    upd.prepare("UPDATE credit_cards SET current_balance = current_balance + :amt WHERE id = :id");
    upd.bindValue(":amt", amount);
    upd.bindValue(":id", cardId);
    return upd.exec();
}

bool DBManager::payCreditCard(int userId, int fromAccountId, int cardId, double amount) {
    if (amount <= 0) return false;

    m_db.transaction();

    // Check account balance
    QSqlQuery balQ(m_db);
    balQ.prepare("SELECT balance FROM accounts WHERE id = :id");
    balQ.bindValue(":id", fromAccountId);
    if (!balQ.exec() || !balQ.next()) {
        m_db.rollback();
        return false;
    }
    double balance = balQ.value(0).toDouble();
    if (balance < amount) {
        m_db.rollback();
        return false;
    }

    // Check card balance
    QSqlQuery cardQ(m_db);
    cardQ.prepare("SELECT current_balance FROM credit_cards WHERE id = :id AND user_id = :user");
    cardQ.bindValue(":id", cardId);
    cardQ.bindValue(":user", userId);
    if (!cardQ.exec() || !cardQ.next()) {
        m_db.rollback();
        return false;
    }
    double cardBal = cardQ.value(0).toDouble();
    if (amount > cardBal) amount = cardBal; // cap to outstanding

    // Debit account
    QSqlQuery updAcc(m_db);
    updAcc.prepare("UPDATE accounts SET balance = balance - :amt WHERE id = :id");
    updAcc.bindValue(":amt", amount);
    updAcc.bindValue(":id", fromAccountId);
    if (!updAcc.exec()) {
        m_db.rollback();
        return false;
    }

    // Credit card
    QSqlQuery updCard(m_db);
    updCard.prepare("UPDATE credit_cards SET current_balance = current_balance - :amt WHERE id = :id");
    updCard.bindValue(":amt", amount);
    updCard.bindValue(":id", cardId);
    if (!updCard.exec()) {
        m_db.rollback();
        return false;
    }

    // Record as a transaction on the bank account
    QSqlQuery t(m_db);
    t.prepare("INSERT INTO transactions (account_id, type, amount, description) "
              "VALUES (:acc, 'Credit Card Payment', :amt, 'Payment to credit card')");
    t.bindValue(":acc", fromAccountId);
    t.bindValue(":amt", amount);
    t.exec();

    m_db.commit();
    return true;
}

void DBManager::applyMonthlyInterestInternal(int accountId,
                                             double interestRate,
                                             const QDate &lastApplied,
                                             const QDate &today) {
    if (interestRate <= 0.0) return;
    if (!lastApplied.isValid()) return;

    int monthsDiff = (lastApplied.daysTo(today)) / 30;
    if (monthsDiff <= 0) return;

    QSqlQuery balQ(m_db);
    balQ.prepare("SELECT balance FROM accounts WHERE id = :id");
    balQ.bindValue(":id", accountId);
    if (!balQ.exec() || !balQ.next()) return;
    double balance = balQ.value(0).toDouble();

    double monthlyRate = interestRate / 12.0;
    for (int i = 0; i < monthsDiff; ++i) {
        balance += balance * monthlyRate;
    }

    QSqlQuery upd(m_db);
    upd.prepare("UPDATE accounts SET balance = :bal, last_interest_applied = :last WHERE id = :id");
    upd.bindValue(":bal", balance);
    upd.bindValue(":last", today.toString("yyyy-MM-dd"));
    upd.bindValue(":id", accountId);
    upd.exec();

    // Record transaction
    QSqlQuery t(m_db);
    t.prepare("INSERT INTO transactions (account_id, type, amount, description) "
              "VALUES (:acc, 'Interest', 0, 'Monthly interest credited')");
    t.bindValue(":acc", accountId);
    t.exec();
}

void DBManager::applyMonthlyInterestForUser(int userId) {
    QSqlQuery q(m_db);
    q.prepare("SELECT id, interest_rate, last_interest_applied FROM accounts "
              "WHERE user_id = :user AND interest_rate > 0");
    q.bindValue(":user", userId);
    if (!q.exec()) return;

    QDate today = QDate::currentDate();
    while (q.next()) {
        int accId = q.value(0).toInt();
        double rate = q.value(1).toDouble();
        QDate last = QDate::fromString(q.value(2).toString(), "yyyy-MM-dd");
        applyMonthlyInterestInternal(accId, rate, last, today);
    }
}

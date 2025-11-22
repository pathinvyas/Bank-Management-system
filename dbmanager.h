#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QDateTime>

class DBManager {
public:
    static bool init(const QString &dbPath = "bank.db");
    static QSqlDatabase database();

    // User management
    static bool createUser(const QString &email,
                           const QString &password,
                           const QString &username,
                           const QDate &dob);
    static int authenticateUser(const QString &email,
                                const QString &password); // returns userId or -1

    // Interest
    static void applyMonthlyInterestForUser(int userId);

    // Accounts
    static int createAccount(int userId,
                             const QString &type,
                             double initialBalance,
                             double interestRate);

    static bool deposit(int accountId, double amount);
    static bool withdraw(int accountId, double amount);
    static bool transferAccountToAccount(int fromAccountId, int toAccountId, double amount);

    // Interac
    static bool registerInteracEmail(int userId, int accountId, const QString &email);
    static bool interacTransfer(int fromAccountId, const QString &toEmail, double amount);

    // Credit card
    static int applyForCreditCard(int userId, double creditLimit);

    // Bill payment
    static bool payBill(int userId, int fromAccountId, int payeeId, double amount);

    // Credit card operations
    static bool spendOnCard(int cardId, double amount);
    static bool payCreditCard(int userId, int fromAccountId, int cardId, double amount);

    // Helpers
    static QString generateAccountNumber();
    static QString generateCardNumber();

private:
    static void createTablesIfNeeded();
    static void createSampleDataIfEmpty();
    static void applyMonthlyInterestInternal(int accountId,
                                             double interestRate,
                                             const QDate &lastApplied,
                                             const QDate &today);

    static QSqlDatabase m_db;
    static int m_nextAccountSeed;
};

#endif // DBMANAGER_H
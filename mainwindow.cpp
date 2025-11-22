#include "mainwindow.h"
#include "dbmanager.h"

#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QComboBox>
#include <QLineEdit>
#include <QHeaderView>
#include <QListWidget>
#include <QSplitter>
#include <QDate>
#include <QMessageBox>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QTextDocument>
#include <QFileDialog>
#include <QEvent>
#include <QMouseEvent>


MainWindow::MainWindow(int userId, QWidget *parent)
    : QMainWindow(parent),
      m_userId(userId),
      m_tabs(new QTabWidget(this)),
      m_accountsTable(nullptr),
      m_accountTypeCombo(nullptr),
      m_initialDepositEdit(nullptr),
      m_savingsRateEdit(nullptr),
      m_depositAccountCombo(nullptr),
      m_depositAmountEdit(nullptr),
      m_withdrawAccountCombo(nullptr),
      m_withdrawAmountEdit(nullptr),
      m_transferFromCombo(nullptr),
      m_transferToCombo(nullptr),
      m_transferAmountEdit(nullptr),
      m_interacFromCombo(nullptr),
      m_interacEmailEdit(nullptr),
      m_interacAmountEdit(nullptr),
      m_cardsTable(nullptr),
      m_cardLimitEdit(nullptr),
      m_cardSpendCardCombo(nullptr),
      m_cardSpendAmountEdit(nullptr),
      m_cardPayFromAccountCombo(nullptr),
      m_cardPayCardCombo(nullptr),
      m_cardPayAmountEdit(nullptr),
      m_billFromCombo(nullptr),
      m_billPayeeCombo(nullptr),
      m_billAmountEdit(nullptr),
      m_overviewBalanceLabel(nullptr),
      m_overviewSavingsLabel(nullptr),
      m_statementsTable(nullptr),
      m_statementsAccountCombo(nullptr),
      m_faqList(nullptr)
{
    setWindowTitle("Sudbury Student Bank – Dashboard");
    resize(1180, 720);

    m_tabs->addTab(buildOverviewTab(),   "Overview");
    m_tabs->addTab(buildAccountsTab(),   "Accounts");
    m_tabs->addTab(buildTransfersTab(),  "Transfers");
    m_tabs->addTab(buildCardsTab(),      "Credit Cards");
    m_tabs->addTab(buildBillsTab(),      "Bill Payments");
    m_tabs->addTab(buildStatementsTab(), "Statements");
    m_tabs->addTab(buildProfileTab(),    "Profile");
    m_tabs->addTab(buildFaqTab(),        "FAQs");

    setCentralWidget(m_tabs);

    DBManager::applyMonthlyInterestForUser(m_userId);
    refreshOverview();
    refreshAccountsTables();
    refreshCreditCards();
    refreshStatements();
    refreshFaqs();
    refreshBillPayees();

    // ----------------------------------------
    // Logout Button (top-right corner)
    // ----------------------------------------
    logoutButton = new QPushButton("Logout", this);
    logoutButton->setFixedSize(100, 36);
    logoutButton->move(width() - 120, 12);

    logoutButton->setStyleSheet(
        "background-color:#d9534f;"
        "color:white;"
        "border-radius:8px;"
        "font-weight:bold;"
        );

    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::handleLogout);

}  // <-- keep this closing brace


QWidget* MainWindow::buildOverviewTab() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *header = new QLabel("Good to see you again 👋", page);
    header->setObjectName("pageTitle");

    m_overviewBalanceLabel = new QLabel("Total balance: $0.00", page);
    m_overviewBalanceLabel->setObjectName("overviewMetric");

    m_overviewSavingsLabel = new QLabel("Savings balance: $0.00", page);
    m_overviewSavingsLabel->setObjectName("overviewMetricSecondary");
    layout->addWidget(header);
    layout->addSpacing(8);
    layout->addWidget(m_overviewBalanceLabel);
    layout->addWidget(m_overviewSavingsLabel);
    layout->addStretch();

    return page;
}

QWidget* MainWindow::buildAccountsTab() {
    auto *page = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(page);

    auto *title = new QLabel("Accounts & Balances", page);
    title->setObjectName("pageTitle");

    m_accountsTable = new QTableView(page);
    m_accountsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_accountsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_accountsTable->setSelectionMode(QAbstractItemView::SingleSelection);

    auto *split = new QSplitter(Qt::Vertical, page);
    split->addWidget(m_accountsTable);

    auto *actionsWidget = new QWidget(page);
    auto *actionsLayout = new QHBoxLayout(actionsWidget);

    // Create account form
    auto *createBox = new QWidget(actionsWidget);
    auto *createLayout = new QFormLayout(createBox);
    createLayout->setLabelAlignment(Qt::AlignRight);

    m_accountTypeCombo = new QComboBox(createBox);
    m_accountTypeCombo->addItems({"Chequing", "Savings"});

    m_initialDepositEdit = new QLineEdit(createBox);
    m_initialDepositEdit->setPlaceholderText("e.g. 100.00");

    m_savingsRateEdit = new QLineEdit(createBox);
    m_savingsRateEdit->setPlaceholderText("Savings rate is defaulted");
    m_savingsRateEdit->setText("0.012");
    m_savingsRateEdit->setReadOnly(true);

    auto *createBtn = new QPushButton("Create account", createBox);

    createLayout->addRow("Type:", m_accountTypeCombo);
    createLayout->addRow("Initial deposit:", m_initialDepositEdit);
    createLayout->addRow("Savings rate:", m_savingsRateEdit);
    createLayout->addRow("", createBtn);

    // Deposit form
    auto *depositBox = new QWidget(actionsWidget);
    auto *depositLayout = new QFormLayout(depositBox);

    m_depositAccountCombo = new QComboBox(depositBox);
    m_depositAmountEdit = new QLineEdit(depositBox);
    m_depositAmountEdit->setPlaceholderText("Amount");
    auto *depositBtn = new QPushButton("Deposit", depositBox);

    depositLayout->addRow("Account:", m_depositAccountCombo);
    depositLayout->addRow("Amount:", m_depositAmountEdit);
    depositLayout->addRow("", depositBtn);

    // Withdraw form
    auto *withdrawBox = new QWidget(actionsWidget);
    auto *withdrawLayout = new QFormLayout(withdrawBox);

    m_withdrawAccountCombo = new QComboBox(withdrawBox);
    m_withdrawAmountEdit = new QLineEdit(withdrawBox);
    m_withdrawAmountEdit->setPlaceholderText("Amount");
    auto *withdrawBtn = new QPushButton("Withdraw", withdrawBox);

    withdrawLayout->addRow("Account:", m_withdrawAccountCombo);
    withdrawLayout->addRow("Amount:", m_withdrawAmountEdit);
    withdrawLayout->addRow("", withdrawBtn);

    actionsLayout->addWidget(createBox);
    actionsLayout->addWidget(depositBox);
    actionsLayout->addWidget(withdrawBox);

    split->addWidget(actionsWidget);

    rootLayout->addWidget(title);
    rootLayout->addWidget(split);

    connect(createBtn,    &QPushButton::clicked, this, &MainWindow::createNewAccount);
    connect(depositBtn,   &QPushButton::clicked, this, &MainWindow::handleDeposit);
    connect(withdrawBtn,  &QPushButton::clicked, this, &MainWindow::handleWithdraw);

    return page;
}

QWidget* MainWindow::buildTransfersTab() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *title = new QLabel("Transfers & Interac", page);
    title->setObjectName("pageTitle");

    auto *split = new QSplitter(Qt::Horizontal, page);

    // Internal transfers
    auto *internalBox = new QWidget(split);
    auto *internalLayout = new QFormLayout(internalBox);
    internalLayout->setLabelAlignment(Qt::AlignRight);

    m_transferFromCombo = new QComboBox(internalBox);
    m_transferToCombo   = new QComboBox(internalBox);
    m_transferAmountEdit = new QLineEdit(internalBox);
    m_transferAmountEdit->setPlaceholderText("Amount");

    auto *internalBtn = new QPushButton("Transfer between my accounts", internalBox);

    internalLayout->addRow("From account:", m_transferFromCombo);
    internalLayout->addRow("To account:",   m_transferToCombo);
    internalLayout->addRow("Amount:",       m_transferAmountEdit);
    internalLayout->addRow("",              internalBtn);

    // Interac transfers
    auto *interacBox = new QWidget(split);
    auto *interacLayout = new QFormLayout(interacBox);
    interacLayout->setLabelAlignment(Qt::AlignRight);

    m_interacFromCombo = new QComboBox(interacBox);
    m_interacEmailEdit = new QLineEdit(interacBox);
    m_interacEmailEdit->setPlaceholderText("friend@example.com");

    m_interacAmountEdit = new QLineEdit(interacBox);
    m_interacAmountEdit->setPlaceholderText("Amount");

    auto *interacBtn = new QPushButton("Interac e-Transfer", interacBox);

    interacLayout->addRow("From account:", m_interacFromCombo);
    interacLayout->addRow("Recipient email:", m_interacEmailEdit);
    interacLayout->addRow("Amount:", m_interacAmountEdit);
    interacLayout->addRow("", interacBtn);

    split->addWidget(internalBox);
    split->addWidget(interacBox);

    layout->addWidget(title);
    layout->addWidget(split);

    connect(internalBtn, &QPushButton::clicked, this, &MainWindow::handleInternalTransfer);
    connect(interacBtn,  &QPushButton::clicked, this, &MainWindow::handleInteracTransfer);

    return page;
}

QWidget* MainWindow::buildCardsTab() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *title = new QLabel("Credit Cards", page);
    title->setObjectName("pageTitle");

    m_cardsTable = new QTableView(page);
    m_cardsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_cardsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    auto *formBox = new QWidget(page);
    auto *formLayout = new QFormLayout(formBox);

    // Apply for new card
    m_cardLimitEdit = new QLineEdit(formBox);
    m_cardLimitEdit->setPlaceholderText("Minimum 2000 CAD");
    auto *applyBtn = new QPushButton("Apply for new credit card", formBox);
    formLayout->addRow("Requested limit:", m_cardLimitEdit);
    formLayout->addRow("", applyBtn);

    // Spend on card
    m_cardSpendCardCombo = new QComboBox(formBox);
    m_cardSpendAmountEdit = new QLineEdit(formBox);
    m_cardSpendAmountEdit->setPlaceholderText("Amount to spend");
    auto *spendBtn = new QPushButton("Simulate card purchase", formBox);
    formLayout->addRow("Card to spend:", m_cardSpendCardCombo);
    formLayout->addRow("Purchase amount:", m_cardSpendAmountEdit);
    formLayout->addRow("", spendBtn);

    // Pay card from bank account
    m_cardPayFromAccountCombo = new QComboBox(formBox);
    m_cardPayCardCombo = new QComboBox(formBox);
    m_cardPayAmountEdit = new QLineEdit(formBox);
    m_cardPayAmountEdit->setPlaceholderText("Payment amount");
    auto *payBtn = new QPushButton("Pay credit card", formBox);
    formLayout->addRow("Pay from account:", m_cardPayFromAccountCombo);
    formLayout->addRow("Card to pay:", m_cardPayCardCombo);
    formLayout->addRow("Payment amount:", m_cardPayAmountEdit);
    formLayout->addRow("", payBtn);

    layout->addWidget(title);
    layout->addWidget(m_cardsTable);
    layout->addWidget(formBox);

    connect(applyBtn, &QPushButton::clicked, this, &MainWindow::handleApplyCreditCard);
    connect(spendBtn, &QPushButton::clicked, this, &MainWindow::handleCardSpend);
    connect(payBtn, &QPushButton::clicked, this, &MainWindow::handleCardPayment);

    return page;
}

QWidget* MainWindow::buildBillsTab() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *title = new QLabel("Bill Payments", page);
    title->setObjectName("pageTitle");

    auto *formBox = new QWidget(page);
    auto *formLayout = new QFormLayout(formBox);

    m_billFromCombo = new QComboBox(formBox);
    m_billPayeeCombo = new QComboBox(formBox);
    m_billAmountEdit = new QLineEdit(formBox);
    m_billAmountEdit->setPlaceholderText("Enter amount");

    auto *payBtn = new QPushButton("Pay Bill", formBox);

    formLayout->addRow("From account:", m_billFromCombo);
    formLayout->addRow("Payee:", m_billPayeeCombo);
    formLayout->addRow("Amount:", m_billAmountEdit);
    formLayout->addRow("", payBtn);

    layout->addWidget(title);
    layout->addWidget(formBox);
    layout->addStretch();

    connect(payBtn, &QPushButton::clicked, this, &MainWindow::handleBillPayment);

    return page;
}

QWidget* MainWindow::buildProfileTab() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *title = new QLabel("Profile & Personalization", page);
    title->setObjectName("pageTitle");

    // Pull data from DB
    QSqlQuery q(DBManager::database());
    q.prepare("SELECT username, email, dob, created_at FROM users WHERE id = :id");
    q.bindValue(":id", m_userId);
    QString name, email, dob, created;
    if (q.exec() && q.next()) {
        name    = q.value(0).toString();
        email   = q.value(1).toString();
        dob     = q.value(2).toString();
        created = q.value(3).toString();
    }

    auto *topRow = new QHBoxLayout();

    // --- CREATE the global avatar here ---
    avatar = new QLabel(page);
    avatar->setObjectName("profilePicFrame");
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setText(name.isEmpty() ? "BB" : name.left(2).toUpper());
    avatar->setMinimumSize(96, 96);
    avatar->setMaximumSize(96, 96);

    // Install event filter and cursor AFTER creation
    avatar->installEventFilter(this);
    avatar->setCursor(Qt::PointingHandCursor);

    // Summary text
    QString summary = QString("Name: %1\nEmail: %2\nDOB: %3\nClient since: %4")
                          .arg(name)
                          .arg(email)
                          .arg(dob)
                          .arg(created);

    auto *infoLabel = new QLabel(summary, page);
    infoLabel->setObjectName("profileSummary");
    infoLabel->setWordWrap(true);

    topRow->addWidget(avatar);
    topRow->addSpacing(16);
    topRow->addWidget(infoLabel, 1);

    auto *note = new QLabel("Smart Banking Client", page);
    note->setWordWrap(true);

    layout->addWidget(title);
    layout->addLayout(topRow);
    layout->addSpacing(12);
    layout->addWidget(note);
    layout->addStretch();

    return page;
}


QWidget* MainWindow::buildFaqTab() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *title = new QLabel("FAQs", page);
    title->setObjectName("pageTitle");

    m_faqList = new QListWidget(page);

    layout->addWidget(title);
    layout->addWidget(m_faqList);

    return page;
}

QWidget* MainWindow::buildStatementsTab() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *title = new QLabel("Account Statements", page);
    title->setObjectName("pageTitle");

    m_statementsAccountCombo = new QComboBox(page);
    m_statementsTable = new QTableView(page);
    m_statementsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // --- Export PDF button ---
    auto *exportBtn = new QPushButton("Export as PDF", page);
    connect(exportBtn, &QPushButton::clicked, this, [this]() {
        exportStatementsAsPdf();
    });

    layout->addWidget(title);
    layout->addWidget(m_statementsAccountCombo);
    layout->addWidget(exportBtn);          // <-- correct position
    layout->addWidget(m_statementsTable);

    connect(m_statementsAccountCombo, &QComboBox::currentIndexChanged,
            this, &MainWindow::refreshStatements);

    return page;
}


void MainWindow::refreshOverview() {
    DBManager::applyMonthlyInterestForUser(m_userId);

    QSqlQuery q(DBManager::database());
    q.prepare("SELECT type, balance FROM accounts WHERE user_id = :user");
    q.bindValue(":user", m_userId);
    double total = 0.0;
    double savings = 0.0;
    if (q.exec()) {
        while (q.next()) {
            QString type = q.value(0).toString();
            double bal = q.value(1).toDouble();
            total += bal;
            if (type.toLower().contains("sav")) savings += bal;
        }
    }
    m_overviewBalanceLabel->setText(QString("Total balance across all accounts: $%1").arg(total, 0, 'f', 2));
    m_overviewSavingsLabel->setText(QString("Total savings balance: $%1").arg(savings, 0, 'f', 2));
}

void MainWindow::createNewAccount() {
    QString type = m_accountTypeCombo->currentText();
    double initial = m_initialDepositEdit->text().toDouble();

    double rate = 0.0;
    if (type.toLower().contains("sav")) {
        // Default savings interest rate (e.g., 1.2% annually)
        rate = 0.012;
    }

    int id = DBManager::createAccount(m_userId, type, initial, rate);
    if (id > 0) {
        refreshAccountsTables();
        refreshOverview();
        QMessageBox::information(this, "Account created",
                                 "Your new " + type + " account has been created.");
    } else {
        QMessageBox::warning(this, "Account not created",
                             "We couldn't create this account. Please try again.");
    }
}

void MainWindow::handleDeposit() {
    int accountId = m_depositAccountCombo->currentData().toInt();
    double amount = m_depositAmountEdit->text().toDouble();
    if (DBManager::deposit(accountId, amount)) {
        refreshAccountsTables();
        refreshOverview();
        QMessageBox::information(this, "Deposit successful",
                                 "Your deposit was applied to the selected account.");
    } else {
        QMessageBox::warning(this, "Deposit failed",
                             "Deposit could not be completed. Check the amount and try again.");
    }
}

void MainWindow::handleWithdraw() {
    int accountId = m_withdrawAccountCombo->currentData().toInt();
    double amount = m_withdrawAmountEdit->text().toDouble();
    if (DBManager::withdraw(accountId, amount)) {
        refreshAccountsTables();
        refreshOverview();
        QMessageBox::information(this, "Withdrawal successful",
                                 "Cash withdrawal completed successfully.");
    } else {
        QMessageBox::warning(this, "Withdrawal failed",
                             "Withdrawal could not be completed. Check your balance and try again.");
    }
}

void MainWindow::handleInternalTransfer() {
    int fromId = m_transferFromCombo->currentData().toInt();
    int toId   = m_transferToCombo->currentData().toInt();
    double amount = m_transferAmountEdit->text().toDouble();
    if (DBManager::transferAccountToAccount(fromId, toId, amount)) {
        refreshAccountsTables();
        refreshOverview();
        QMessageBox::information(this, "Transfer successful",
                                 "Funds were moved between your accounts.");
    } else {
        QMessageBox::warning(this, "Transfer failed",
                             "Internal transfer could not be completed.");
    }
}

void MainWindow::handleInteracTransfer() {
    int fromId = m_interacFromCombo->currentData().toInt();
    QString email = m_interacEmailEdit->text();
    double amount = m_interacAmountEdit->text().toDouble();

    if (DBManager::interacTransfer(fromId, email, amount)) {
        refreshAccountsTables();
        refreshOverview();
        QMessageBox::information(this, "Interac sent",
                                 "Amount was sent successfully.");
    } else {
        QMessageBox::warning(this, "Interac failed",
                             "We couldn't complete this Interac transfer. "
                             "Check the recipient email and balance.");
    }
}

void MainWindow::handleApplyCreditCard() {
    double limit = m_cardLimitEdit->text().toDouble();
    int id = DBManager::applyForCreditCard(m_userId, limit);
    if (id > 0) {
        refreshCreditCards();
        QMessageBox::information(this, "Card approved",
                                 "Your new credit card has been created.");
    } else {
        QMessageBox::warning(this, "Application failed",
                             "We couldn't create a credit card with this request.");
    }
}

void MainWindow::handleBillPayment() {
    int fromId = m_billFromCombo->currentData().toInt();
    int payeeId = m_billPayeeCombo->currentData().toInt();
    double amount = m_billAmountEdit->text().toDouble();

    if (DBManager::payBill(m_userId, fromId, payeeId, amount)) {
        refreshAccountsTables();
        refreshOverview();
        QMessageBox::information(this, "Bill paid",
                                 "Your bill payment was submitted successfully.");
    } else {
        QMessageBox::warning(this, "Bill payment failed",
                             "We couldn't complete this bill payment. Check balance and amount.");
    }
}

void MainWindow::handleCardSpend() {
    int cardId = m_cardSpendCardCombo->currentData().toInt();
    double amount = m_cardSpendAmountEdit->text().toDouble();
    if (DBManager::spendOnCard(cardId, amount)) {
        refreshCreditCards();
        QMessageBox::information(this, "Purchase simulated",
                                 "The amount was added to your card balance.");
    } else {
        QMessageBox::warning(this, "Purchase failed",
                             "Card purchase could not be simulated. Check limit and amount.");
    }
}

void MainWindow::handleCardPayment() {
    int fromAccountId = m_cardPayFromAccountCombo->currentData().toInt();
    int cardId = m_cardPayCardCombo->currentData().toInt();
    double amount = m_cardPayAmountEdit->text().toDouble();
    if (DBManager::payCreditCard(m_userId, fromAccountId, cardId, amount)) {
        refreshCreditCards();
        refreshAccountsTables();
        refreshOverview();
        QMessageBox::information(this, "Payment posted",
                                 "Your credit card payment has been applied.");
    } else {
        QMessageBox::warning(this, "Payment failed",
                             "We couldn't process this payment. Check balances and try again.");
    }
}

void MainWindow::refreshAccountsTables() {
    // Table model
    auto *model = new QSqlQueryModel(this);
    QSqlQuery q(DBManager::database());
    q.prepare("SELECT account_number AS 'Account', type AS 'Type', "
              "printf('%.2f', balance) AS 'Balance', "
              "printf('%.3f', interest_rate) AS 'Rate' "
              "FROM accounts WHERE user_id = :user");
    q.bindValue(":user", m_userId);
    q.exec();
    model->setQuery(q);
    m_accountsTable->setModel(model);

    // Fill combo boxes with account id + display text
    auto fillCombo = [this](QComboBox *combo) {
        combo->clear();
        QSqlQuery query(DBManager::database());
        query.prepare("SELECT id, account_number, type FROM accounts WHERE user_id = :user");
        query.bindValue(":user", m_userId);
        if (query.exec()) {
            while (query.next()) {
                int id = query.value(0).toInt();
                QString label = QString("%1 (%2)").arg(query.value(1).toString(), query.value(2).toString());
                combo->addItem(label, id);
            }
        }
    };

    fillCombo(m_depositAccountCombo);
    fillCombo(m_withdrawAccountCombo);
    fillCombo(m_transferFromCombo);
    fillCombo(m_transferToCombo);
    fillCombo(m_interacFromCombo);
    fillCombo(m_billFromCombo);

    // Statements account combo
    m_statementsAccountCombo->clear();
    QSqlQuery q2(DBManager::database());
    q2.prepare("SELECT id, account_number, type FROM accounts WHERE user_id = :user");
    q2.bindValue(":user", m_userId);
    if (q2.exec()) {
        while (q2.next()) {
            int id = q2.value(0).toInt();
            QString label = QString("%1 (%2)").arg(q2.value(1).toString(), q2.value(2).toString());
            m_statementsAccountCombo->addItem(label, id);
        }
    }
}

void MainWindow::refreshCreditCards() {
    auto *model = new QSqlQueryModel(this);
    QSqlQuery q(DBManager::database());
    q.prepare("SELECT id, card_number AS 'Card', "
              "printf('%.2f', credit_limit) AS 'Limit', "
              "printf('%.2f', current_balance) AS 'Balance', "
              "status AS 'Status' "
              "FROM credit_cards WHERE user_id = :user");
    q.bindValue(":user", m_userId);
    q.exec();
    model->setQuery(q);
    m_cardsTable->setModel(model);
    m_cardsTable->hideColumn(0); // internal id

    // Fill combos for card actions
    m_cardSpendCardCombo->clear();
    m_cardPayCardCombo->clear();

    QSqlQuery q2(DBManager::database());
    q2.prepare("SELECT id, card_number FROM credit_cards WHERE user_id = :user");
    q2.bindValue(":user", m_userId);
    if (q2.exec()) {
        while (q2.next()) {
            int id = q2.value(0).toInt();
            QString label = q2.value(1).toString();
            m_cardSpendCardCombo->addItem(label, id);
            m_cardPayCardCombo->addItem(label, id);
        }
    }

    // Pay from account combo uses accounts of this user
    m_cardPayFromAccountCombo->clear();
    QSqlQuery q3(DBManager::database());
    q3.prepare("SELECT id, account_number, type FROM accounts WHERE user_id = :user");
    q3.bindValue(":user", m_userId);
    if (q3.exec()) {
        while (q3.next()) {
            int id = q3.value(0).toInt();
            QString label = QString("%1 (%2)").arg(q3.value(1).toString(), q3.value(2).toString());
            m_cardPayFromAccountCombo->addItem(label, id);
        }
    }
}

void MainWindow::refreshStatements() {
    int accountId = m_statementsAccountCombo->currentData().toInt();
    if (accountId <= 0) return;

    auto *model = new QSqlQueryModel(this);
    QSqlQuery q(DBManager::database());
    q.prepare("SELECT timestamp AS 'When', type AS 'Type', "
              "printf('%.2f', amount) AS 'Amount', description AS 'Description' "
              "FROM transactions WHERE account_id = :acc "
              "ORDER BY datetime(timestamp) DESC LIMIT 100");
    q.bindValue(":acc", accountId);
    q.exec();
    model->setQuery(q);
    m_statementsTable->setModel(model);
}

void MainWindow::refreshBillPayees() {
    if (!m_billPayeeCombo) return;
    m_billPayeeCombo->clear();
    QSqlQuery q(DBManager::database());
    q.prepare("SELECT id, name, category FROM bill_payees ORDER BY name");
    if (q.exec()) {
        while (q.next()) {
            int id = q.value(0).toInt();
            QString name = q.value(1).toString();
            QString cat  = q.value(2).toString();
            QString label = cat.isEmpty() ? name : QString("%1 (%2)").arg(name, cat);
            m_billPayeeCombo->addItem(label, id);
        }
    }
}

void MainWindow::refreshFaqs() {
    m_faqList->clear();
    QSqlQuery q(DBManager::database());
    q.prepare("SELECT question, answer FROM faqs ORDER BY id");
    if (q.exec()) {
        while (q.next()) {
            QString text = QString("Q: %1\nA: %2")
                           .arg(q.value(0).toString())
                           .arg(q.value(1).toString());
            m_faqList->addItem(text);
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    if (logoutButton) {
        logoutButton->move(width() - 120, 12);
    }
}

void MainWindow::handleLogout() {
    QMessageBox::StandardButton reply =
        QMessageBox::question(this,
                              "Logout",
                              "Do you really want to logout?",
                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        close();

        // If you want to reopen login window, add here:
        // LoginWindow *login = new LoginWindow();
        // login->show();
    }
}


void MainWindow::exportStatementsAsPdf()
{
    int accountId = m_statementsAccountCombo->currentData().toInt();
    if (accountId <= 0) {
        QMessageBox::warning(this, "Export Failed", "Please select an account first.");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Statements as PDF",
        "statement.pdf",
        "PDF Files (*.pdf)"
        );
    if (filePath.isEmpty())
        return;

    // QPrinter in PDF mode
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15));

    // Build HTML contents
    QString html;
    html += "<h2>Sudbury Student Bank – Account Statement</h2>";
    html += "<p><b>Account:</b> " + m_statementsAccountCombo->currentText() + "</p>";
    html += "<hr>";

    html += "<table border='1' cellspacing='0' cellpadding='4' width='100%'>";
    html += "<tr style='background:#EEE; font-weight:bold;'>"
            "<td>Date</td><td>Type</td><td>Amount</td><td>Description</td>"
            "</tr>";

    QSqlQuery q(DBManager::database());
    q.prepare("SELECT timestamp, type, amount, description "
              "FROM transactions WHERE account_id = :acc "
              "ORDER BY datetime(timestamp) DESC");
    q.bindValue(":acc", accountId);
    q.exec();

    while (q.next()) {
        html += "<tr>";
        html += "<td>" + q.value(0).toString() + "</td>";
        html += "<td>" + q.value(1).toString() + "</td>";
        html += "<td>$" + QString::number(q.value(2).toDouble(), 'f', 2) + "</td>";
        html += "<td>" + q.value(3).toString() + "</td>";
        html += "</tr>";
    }

    html += "</table>";

    // Generate PDF using HTML
    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Export Successful",
                             "Your PDF statement has been generated successfully.");
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == avatar && event->type() == QEvent::MouseButtonPress) {
        QString file = QFileDialog::getOpenFileName(
            this,
            "Choose Profile Picture",
            "",
            "Images (*.png *.jpg *.jpeg *.bmp)"
            );

        if (!file.isEmpty()) {
            QPixmap pic(file);
            avatar->setPixmap(
                pic.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                );
            avatar->setText("");
        }
        return true; // event handled
    }

    return QMainWindow::eventFilter(obj, event);
}

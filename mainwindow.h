#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>

class QTabWidget;
class QTableView;
class QComboBox;
class QLineEdit;
class QLabel;
class QTextEdit;
class QListWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(int userId, QWidget *parent = nullptr);

private slots:
    void refreshOverview();
    void createNewAccount();
    void handleDeposit();
    void handleWithdraw();
    void handleInternalTransfer();
    void handleInteracTransfer();
    void handleApplyCreditCard();
    void handleBillPayment();
    void handleCardSpend();
    void handleCardPayment();
    void refreshAccountsTables();
    void refreshCreditCards();
    void refreshStatements();
    void refreshFaqs();
    void refreshBillPayees();

    void handleLogout();              // <-- LOGOUT
    void exportStatementsAsPdf();     // <-- NEW PDF EXPORT SLOT


private:
    QWidget* buildOverviewTab();
    QWidget* buildAccountsTab();
    QWidget* buildTransfersTab();
    QWidget* buildCardsTab();
    QWidget* buildBillsTab();
    QWidget* buildProfileTab();
    QWidget* buildFaqTab();
    QWidget* buildStatementsTab();

    void resizeEvent(QResizeEvent *event) override;   // <-- logout button positioning

    int m_userId;

    bool eventFilter(QObject *obj, QEvent *event) override;
    QLabel *avatar;  // reuse your existing avatar pointer


    QTabWidget *m_tabs;

    QTableView *m_accountsTable;
    QComboBox  *m_accountTypeCombo;
    QLineEdit  *m_initialDepositEdit;
    QLineEdit  *m_savingsRateEdit;

    QComboBox  *m_depositAccountCombo;
    QLineEdit  *m_depositAmountEdit;

    QComboBox  *m_withdrawAccountCombo;
    QLineEdit  *m_withdrawAmountEdit;

    QComboBox  *m_transferFromCombo;
    QComboBox  *m_transferToCombo;
    QLineEdit  *m_transferAmountEdit;

    QComboBox  *m_interacFromCombo;
    QLineEdit  *m_interacEmailEdit;
    QLineEdit  *m_interacAmountEdit;

    QTableView *m_cardsTable;
    QLineEdit  *m_cardLimitEdit;

    QComboBox  *m_cardSpendCardCombo;
    QLineEdit  *m_cardSpendAmountEdit;
    QComboBox  *m_cardPayFromAccountCombo;
    QComboBox  *m_cardPayCardCombo;
    QLineEdit  *m_cardPayAmountEdit;

    QComboBox  *m_billFromCombo;
    QComboBox  *m_billPayeeCombo;
    QLineEdit  *m_billAmountEdit;

    QLabel     *m_overviewBalanceLabel;
    QLabel     *m_overviewSavingsLabel;

    QTableView *m_statementsTable;
    QComboBox  *m_statementsAccountCombo;

    QListWidget *m_faqList;

    QPushButton *logoutButton;

};

#endif // MAINWINDOW_H

#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;
class QStackedWidget;
class QDateEdit;

class LoginWindow : public QWidget {
    Q_OBJECT
public:
    explicit LoginWindow(QWidget *parent = nullptr);

signals:
    void loginSucceeded(int userId);

private slots:
    void handleLogin();
    void handleSignup();
    void showSignup();
    void showLogin();

private:
    void buildUi();
    QWidget* buildLoginPage();
    QWidget* buildSignupPage();

    QStackedWidget *m_stack;

    // Login controls
    QLineEdit *m_loginEmail;
    QLineEdit *m_loginPassword;
    QLabel    *m_loginStatus;

    // Signup controls
    QLineEdit *m_signupEmail;
    QLineEdit *m_signupPassword;
    QLineEdit *m_signupUsername;
    QDateEdit *m_signupDob;
    QLabel    *m_signupStatus;
};

#endif // LOGINWINDOW_H
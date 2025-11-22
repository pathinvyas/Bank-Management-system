#include "loginwindow.h"
#include "dbmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDateEdit>
#include <QDate>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent),
      m_stack(new QStackedWidget(this)),
      m_loginEmail(nullptr),
      m_loginPassword(nullptr),
      m_loginStatus(nullptr),
      m_signupEmail(nullptr),
      m_signupPassword(nullptr),
      m_signupUsername(nullptr),
      m_signupDob(nullptr),
      m_signupStatus(nullptr)
{
    buildUi();
    setWindowTitle("Sudnury Studnt Bank");
    resize(900, 520);
}

void LoginWindow::buildUi() {
    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(40, 40, 40, 40);
    rootLayout->setSpacing(40);

    // Left branding panel
    auto *brandPanel = new QWidget(this);
    auto *brandLayout = new QVBoxLayout(brandPanel);
    brandLayout->setSpacing(24);

    auto *title = new QLabel("Sudbury Student Bank", brandPanel);
    title->setObjectName("brandTitle");

    auto *subtitle = new QLabel("Modern, student-ready banking simulation.\nSign in securely to manage your accounts.", brandPanel);
    subtitle->setWordWrap(true);

    auto *highlight = new QLabel("• A Smarter Way to Bank\n"
                                 "• Your Money, Always a Step Ahead\n"
                                 "• Where Security Meets Simplicity\n"
                                 "• Banking Designed Around You\n"
                                 "• Instant Transfers. Smarter Savings. Full Control\n"
                                 "• Your Digital Wallet, Reimagined\n"
                                 "• Your Finances, More Connected Than Ever\n"
                                 "• Banking That Works While You Do", brandPanel);
    highlight->setWordWrap(true);

    brandLayout->addWidget(title);
    brandLayout->addWidget(subtitle);
    brandLayout->addSpacing(20);
    brandLayout->addWidget(highlight);
    brandLayout->addStretch();

    // Right auth stack
    m_stack->addWidget(buildLoginPage());
    m_stack->addWidget(buildSignupPage());

    rootLayout->addWidget(brandPanel, 1);
    rootLayout->addWidget(m_stack, 1);
}

QWidget* LoginWindow::buildLoginPage() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setSpacing(16);

    auto *header = new QLabel("Welcome back", page);
    header->setObjectName("pageTitle");

    auto *caption = new QLabel("Sign in with your email and password to access.", page);
    caption->setWordWrap(true);

    auto *formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight);

    m_loginEmail = new QLineEdit(page);
    m_loginEmail->setPlaceholderText("you@example.com");

    m_loginPassword = new QLineEdit(page);
    m_loginPassword->setPlaceholderText("Enter your password");
    m_loginPassword->setEchoMode(QLineEdit::Password);

    formLayout->addRow("Email:", m_loginEmail);
    formLayout->addRow("Password:", m_loginPassword);

    auto *btnRow = new QHBoxLayout();
    auto *loginBtn = new QPushButton("Login", page);
    auto *signupLink = new QPushButton("Create an account", page);
    signupLink->setFlat(true);
    signupLink->setObjectName("linkButton");

    btnRow->addWidget(loginBtn);
    btnRow->addStretch();
    btnRow->addWidget(signupLink);

    m_loginStatus = new QLabel(page);
    m_loginStatus->setObjectName("statusLabel");

    layout->addWidget(header);
    layout->addWidget(caption);
    layout->addSpacing(10);
    layout->addLayout(formLayout);
    layout->addSpacing(10);
    layout->addLayout(btnRow);
    layout->addWidget(m_loginStatus);
    layout->addStretch();

    connect(loginBtn, &QPushButton::clicked, this, &LoginWindow::handleLogin);
    connect(signupLink, &QPushButton::clicked, this, &LoginWindow::showSignup);

    return page;
}

QWidget* LoginWindow::buildSignupPage() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setSpacing(16);

    auto *header = new QLabel("Create your bank ID with us", page);
    header->setObjectName("pageTitle");

    auto *caption = new QLabel("Sign up in seconds.", page);
    caption->setWordWrap(true);

    auto *formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight);

    m_signupEmail = new QLineEdit(page);
    m_signupEmail->setPlaceholderText("you@example.com");

    m_signupPassword = new QLineEdit(page);
    m_signupPassword->setPlaceholderText("Choose a strong password");
    m_signupPassword->setEchoMode(QLineEdit::Password);

    m_signupUsername = new QLineEdit(page);
    m_signupUsername->setPlaceholderText("Display name for your profile");

    m_signupDob = new QDateEdit(page);
    m_signupDob->setDisplayFormat("yyyy-MM-dd");
    m_signupDob->setCalendarPopup(true);
    m_signupDob->setDate(QDate(2003, 1, 1));

    formLayout->addRow("Email:", m_signupEmail);
    formLayout->addRow("Password:", m_signupPassword);
    formLayout->addRow("Username:", m_signupUsername);
    formLayout->addRow("Date of birth:", m_signupDob);

    auto *btnRow = new QHBoxLayout();
    auto *signupBtn = new QPushButton("Sign up", page);
    auto *backToLogin = new QPushButton("Back to login", page);
    backToLogin->setFlat(true);
    backToLogin->setObjectName("linkButton");

    btnRow->addWidget(signupBtn);
    btnRow->addStretch();
    btnRow->addWidget(backToLogin);

    m_signupStatus = new QLabel(page);
    m_signupStatus->setObjectName("statusLabel");

    layout->addWidget(header);
    layout->addWidget(caption);
    layout->addSpacing(10);
    layout->addLayout(formLayout);
    layout->addSpacing(10);
    layout->addLayout(btnRow);
    layout->addWidget(m_signupStatus);
    layout->addStretch();

    connect(signupBtn, &QPushButton::clicked, this, &LoginWindow::handleSignup);
    connect(backToLogin, &QPushButton::clicked, this, &LoginWindow::showLogin);

    return page;
}

void LoginWindow::handleLogin() {
    const QString email = m_loginEmail->text();
    const QString password = m_loginPassword->text();

    int userId = DBManager::authenticateUser(email, password);
    if (userId > 0) {
        m_loginStatus->setText("Login successful. Loading your dashboard...");
        emit loginSucceeded(userId);
    } else {
        m_loginStatus->setText("Invalid credentials. Try again or create an account.");
    }
}

void LoginWindow::handleSignup() {
    QString email = m_signupEmail->text().trimmed();
    QString password = m_signupPassword->text();
    QString username = m_signupUsername->text().trimmed();
    QDate dob = m_signupDob->date();

    if (email.isEmpty() || password.isEmpty() || username.isEmpty()) {
        m_signupStatus->setText("Please fill in all fields.");
        return;
    }

    if (!DBManager::createUser(email, password, username, dob)) {
        m_signupStatus->setText("Could not create user. Is this email already registered?");
        return;
    }

    m_signupStatus->setText("Account created. You can login now.");
}

void LoginWindow::showSignup() {
    m_stack->setCurrentIndex(1);
}

void LoginWindow::showLogin() {
    m_stack->setCurrentIndex(0);
}

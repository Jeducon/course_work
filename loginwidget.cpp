#include "loginwidget.h"
#include "database.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
{
    m_stack = new QStackedWidget(this);

    m_loginPage = new QWidget(this);

    auto *loginCard = new QWidget(m_loginPage);
    loginCard->setObjectName("authCard");

    auto *loginTitle = new QLabel(tr("Вхід до бібліотеки"), loginCard);
    loginTitle->setObjectName("authTitle");

    auto *loginSubtitle = new QLabel(
        tr("Увійдіть у систему, щоб переглядати та брати книги"),
        loginCard);
    loginSubtitle->setObjectName("authSubtitle");
    loginSubtitle->setWordWrap(true);

    m_LoginEdit = new QLineEdit(loginCard);
    m_PasswordEdit = new QLineEdit(loginCard);
    m_PasswordEdit->setEchoMode(QLineEdit::Password);

    m_LoginButton = new QPushButton(tr("Увійти"), loginCard);
    m_registerButton = new QPushButton(tr("Реєстрація"), loginCard);

    m_loginStatusLabel = new QLabel(loginCard);
    m_loginStatusLabel->setStyleSheet("color: green;");
    m_loginStatusLabel->clear();

    auto *loginForm = new QFormLayout;
    loginForm->addRow(tr("Логін"), m_LoginEdit);
    loginForm->addRow(tr("Пароль"), m_PasswordEdit);

    auto *loginButtonsLayout = new QVBoxLayout;
    loginButtonsLayout->addWidget(m_LoginButton);
    loginButtonsLayout->addWidget(m_registerButton);

    auto *loginCardLayout = new QVBoxLayout(loginCard);
    loginCardLayout->setContentsMargins(24, 24, 24, 24);
    loginCardLayout->setSpacing(10);
    loginCardLayout->addWidget(loginTitle);
    loginCardLayout->addWidget(loginSubtitle);
    loginCardLayout->addLayout(loginForm);
    loginCardLayout->addSpacing(6);
    loginCardLayout->addLayout(loginButtonsLayout);
    loginCardLayout->addWidget(m_loginStatusLabel);

    auto *loginPageLayout = new QVBoxLayout(m_loginPage);
    loginPageLayout->addStretch();
    loginPageLayout->addWidget(loginCard, 0, Qt::AlignHCenter);
    loginPageLayout->addStretch();

    m_registerPage = new QWidget(this);

    auto *registerCard = new QWidget(m_registerPage);
    registerCard->setObjectName("authCard");

    auto *registerTitle = new QLabel(tr("Реєстрація"), registerCard);
    registerTitle->setObjectName("authTitle");

    auto *registerSubtitle = new QLabel(
        tr("Створіть обліковий запис читача бібліотеки"),
        registerCard);
    registerSubtitle->setObjectName("authSubtitle");
    registerSubtitle->setWordWrap(true);

    m_regLoginEdit = new QLineEdit(registerCard);
    m_regPasswordEdit = new QLineEdit(registerCard);
    m_regRepeatEdit = new QLineEdit(registerCard);
    m_regFullNameEdit = new QLineEdit(registerCard);
    m_regAddressEdit = new QLineEdit(registerCard);
    m_regEmailEdit = new QLineEdit(registerCard);
    m_regPhoneEdit = new QLineEdit(registerCard);

    m_regPasswordEdit->setEchoMode(QLineEdit::Password);
    m_regRepeatEdit->setEchoMode(QLineEdit::Password);

    m_doRegisterButton = new QPushButton(tr("Зареєструватися"), registerCard);
    m_backToLoginButton = new QPushButton(tr("Назад до входу"), registerCard);

    auto *regForm = new QFormLayout;
    regForm->addRow(tr("Логін"), m_regLoginEdit);
    regForm->addRow(tr("Пароль"), m_regPasswordEdit);
    regForm->addRow(tr("Повторіть пароль"), m_regRepeatEdit);
    regForm->addRow(tr("ПІБ"), m_regFullNameEdit);
    regForm->addRow(tr("Адреса"), m_regAddressEdit);
    regForm->addRow(tr("E-mail"), m_regEmailEdit);
    regForm->addRow(tr("Телефон"), m_regPhoneEdit);

    auto *regButtonsLayout = new QVBoxLayout;
    regButtonsLayout->addWidget(m_doRegisterButton);
    regButtonsLayout->addWidget(m_backToLoginButton);

    auto *registerCardLayout = new QVBoxLayout(registerCard);
    registerCardLayout->setContentsMargins(24, 24, 24, 24);
    registerCardLayout->setSpacing(10);
    registerCardLayout->addWidget(registerTitle);
    registerCardLayout->addWidget(registerSubtitle);
    registerCardLayout->addLayout(regForm);
    registerCardLayout->addSpacing(6);
    registerCardLayout->addLayout(regButtonsLayout);

    auto *registerPageLayout = new QVBoxLayout(m_registerPage);
    registerPageLayout->addStretch();
    registerPageLayout->addWidget(registerCard, 0, Qt::AlignHCenter);
    registerPageLayout->addStretch();

    m_stack->addWidget(m_loginPage);
    m_stack->addWidget(m_registerPage);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_stack);
    setLayout(mainLayout);

    connect(m_LoginButton, &QPushButton::clicked, this, &LoginWidget::onLoginClicked);
    connect(m_registerButton, &QPushButton::clicked, this, &LoginWidget::onShowRegisterPage);
    connect(m_backToLoginButton, &QPushButton::clicked, this, &LoginWidget::onShowLoginPage);
    connect(m_doRegisterButton, &QPushButton::clicked, this, &LoginWidget::onRegisterClicked);
}

void LoginWidget::onLoginClicked()
{
    const QString login = m_LoginEdit->text();
    const QString pass = m_PasswordEdit->text();

    QSqlDatabase db = database::db();
    if (!db.isOpen()) {
        QMessageBox::warning(this, tr("Error"), tr("No database connection"));
        return;
    }

    QSqlQuery q(db);
    q.prepare("SELECT role FROM Users "
              "WHERE username = :u AND password = :p");
    q.bindValue(":u", login);
    q.bindValue(":p", pass);

    if (!q.exec()) {
        QMessageBox::warning(this, tr("Error"), tr("Database Request error"));
        return;
    }

    if (q.next()) {
        const QString role = q.value(0).toString();
        emit LoginSuccess(login, role);
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Incorrect Login or Password"));
    }
}

bool isEmailValid(const QString &email)
{
    static const QRegularExpression emailRx(R"(^[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,}$)",
                                            QRegularExpression::CaseInsensitiveOption);
    return emailRx.match(email).hasMatch();
}

void LoginWidget::onRegisterClicked()
{
    const QString login = m_regLoginEdit->text().trimmed();
    const QString pass = m_regPasswordEdit->text();
    const QString repeat = m_regRepeatEdit->text();
    const QString fullName = m_regFullNameEdit->text().trimmed();
    const QString address = m_regAddressEdit->text().trimmed();
    const QString email = m_regEmailEdit->text().trimmed();
    const QString phone = m_regPhoneEdit->text().trimmed();

    if (login.isEmpty() || pass.isEmpty() || repeat.isEmpty() || fullName.isEmpty() || address.isEmpty()
        || email.isEmpty() || phone.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("All fields must be filled"));
        return;
    }
    if (pass != repeat) {
        QMessageBox::warning(this, tr("Error"), tr("Passwords don't match"));
        return;
    }
    if (pass.length() < 6) {
        QMessageBox::warning(this, tr("Error"), tr("Password must be at least 6 characters long"));
        return;
    }

    if (!isEmailValid(email)) {
        QMessageBox::warning(this, tr("Error"), tr("E-mail format is incorrect"));
        return;
    }

    emit RegisterRequested(login, pass, fullName, address, phone, email);
    emit RegistrationSucceeded();
}

void LoginWidget::onShowRegisterPage()
{
    m_stack->setCurrentWidget(m_registerPage);
}

void LoginWidget::onShowLoginPage()
{
    m_stack->setCurrentWidget(m_loginPage);
}

void LoginWidget::setLoginStatusMessage(const QString &msg)
{
    m_loginStatusLabel->setText(msg);
}

void LoginWidget::showLoginPage()
{
    m_stack->setCurrentIndex(0);
}

void LoginWidget::clearFields()
{
    m_LoginEdit->clear();
    m_PasswordEdit->clear();
    m_loginStatusLabel->clear();

    m_regLoginEdit->clear();
    m_regPasswordEdit->clear();
    m_regRepeatEdit->clear();
    m_regFullNameEdit->clear();
    m_regEmailEdit->clear();
    m_regPhoneEdit->clear();
}

#include "loginwidget.h"
#include "database.h"


#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QStackedWidget>
#include <QLabel>
#include <QRegularExpression>


LoginWidget::LoginWidget(QWidget *parent) : QWidget(parent){

    m_stack = new QStackedWidget(this);

    m_loginPage = new QWidget(this);
    m_LoginEdit = new QLineEdit(m_loginPage);
    m_PasswordEdit = new QLineEdit(m_loginPage);
    m_PasswordEdit->setEchoMode(QLineEdit::Password);

    m_LoginButton = new QPushButton(tr("Увійти"), m_loginPage);
    m_registerButton = new QPushButton(tr("Реєстрація"), m_loginPage);

    m_loginStatusLabel = new QLabel(m_loginPage);
    m_loginStatusLabel -> setStyleSheet("color: green;");
    m_loginStatusLabel -> clear();

    auto *loginform = new QFormLayout;
    loginform -> addRow(tr("Login"), m_LoginEdit);
    loginform -> addRow(tr("Password"), m_PasswordEdit);

    auto *loginLayout = new QVBoxLayout(m_loginPage);
    loginLayout -> addLayout(loginform);
    loginLayout -> addWidget(m_LoginButton);
    loginLayout->addWidget(m_registerButton);
    loginLayout -> addWidget(m_loginStatusLabel);
    loginLayout -> addStretch();
    m_loginPage -> setLayout(loginLayout);

    m_registerPage = new QWidget(this);
    m_regLoginEdit = new QLineEdit(m_registerPage);
    m_regPasswordEdit = new QLineEdit(m_registerPage);
    m_regRepeatEdit   = new QLineEdit(m_registerPage);
    m_regFullNameEdit = new QLineEdit(m_registerPage);
    m_regEmailEdit    = new QLineEdit(m_registerPage);
    m_regPhoneEdit    = new QLineEdit(m_registerPage);

    m_regPasswordEdit -> setEchoMode(QLineEdit::Password);
    m_regRepeatEdit -> setEchoMode(QLineEdit::Password);

    m_doRegisterButton = new QPushButton(tr("Зареєструватися"), m_registerPage);
    m_backToLoginButton = new QPushButton(tr("Назад до входу"), m_registerPage);

    auto *regForm = new QFormLayout;
    regForm -> addRow(tr("Login"), m_regLoginEdit);
    regForm -> addRow(tr("Password"), m_regPasswordEdit);
    regForm -> addRow(tr("Repeat Password"), m_regRepeatEdit);
    regForm ->addRow(tr("ПІБ"), m_regFullNameEdit);
    regForm ->addRow(tr("E-mail"), m_regEmailEdit);
    regForm ->addRow(tr("Телефон"), m_regPhoneEdit);

    auto *regLayout = new QVBoxLayout(m_registerPage);
    regLayout -> addLayout(regForm);
    regLayout -> addWidget(m_doRegisterButton);
    regLayout -> addWidget(m_backToLoginButton);
    regLayout -> addStretch();
    m_registerPage -> setLayout(regLayout);

    m_stack -> addWidget(m_loginPage);
    m_stack -> addWidget(m_registerPage);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout -> addWidget(m_stack);
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
    if(!db.isOpen()){
        QMessageBox::warning(this, tr("Error"), tr("No database connection"));
        return;
    }

    QSqlQuery q(db);
    q.prepare("SELECT role FROM Users "
              "WHERE username = :u AND password = :p");
    q.bindValue(":u", login);
    q.bindValue(":p", pass);

    if(!q.exec()){
        QMessageBox::warning(this, tr("Error"), tr("Database Request error"));
        return;
    }

    if(q.next()){
        const QString role = q.value(0).toString();
        emit LoginSuccess(login, role);
    }
    else{
        QMessageBox::warning(this, tr("Error"), tr("Incorrect Login or Password"));
    }
}

bool isEmailValid(const QString &email)
{
    static const QRegularExpression emailRx(
        R"(^[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,}$)",
        QRegularExpression::CaseInsensitiveOption);
    return emailRx.match(email).hasMatch();
}

void LoginWidget::onRegisterClicked(){
    const QString login = m_regLoginEdit ->text().trimmed();
    const QString pass = m_regPasswordEdit->text();
    const QString repeat = m_regRepeatEdit->text();
    const QString fullName = m_regFullNameEdit->text().trimmed();
    const QString email    = m_regEmailEdit->text().trimmed();
    const QString phone    = m_regPhoneEdit->text().trimmed();

    if(login.isEmpty() || pass.isEmpty() || repeat.isEmpty()
        || fullName.isEmpty() || email.isEmpty() || phone.isEmpty()){
        QMessageBox::warning(this, tr("Error"), tr("All fields must be filled"));
        return;
    }
    if(pass != repeat){
        QMessageBox::warning(this, tr("Error"), tr ("Passwords don't match"));
        return;
    }
    if(pass.length() < 6){
        QMessageBox::warning(this, tr("Error"), tr("Password must be at least 6 characters long"));
        return;
    }

    if (!isEmailValid(email)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("E-mail format is incorrect"));
        return;
    }

    emit RegisterRequested(login, pass, fullName, phone, email);
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

void LoginWidget::setLoginStatusMessage(const QString &msg){
    m_loginStatusLabel -> setText(msg);
}

void LoginWidget::showLoginPage()
{
    m_stack->setCurrentIndex(0);
}

void LoginWidget::clearFields()
{
    m_LoginEdit -> clear();
    m_PasswordEdit -> clear();
    m_loginStatusLabel -> clear();

    m_regLoginEdit -> clear();
    m_regPasswordEdit->clear();
    m_regRepeatEdit->clear();
    m_regFullNameEdit->clear();
    m_regEmailEdit->clear();
    m_regPhoneEdit->clear();
}


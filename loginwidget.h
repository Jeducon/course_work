#pragma once

#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QStackedWidget;
class QLabel;

class LoginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LoginWidget(QWidget *parent = nullptr);
    void showLoginPage();
    void setLoginStatusMessage(const QString &msg);
    void clearFields();

signals:
    void LoginSuccess(const QString &username, const QString &role);
    void LoginFailed();
    void RegisterRequested(const QString &login,
                           const QString &password,
                           const QString &fullName,
                           const QString &address,
                           const QString &phone,
                           const QString &email);
    void RegistrationSucceeded();

private slots:
    void onLoginClicked();
    void onShowRegisterPage();
    void onShowLoginPage();
    void onRegisterClicked();

private:
    QStackedWidget *m_stack;

    QWidget *m_loginPage;
    QLineEdit *m_LoginEdit;
    QLineEdit *m_PasswordEdit;
    QPushButton *m_LoginButton;
    QPushButton *m_registerButton;

    QWidget *m_registerPage;
    QLineEdit *m_regLoginEdit;
    QLineEdit *m_regPasswordEdit;
    QLineEdit *m_regRepeatEdit;
    QLineEdit *m_regFullNameEdit;
    QLineEdit *m_regAddressEdit;
    QLineEdit *m_regEmailEdit;
    QLineEdit *m_regPhoneEdit;

    QPushButton *m_doRegisterButton;
    QPushButton *m_backToLoginButton;

    QLabel *m_loginStatusLabel;
};

#endif

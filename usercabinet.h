#pragma once

#ifndef USERCABINET_H
#define USERCABINET_H

#include <QWidget>
#include <QAbstractItemModel>

class QLabel;
class QTableView;
class QPushButton;

class usercabinet : public QWidget
{
    Q_OBJECT
public:
    explicit usercabinet(QWidget *parent = nullptr);

    void setUserName(const QString &name);
    void setUserInfo(const QString &fullName,
                     const QString &address,
                     const QString &phone,
                     const QString &email);
    void setUserPhoto(const QString &photoPath);
    void setLoansModel(QAbstractItemModel *model);

    void setReaderStats(int total,
                        int active,
                        int returned,
                        int overdue,
                        const QString &topGenres,
                        const QString &topAuthors);

signals:
    void backToLibrary();
    void logoutRequested();

private slots:
    void onChangePhotoClicked();

private:
    QString m_username;

    QLabel *m_welcomeLabel;
    QLabel *m_photoLabel;
    QLabel *m_nameLabel;
    QLabel *m_addressLabel;
    QLabel *m_phoneLabel;
    QLabel *m_emailLabel;

    QLabel *m_totalBooksLabel;
    QLabel *m_activeBooksLabel;
    QLabel *m_returnedBooksLabel;
    QLabel *m_overdueBooksLabel;
    QLabel *m_topGenresLabel;
    QLabel *m_topAuthorsLabel;

    QTableView *m_loansView;
    QPushButton *m_backButton;
    QPushButton *m_changePhotoButton;
    QPushButton *m_logoutButton;
};

#endif

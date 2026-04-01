#pragma once

#include <QWidget>
#include <QAbstractItemModel>

class QLabel;
class QTableView;
class QPushButton;

class AdminCabinet : public QWidget
{
    Q_OBJECT
public:
    explicit AdminCabinet(QWidget *parent = nullptr);

    void setAdminInfo(const QString &fullName,
                      const QString &email);
    void setLoansModel(QAbstractItemModel *model);

    void setStats(int totalBooks,
                  int availableBooks,
                  int loanedBooks,
                  int activeLoans,
                  int overdueLoans,
                  int totalUsers);

signals:
    void backToLibrary();
    void returnLoanRequested(const QModelIndex &index);

private slots:
    void onReturnClicked();

private:
    QLabel     *m_welcomeLabel;
    QLabel     *m_nameLabel;
    QLabel     *m_emailLabel;

    QLabel *m_totalBooksLabel;
    QLabel *m_availableBooksLabel;
    QLabel *m_loanedBooksLabel;
    QLabel *m_activeLoansLabel;
    QLabel *m_overdueLoansLabel;
    QLabel *m_totalUsersLabel;

    QTableView *m_loansView;
    QPushButton *m_backButton;
    QPushButton *m_returnButton;
};

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

signals:
    void backToLibrary();
    void returnLoanRequested(const QModelIndex &index);

private slots:
    void onReturnClicked();

private:
    QLabel     *m_welcomeLabel;
    QLabel     *m_nameLabel;
    QLabel     *m_emailLabel;

    QTableView *m_loansView;
    QPushButton *m_backButton;
    QPushButton *m_returnButton;
};

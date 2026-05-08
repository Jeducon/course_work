#pragma once

#include <QWidget>
#include <QAbstractItemModel>

class QLabel;
class QTableView;
class QPushButton;
class QChart;
class QChartView;
class QComboBox;
class QTabWidget;
class QFrame;

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

    void setBadUsersModel(QAbstractItemModel *model);
    void setGoodUsersModel(QAbstractItemModel *model);
    void setPeakStats(const QString &peakDay,
                      const QString &peakMonth,
                      const QString &topGenre);

    void setLoansChart(QChart *chart);
    int currentChartIndex() const;
    void resetToHomePage();
    void setTopBookCard(const QString &title,
                        const QString &meta,
                        const QPixmap &cover,
                        int loansCount);

signals:
    void backToLibrary();
    void returnLoanRequested(const QModelIndex &index);
    void chartTypeChanged(int index);
    void exportReportRequested();

private slots:
    void onReturnClicked();
    void onChartTypeChanged(int index);

private:
    QLabel *m_welcomeLabel;
    QLabel *m_nameLabel;
    QLabel *m_emailLabel;

    QLabel *m_totalBooksLabel;
    QLabel *m_availableBooksLabel;
    QLabel *m_loanedBooksLabel;
    QLabel *m_activeLoansLabel;
    QLabel *m_overdueLoansLabel;
    QLabel *m_totalUsersLabel;

    QLabel *m_peakDayLabel;
    QLabel *m_peakMonthLabel;
    QLabel *m_topBookLabel;
    QLabel *m_topGenreLabel;

    QTableView *m_loansView;
    QTableView *m_badUsersView;
    QTableView *m_goodUsersView;

    QPushButton *m_backButton;
    QPushButton *m_returnButton;

    QChartView *m_loansChartView;
    QComboBox *m_chartTypeCombo;

    QTabWidget *m_tabs;
    QWidget *m_overviewTab;
    QWidget *m_analyticsTab;
    QWidget *m_loansTab;

    QPushButton *m_exportReportButton;

    QWidget    *m_topBookContainer = nullptr;
    QTableView *m_topBookView      = nullptr;

    QFrame *m_topBookCard;
    QLabel *m_topBookCoverLabel;
    QLabel *m_topBookTitleValueLabel;
    QLabel *m_topBookMetaValueLabel;
};

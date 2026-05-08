#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QSqlTableModel>
#include <QListView>
#include <QTextEdit>


class QFrame;
class QStackedWidget;
class LoginWidget;
class QWidget;
class usercabinet;
class QTableView;
class QPushButton;
class QLineEdit;
class QComboBox;
class booksmodel;
class BookCardDelegate;
class QSqlTableModel;
class LoansModel;
class QSortFilterProxyModel;
class QToolButton;
class AdminCabinet;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    int m_currentUserId = -1;
    QWidget *setupLibraryPage();
    QWidget *setupLibraryTopBar();

    QStackedWidget *m_stack;
    LoginWidget *m_loginWidget;
    QWidget *m_libraryWidget;
    usercabinet *m_cabinetWidget;
    booksmodel *m_booksModel;

    QSortFilterProxyModel *m_booksProxyModel = nullptr;

    QListView* m_booksListView;
    QPushButton* m_addBookButton;
    QPushButton *m_editBookButton;
    QPushButton *m_deleteBooksButton;

    bool m_isEditMode = false;
    int m_editBookId = -1;

    QString m_currentUserName;
    QString m_currentUserRole;

    QString m_currentCoverPath;
    QLabel *m_coverPreviewLabel;
    QPushButton *m_chooseCoverButton;

    QWidget *m_addBookPage;
    QLineEdit *m_titleEdit;
    QComboBox *m_authorCombo;
    QPushButton *m_addAuthorButton;
    QLineEdit *m_genreEdit;
    QLineEdit *m_yearEdit;
    QComboBox *m_statusCombo;
    QPushButton *m_saveBookButton;
    QPushButton *m_cancelAddBookButton;

    QLabel *m_bookCoverLabel;

    BookCardDelegate* m_bookCardDelegate;

    QWidget *m_bookDetailsPage;
    QLabel *m_detailsCoverLabel;
    QLabel *m_detailsTitleLabel;
    QLabel *m_detailsAuthorLabel;
    QLabel *m_detailsGenreLabel;
    QLabel *m_detailsYearLabel;
    QLabel *m_detailsStatusLabel;
    QTextEdit *m_detailsDescriptionEdit;
    QLabel *m_detailsReadersCountLabel;

    QPushButton *m_detailsBackButton;

    LoansModel* m_loansModel = nullptr;
    QSqlQueryModel *m_badUsersModel = nullptr;
    QSqlQueryModel *m_goodUsersModel = nullptr;
    QPushButton* m_authButton = nullptr;

    QToolButton *m_filtersButton = nullptr;
    QLineEdit *m_searchEdit = nullptr;

    QFrame *m_filtersPanel = nullptr;
    QComboBox *m_authorFilterCombo = nullptr;
    QComboBox *m_genreFilterCombo = nullptr;
    QLineEdit *m_yearFilterEdit = nullptr;
    QPushButton *m_resetFiltersButton = nullptr;

    AdminCabinet *m_adminCabinet = nullptr;

    QTextEdit *m_descriptionEdit;

private slots:
    void onLoginSuccess(const QString &username, const QString &role);
    void showUserCabinet();
    void showLibrary();
    void onDeleteBooksClicked();
    void onRegisterRequested(const QString &login,
                             const QString &pass,
                             const QString &fullName,
                             const QString &address,
                             const QString &phone,
                             const QString &email);
    void onRegistrationSucceeded();
    void onLogoutClicked();
    void onAddBookPageRequested();
    void onEditBookClicked();
    void openBookFormForEdit(const QModelIndex &proxyIndex);
    void onSaveBookClicked();
    void onChooseCoverClicked();
    void onCurrentBookChanged(const QModelIndex &current, const QModelIndex &previous);
    void onBookInfoRequested(const QModelIndex &index);
    void onBookDoubleClicked(const QModelIndex &index);
    void showBookDetails(const QModelIndex &index);
    void onDetailsBackClicked();
    void onTakeRequested(const QModelIndex &index);
    void showLoginDialog();
    void onAuthButtonClicked();
    void refreshLoans();
    void refreshReaderStats();
    void showFiltersPopup();
    void showAdminCabinet();
    void onAdminReturnLoanRequested(const QModelIndex &index);
    void refreshAdminStats();
    void refreshAllOverdues();
    void refreshPeakStats();
    void refreshLoansChart();
    void refreshGenreChart();
    void onChartTypeChanged(int index);
    void refreshCurrentUserChart();
    void refreshUserGenreChart();
    void refreshUserAuthorChart();
    void onUserChartTypeChanged(int index);
    void onExportAdminReportRequested();
    void refreshStatusChart();
    void refreshTopBooksChart();

    void reloadAuthors();
    int selectedAuthorId() const;
    void selectAuthorById(int authorId);
    void onAddAuthorClicked();

    void resetLibraryFilters();
signals:
};

#endif

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QLabel>
#include <QListView>

class QStackedWidget;
class LoginWidget;
class QWidget;
class usercabinet;
class QTableView;
class QPushButton;
class QLineEdit;
class QComboBox;
class booksmodel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private:
    QWidget* setupLibraryPage();
    QWidget* setupLibraryTopBar();

    QStackedWidget* m_stack;
    LoginWidget* m_loginWidget;
    QWidget* m_libraryWidget;
    usercabinet* m_cabinetWidget;
    booksmodel* m_booksModel;

    QListView* m_booksListView;
    QPushButton* m_addBookButton;

    QPushButton* m_deleteBooksButton;

    QString m_currentUserName;
    QString m_currentUserRole;

    QString m_currentCoverPath;
    QLabel* m_coverPreviewLabel;
    QPushButton* m_chooseCoverButton;

    QWidget* m_addBookPage;
    QLineEdit* m_titleEdit;
    QLineEdit* m_authorEdit;
    QLineEdit* m_genreEdit;
    QLineEdit* m_yearEdit;
    QComboBox* m_statusCombo;
    QPushButton* m_saveBookButton;
    QPushButton* m_cancelAddBookButton;

    QLabel* m_bookCoverLabel;

private slots:
    void onLoginSuccess(const QString &username, const QString &role);
    void showUserCabinet();
    void showLibrary();
    void onAddBookClicked();
    void onDeleteBooksClicked();
    void onRegisterRequested(const QString &login,
                             const QString &pass,
                             const QString &fullName,
                             const QString &phone,
                             const QString &email);
    void onRegistrationSucceeded();
    void onLogoutClicked();
    void onAddBookPageRequested();
    void onSaveBookClicked();
    void onChooseCoverClicked();
    void onCurrentBookChanged(const QModelIndex &current, const QModelIndex &previous);
signals:
};

#endif

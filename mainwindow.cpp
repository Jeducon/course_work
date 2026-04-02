#include "mainwindow.h"
#include "bookcarddelegate.h"
#include "loginwidget.h"
#include "usercabinet.h"
#include "database.h"
#include "addbookdialog.h"
#include "booksmodel.h"
#include "loansmodel.h"
#include "booksfilterproxymodel.h"
#include "admincabinet.h"

#include <QStackedWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSqlTableModel>
#include <QMessageBox>
#include <QSqlQuery>
#include <QItemSelectionModel>
#include <QComboBox>
#include <QLineEdit>
#include <QFormLayout>
#include <QFileDialog>
#include <QSqlError>
#include <QPixmap>
#include <QListView>
#include <QDate>
#include <QSortFilterProxyModel>
#include <QSet>
#include <QLabel>
#include <QToolButton>
#include <QIcon>
#include <algorithm>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_stack = new QStackedWidget(this);

    m_booksModel = new booksmodel(this, database::db());
    m_booksModel->setTable("Books");
    m_booksModel->select();

    m_booksProxyModel = new BooksFilterProxyModel(this);
    m_booksProxyModel->setSourceModel(m_booksModel);
    m_booksProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_booksModel->setHeaderData(1, Qt::Horizontal, tr("Назва"));
    m_booksModel->setHeaderData(2, Qt::Horizontal, tr("Автор"));
    m_booksModel->setHeaderData(3, Qt::Horizontal, tr("Жанр"));
    m_booksModel->setHeaderData(4, Qt::Horizontal, tr("Рік"));
    m_booksModel->setHeaderData(5, Qt::Horizontal, tr("Статус"));

    setCentralWidget(m_stack);

    m_loansModel = new LoansModel(this);

    m_loginWidget = new LoginWidget(this);
    m_libraryWidget = setupLibraryPage();

    auto *proxy = qobject_cast<BooksFilterProxyModel*>(m_booksProxyModel);
    if (proxy && m_searchEdit) {
        connect(m_searchEdit, &QLineEdit::textChanged,
                this, [proxy](const QString &text){
                    proxy->setSearchText(text);
                });
    }

    m_bookCardDelegate = new BookCardDelegate(m_booksListView);
    m_bookCardDelegate->setUserRole(m_currentUserRole);
    m_booksListView->setItemDelegate(m_bookCardDelegate);

    if (m_booksListView && m_booksListView->selectionModel()) {
        connect(m_booksListView->selectionModel(),
                &QItemSelectionModel::currentRowChanged,
                this,
                &MainWindow::onCurrentBookChanged);
    }

    m_addBookPage = new QWidget(this);

    m_titleEdit = new QLineEdit(m_addBookPage);
    m_authorEdit = new QLineEdit(m_addBookPage);
    m_genreEdit = new QLineEdit(m_addBookPage);
    m_yearEdit = new QLineEdit(m_addBookPage);
    m_statusCombo = new QComboBox(m_addBookPage);
    m_statusCombo->addItems({tr("available"), tr("loaned")});

    m_coverPreviewLabel = new QLabel(m_addBookPage);
    m_coverPreviewLabel->setFixedSize(120, 160);
    m_coverPreviewLabel->setFrameShape(QFrame::Box);
    m_coverPreviewLabel->setAlignment(Qt::AlignCenter);
    m_coverPreviewLabel->setText(tr("No cover"));
    m_coverPreviewLabel->setScaledContents(true);

    m_chooseCoverButton = new QPushButton(tr("Обрати обкладинку"), m_addBookPage);
    m_saveBookButton = new QPushButton(tr("Зберегти книгу"), m_addBookPage);
    m_cancelAddBookButton = new QPushButton(tr("Назад до каталогу"), m_addBookPage);

    m_bookDetailsPage = new QWidget(this);

    m_detailsCoverLabel = new QLabel(m_bookDetailsPage);
    m_detailsCoverLabel -> setFixedSize(200, 280);
    m_detailsCoverLabel -> setFrameShape(QFrame::Box);
    m_detailsCoverLabel -> setAlignment(Qt::AlignCenter);
    m_detailsCoverLabel -> setText(tr("No cover"));
    m_detailsCoverLabel -> setScaledContents(true);

    m_detailsTitleLabel  = new QLabel(m_bookDetailsPage);
    m_detailsAuthorLabel = new QLabel(m_bookDetailsPage);
    m_detailsGenreLabel  = new QLabel(m_bookDetailsPage);
    m_detailsYearLabel   = new QLabel(m_bookDetailsPage);
    m_detailsStatusLabel = new QLabel(m_bookDetailsPage);

    m_detailsTitleLabel->setStyleSheet("font-weight: bold; font-size: 18px;");

    m_detailsBackButton = new QPushButton(tr("Назад до каталогу"), m_bookDetailsPage);

    auto *detailsForm = new QFormLayout;
    detailsForm->addRow(tr("Назва:"),  m_detailsTitleLabel);
    detailsForm->addRow(tr("Автор:"),  m_detailsAuthorLabel);
    detailsForm->addRow(tr("Жанр:"),   m_detailsGenreLabel);
    detailsForm->addRow(tr("Рік:"),    m_detailsYearLabel);
    detailsForm->addRow(tr("Статус:"), m_detailsStatusLabel);

    auto *detailsTopLayout = new QHBoxLayout;
    detailsTopLayout->addWidget(m_detailsCoverLabel);
    detailsTopLayout->addLayout(detailsForm);

    auto *detailsPageLayout = new QVBoxLayout(m_bookDetailsPage);
    detailsPageLayout->addLayout(detailsTopLayout);
    detailsPageLayout->addStretch();
    detailsPageLayout->addWidget(m_detailsBackButton, 0, Qt::AlignRight);
    m_bookDetailsPage->setLayout(detailsPageLayout);

    auto *formLayout = new QFormLayout;
    formLayout->addRow(tr("Назва:"), m_titleEdit);
    formLayout->addRow(tr("Автор:"), m_authorEdit);
    formLayout->addRow(tr("Жанр:"), m_genreEdit);
    formLayout->addRow(tr("Рік:"), m_yearEdit);
    formLayout->addRow(tr("Статус:"), m_statusCombo);

    auto *coverLayout = new QHBoxLayout;
    coverLayout->addWidget(m_coverPreviewLabel);
    coverLayout->addWidget(m_chooseCoverButton);
    coverLayout->addStretch();

    auto *topLayout = new QHBoxLayout;
    topLayout->addLayout(formLayout);
    topLayout->addLayout(coverLayout);

    auto *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(m_saveBookButton);
    buttonsLayout->addWidget(m_cancelAddBookButton);
    buttonsLayout->addStretch();

    auto *pageLayout = new QVBoxLayout(m_addBookPage);
    pageLayout->addLayout(topLayout);
    pageLayout->addLayout(buttonsLayout);
    m_addBookPage->setLayout(pageLayout);

    m_stack->addWidget(m_addBookPage);

    m_cabinetWidget = new usercabinet(this);
    m_adminCabinet  = new AdminCabinet(this);

    m_stack->addWidget(m_loginWidget);
    m_stack->addWidget(m_libraryWidget);
    m_stack->addWidget(m_cabinetWidget);
    m_stack->addWidget(m_adminCabinet);

    m_stack->setCurrentWidget(m_libraryWidget);

    m_stack->addWidget(m_bookDetailsPage);

    connect(m_loginWidget, &LoginWidget::LoginSuccess,
            this, &MainWindow::onLoginSuccess);

    connect(m_cabinetWidget, &usercabinet::backToLibrary, this, &MainWindow::showLibrary);

    connect(m_loginWidget, &LoginWidget::RegisterRequested, this, &MainWindow::onRegisterRequested);

    connect(m_deleteBooksButton, &QPushButton::clicked, this, &MainWindow::onDeleteBooksClicked);

    connect(m_loginWidget,
            &LoginWidget::RegistrationSucceeded,
            this,
            &MainWindow::onRegistrationSucceeded);

    connect(m_cabinetWidget, &usercabinet::logoutRequested, this, &MainWindow::onLogoutClicked);

    connect(m_saveBookButton, &QPushButton::clicked, this, &MainWindow::onSaveBookClicked);

    connect(m_cancelAddBookButton, &QPushButton::clicked, this, &MainWindow::showLibrary);

    connect(m_chooseCoverButton, &QPushButton::clicked, this, &MainWindow::onChooseCoverClicked);

    connect(m_bookCardDelegate, &BookCardDelegate::bookInfoRequested,
            this, &MainWindow::onBookInfoRequested);

    connect(m_detailsBackButton, &QPushButton::clicked,
            this, &MainWindow::onDetailsBackClicked);

    bool ok = connect(m_bookCardDelegate, &BookCardDelegate::takeRequested,
            this, &MainWindow::onTakeRequested);

    qDebug() << "connect takeRequested:" << ok;

    connect(m_filtersButton, &QToolButton::clicked,
            this, &MainWindow::showFiltersPopup);

    connect(m_adminCabinet, &AdminCabinet::backToLibrary,
            this, &MainWindow::showLibrary);

    connect(m_adminCabinet, &AdminCabinet::returnLoanRequested,
            this, &MainWindow::onAdminReturnLoanRequested);

}

void MainWindow::onLoginSuccess(const QString &username, const QString &role)
{
    m_currentUserName = username;
    m_currentUserRole = role;

    m_cabinetWidget->setUserName(username);

    if (role == "admin") {
        m_addBookButton->setVisible(true);
        m_addBookButton->setEnabled(true);
        m_deleteBooksButton->setVisible(true);
        m_deleteBooksButton->setEnabled(true);
    } else {
        m_addBookButton->setVisible(false);
        m_deleteBooksButton->setVisible(false);
    }

    QSqlQuery q(database::db());
    q.prepare("SELECT id, full_name, address, phone, email, photo_path "
              "FROM Users WHERE username = :u");
    q.bindValue(":u", username);

    if (!q.exec()) {
        qDebug() << "Login SELECT error:" << q.lastError().text();
        return;
    }

    if (!q.next()) {
        qDebug() << "Login SELECT: no row for username" << username;
        return;
    }

    m_currentUserId = q.value(0).toInt();
    const QString fullName = q.value(1).toString();
    const QString address = q.value(2).toString();
    const QString phone = q.value(3).toString();
    const QString email = q.value(4).toString();
    const QString photoPath = q.value(5).toString();

    m_cabinetWidget->setUserName(username);
    m_cabinetWidget->setUserInfo(fullName, address, phone, email);
    m_cabinetWidget->setUserPhoto(photoPath);


    if (role == "admin" && m_adminCabinet) {
        m_adminCabinet->setAdminInfo(fullName, email);
    }

    refreshLoans();

    if (m_bookCardDelegate) {
        m_bookCardDelegate->setUserRole(role);
        m_booksListView->viewport()->update();
    }



    m_loansModel->setHeaderData(1, Qt::Horizontal, tr("Назва"));
    m_loansModel->setHeaderData(3, Qt::Horizontal, tr("Дата видачі"));
    m_loansModel->setHeaderData(4, Qt::Horizontal, tr("Повернути до"));
    m_loansModel->setHeaderData(5, Qt::Horizontal, tr("Дата повернення"));
    m_loansModel->setHeaderData(6, Qt::Horizontal, tr("Статус"));

    if(m_authButton){
        m_authButton -> setText("Log out");
    }

    if (role == "admin") {
        showAdminCabinet();
    } else {
        m_cabinetWidget->setLoansModel(m_loansModel);
        m_stack->setCurrentWidget(m_cabinetWidget);
    }
}

QWidget *MainWindow::setupLibraryPage()
{
    auto *page = new QWidget(this);

    QPushButton* cabinetButton = new QPushButton(tr("Personal Cabinet"), page);

    m_authButton = new QPushButton(tr("Log in"), page);

    m_filtersButton = new QToolButton(page);
    m_filtersButton->setText(tr("Фільтри"));
    m_filtersButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    m_searchEdit = new QLineEdit(page);
    m_searchEdit->setPlaceholderText(tr("Пошук..."));
    m_searchEdit->setClearButtonEnabled(true);

    m_booksListView = new QListView(page);
    m_booksListView->setModel(m_booksProxyModel);
    m_booksListView->setViewMode(QListView::IconMode);
    m_booksListView->setResizeMode(QListView::Adjust);
    m_booksListView->setWrapping(true);
    m_booksListView->setSpacing(16);
    m_booksListView->setIconSize(QSize(160, 240));
    m_booksListView->setGridSize(QSize(220, 340));
    m_booksListView->setUniformItemSizes(true);

    m_booksListView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_booksListView->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_booksListView->setCurrentIndex(QModelIndex());

    m_addBookButton = new QPushButton(tr("Додати книгу"), page);
    m_deleteBooksButton = new QPushButton(tr("Видалити вибрані книги"), page);

    m_addBookButton->setVisible(false);
    m_deleteBooksButton->setVisible(false);

    auto *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(m_addBookButton);
    buttonsLayout->addWidget(m_deleteBooksButton);
    buttonsLayout->addStretch();

    auto *topLayout = new QHBoxLayout;
    topLayout->addWidget(cabinetButton);
    topLayout->addWidget(m_searchEdit);
    topLayout->addStretch();
    topLayout->addWidget(m_filtersButton);
    topLayout->addWidget(m_authButton);

    auto *centerLayout = new QHBoxLayout;
    centerLayout -> addWidget(m_booksListView);
    /*centerLayout -> addWidget(m_bookCoverLabel); */

    auto *layout = new QVBoxLayout(page);
    layout->addLayout(topLayout);
    layout->addLayout(centerLayout);
    layout->addLayout(buttonsLayout);
    page->setLayout(layout);

    connect(m_addBookButton, &QPushButton::clicked, this, &MainWindow::onAddBookPageRequested);

    connect(cabinetButton, &QPushButton::clicked, this, &MainWindow::showUserCabinet);

    connect(m_authButton, &QPushButton::clicked, this, &MainWindow::showLoginDialog);

    /*connect(m_booksListView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onCurrentBookChanged); */

    return page;
}

void MainWindow::showLoginDialog()
{
    m_stack -> setCurrentWidget(m_loginWidget);
}
void MainWindow::showUserCabinet()
{
    if(m_currentUserId <= 0 || m_currentUserName.isEmpty()){
        m_stack -> setCurrentWidget(m_loginWidget);
        return;
    }
    if (m_currentUserRole == "admin") {
        showAdminCabinet();
    } else {
        m_cabinetWidget->setUserName(m_currentUserName);
        m_stack->setCurrentWidget(m_cabinetWidget);
    }
}
void MainWindow::onAuthButtonClicked(){
    if(m_currentUserName.isEmpty()){
        m_stack -> setCurrentWidget(m_loginWidget);
    }
    else{
        onLogoutClicked();
    }
}

void MainWindow::showLibrary()
{
    m_stack->setCurrentWidget(m_libraryWidget);
}


void MainWindow::onRegisterRequested(const QString &login,
                                     const QString &pass,
                                     const QString &fullName,
                                     const QString &address,
                                     const QString &phone,
                                     const QString &email)
{
    if (login.isEmpty() || pass.isEmpty() || fullName.isEmpty() || phone.isEmpty()
        || email.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("All fields must be filled"));
        return;
    }

    const QString role = "user";

    QSqlQuery q(database::db());
    q.prepare(
        "INSERT INTO Users(username, password, role, full_name, address, phone, email, photo_path) "
        "VALUES (:u, :p, :r, :fn, :addr, :ph, :em, '')");
    q.bindValue(":u", login);
    q.bindValue(":p", pass);
    q.bindValue(":r", role);
    q.bindValue(":fn", fullName);
    q.bindValue(":addr", address);
    q.bindValue(":ph", phone);
    q.bindValue(":em", email);

    if (!q.exec()) {
        QMessageBox::warning(this, tr("Error"), tr("Can't register, maybe someone took your login"));
        return;
    }
}

void MainWindow::onDeleteBooksClicked()
{
    if (m_currentUserRole != "admin") {
        QMessageBox::warning(this, tr("Access Denied"), tr("Only admin can delete books"));
        return;
    }

    QModelIndex proxyIndex = m_booksListView->currentIndex();
    if (!proxyIndex.isValid()) {
        QMessageBox::information(this, tr("No selection"), tr("Select a book to delete"));
        return;
    }

    if (QMessageBox::question(this,
                              tr("Confirm delete"),
                              tr("Delete selected book?"))
        != QMessageBox::Yes) {
        return;
    }

    auto *proxy = qobject_cast<BooksFilterProxyModel*>(m_booksProxyModel);
    if (!proxy) {
        QMessageBox::warning(this, tr("Error"), tr("Proxy model is not available"));
        return;
    }

    QModelIndex sourceIndex = proxy->mapToSource(proxyIndex);
    if (!sourceIndex.isValid()) {
        QMessageBox::warning(this, tr("Error"), tr("Selected source index is invalid"));
        return;
    }

    if (!m_booksModel->removeRow(sourceIndex.row())) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to remove selected row"));
        return;
    }

    if (!m_booksModel->submitAll()) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to delete selected book"));
        m_booksModel->revertAll();
    } else {
        m_booksModel->select();
        m_booksListView->clearSelection();
        m_booksListView->setCurrentIndex(QModelIndex());
    }
}

void MainWindow::onRegistrationSucceeded()
{
    m_loginWidget->showLoginPage();
    m_loginWidget->setLoginStatusMessage(tr("Реєстрація успішна. Увійдіть у свій акаунт."));
}

void MainWindow::onLogoutClicked()
{
    m_currentUserName.clear();
    m_currentUserRole.clear();
    m_currentUserId = -1;

    if(m_authButton){
        m_authButton -> setText("Log in");
    }

    m_loginWidget->clearFields();
    m_loginWidget->showLoginPage();
    m_stack->setCurrentWidget(m_libraryWidget);

    m_addBookButton->setVisible(false);
    m_addBookButton->setEnabled(false);

    m_deleteBooksButton->setVisible(false);
    m_deleteBooksButton->setEnabled(false);
}

void MainWindow::onAddBookPageRequested()
{
    m_titleEdit->clear();
    m_authorEdit->clear();
    m_genreEdit->clear();
    m_yearEdit->clear();
    m_statusCombo->setCurrentIndex(0);

    m_stack->setCurrentWidget(m_addBookPage);
    m_currentCoverPath.clear();
    m_coverPreviewLabel->setPixmap(QPixmap());
    m_coverPreviewLabel->setText("No cover");
}

void MainWindow::onSaveBookClicked()
{
    const QString title = m_titleEdit->text().trimmed();
    const QString author = m_authorEdit->text().trimmed();
    const QString genre = m_genreEdit->text().trimmed();
    const int year = m_yearEdit->text().toInt();
    const QString status = m_statusCombo->currentText();

    if (title.isEmpty() || author.isEmpty() || genre.isEmpty() || year <= 0) {
        QMessageBox::warning(this, tr("Помилка"), tr("Заповніть усі поля та коректний рік."));
        return;
    }

    int row = m_booksModel->rowCount();
    if (!m_booksModel->insertRow(row)) {
        QMessageBox::warning(this, tr("Помилка"), tr("Не вдалося вставити рядок у модель."));
        return;
    }

    m_booksModel->setData(m_booksModel->index(row, 1), title);
    m_booksModel->setData(m_booksModel->index(row, 2), author);
    m_booksModel->setData(m_booksModel->index(row, 3), genre);
    m_booksModel->setData(m_booksModel->index(row, 4), year);
    m_booksModel->setData(m_booksModel->index(row, 5), status);
    m_booksModel->setData(m_booksModel->index(row, 6), m_currentCoverPath);

    if (!m_booksModel->submitAll()) {
        QMessageBox::warning(this, tr("Помилка"), m_booksModel->lastError().text());
        m_booksModel->revertAll();
        return;
    }

    m_booksModel->select();
    showLibrary();
}

void MainWindow::onChooseCoverClicked()
{
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Виберіть обкладинку"),
                                                QString(),
                                                tr("Images *.png *.jpg *.jpeg *.bmp"));
    if (file.isEmpty())
        return;

    QPixmap pix(file);
    if (pix.isNull())
        return;

    m_currentCoverPath = file;
    m_coverPreviewLabel->setPixmap(pix);
    m_coverPreviewLabel->setText(QString());
}

void MainWindow::onCurrentBookChanged(const QModelIndex &current,
                                      const QModelIndex &previous)
{
    Q_UNUSED(previous);

    if(!current.isValid())
        return;

    QString title = m_booksModel -> data(
                                    m_booksModel -> index(current.row(),1)
                                    ).toString();
    qDebug() << "Selected book" << title;
}

void MainWindow::showBookDetails(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    int row = index.row();

    QString title  = m_booksModel->data(m_booksModel->index(row, 1)).toString();
    QString author = m_booksModel->data(m_booksModel->index(row, 2)).toString();
    QString genre  = m_booksModel->data(m_booksModel->index(row, 3)).toString();
    QString year   = m_booksModel->data(m_booksModel->index(row, 4)).toString();
    QString status = m_booksModel->data(m_booksModel->index(row, 5)).toString();
    QString cover  = m_booksModel->data(m_booksModel->index(row, 6)).toString();

    m_detailsTitleLabel->setText(title);
    m_detailsAuthorLabel->setText(author);
    m_detailsGenreLabel->setText(genre);
    m_detailsYearLabel->setText(year);
    m_detailsStatusLabel->setText(status);

    QPixmap pix(cover);
    if(!pix.isNull())
    {
        m_detailsCoverLabel ->setPixmap(
            pix.scaled(m_detailsCoverLabel->size(),
                       Qt::KeepAspectRatio,
                       Qt::SmoothTransformation));
    }
    else
    {
        m_detailsCoverLabel -> setPixmap(QPixmap());
        m_detailsCoverLabel -> setText(tr("No cover"));
    }

    m_stack -> setCurrentWidget(m_bookDetailsPage);
}

void MainWindow::onDetailsBackClicked()
{
    m_stack->setCurrentWidget(m_libraryWidget);
}


void MainWindow::onBookInfoRequested(const QModelIndex &index)
{
    showBookDetails(index);
}

void MainWindow::onTakeRequested(const QModelIndex &proxyIndex)
{
    if (!proxyIndex.isValid())
        return;

    if (m_currentUserId <= 0) {
        QMessageBox::warning(this, tr("Помилка"), tr("Користувач не авторизований"));
        return;
    }

    auto *proxy = qobject_cast<BooksFilterProxyModel*>(m_booksProxyModel);
    if (!proxy) {
        QMessageBox::warning(this, tr("Помилка"), tr("Proxy model is not available"));
        return;
    }

    QModelIndex sourceIndex = proxy->mapToSource(proxyIndex);
    if (!sourceIndex.isValid())
        return;

    int row = sourceIndex.row();

    // Отримуємо id книги з моделі
    int bookId = m_booksModel->bookIdAtRow(row);
    QString title  = m_booksModel->data(m_booksModel->index(row, 1)).toString();
    QString status = m_booksModel->data(m_booksModel->index(row, 5)).toString();

    qDebug() << "onTakeRequested: source row =" << row
             << "title =" << title
             << "bookId =" << bookId
             << "status =" << status;

    if (status == "loaned") {
        QMessageBox::information(this, tr("Недоступно"), tr("Книга вже видана"));
        return;
    }

    // Ліміт активних позик
    const int maxActiveLoans = 5;
    {
        QSqlQuery q(database::db());
        q.prepare("SELECT COUNT(*) FROM Loans "
                  "WHERE user_id = :u AND status = 'active'");
        q.bindValue(":u", m_currentUserId);

        if (!q.exec() || !q.next()) {
            QMessageBox::warning(this, tr("Помилка"),
                                 tr("Не вдалося перевірити кількість позик"));
            return;
        }

        int activeCount = q.value(0).toInt();
        if (activeCount >= maxActiveLoans) {
            QMessageBox::information(this, tr("Ліміт позик"),
                                     tr("Ви вже маєте %1 активних позик.")
                                         .arg(maxActiveLoans));
            return;
        }
    }

    qDebug() << "About to call takeBook, userId =" << m_currentUserId
             << "bookId =" << bookId;

    bool ok = database::takeBook(m_currentUserId, bookId);
    qDebug() << "takeBook result =" << ok;

    if (!ok) {
        QMessageBox::warning(this, tr("Помилка"), tr("Не вдалося оформити позику"));
        return;
    }

    m_booksModel->select();
    m_booksListView->viewport()->update();
    refreshLoans();
}


void MainWindow::refreshLoans()
{
    if(m_currentUserId <= 0){
        return;
    }

    {
        QSqlQuery q(database::db());
        q.prepare("UPDATE Loans "
                  "SET status = 'overdue' "
                  "WHERE user_id = :u "
                  "  AND status = 'active' "
                  "  AND due_date IS NOT NULL "
                  "  AND due_date < DATE('now')");
        q.bindValue(":u", m_currentUserId);
        if (!q.exec()) {
            qDebug() << "refreshLoans: failed to update overdue loans:"
                     << q.lastError().text();
        }
    }

    qDebug() << "current user id =" << m_currentUserId;
    QString sql =
        "SELECT Loans.id, "
        "       Books.title, "
        "       Books.cover_path, "
        "       Loans.issue_date, "
        "       Loans.due_date, "
        "       Loans.return_date, "
        "       Loans.status "
        "FROM Loans "
        "JOIN Books ON Loans.book_id = Books.id "
        "WHERE Loans.user_id = %1";

    QString finalSql = sql.arg(m_currentUserId);
    m_loansModel->setQuery(finalSql, database::db());
    qDebug() << "refreshLoans error:" << m_loansModel->lastError().text();
    qDebug() << "refreshLoans rowCount:" << m_loansModel->rowCount();

    m_loansModel->setHeaderData(1, Qt::Horizontal, tr("Назва"));
    m_loansModel->setHeaderData(3, Qt::Horizontal, tr("Дата видачі"));
    m_loansModel->setHeaderData(4, Qt::Horizontal, tr("Повернути до"));
    m_loansModel->setHeaderData(5, Qt::Horizontal, tr("Дата повернення"));
    m_loansModel->setHeaderData(6, Qt::Horizontal, tr("Статус"));

    m_cabinetWidget->setLoansModel(m_loansModel);

}

void MainWindow::showFiltersPopup(){
    if (!m_booksModel || !m_filtersButton)
        return;

    QWidget *popup = new QWidget(this, Qt::Popup);
    auto *layout = new QVBoxLayout(popup);

    auto *authorCombo = new QComboBox(popup);
    auto *genreCombo  = new QComboBox(popup);
    auto *yearCombo   = new QComboBox(popup);

    authorCombo->addItem(tr("Всі автори"), QVariant());
    genreCombo->addItem(tr("Всі жанри"), QVariant());
    yearCombo->addItem(tr("Всі роки"), QVariant());

    QSet<QString> authors, genres, years;
    for (int r = 0; r < m_booksModel->rowCount(); ++r) {
        authors.insert(m_booksModel->data(m_booksModel->index(r, 2)).toString());
        genres.insert(m_booksModel->data(m_booksModel->index(r, 3)).toString());
        years.insert(m_booksModel->data(m_booksModel->index(r, 4)).toString());
    }
    for (const QString &a : std::as_const(authors))
        authorCombo->addItem(a, a);
    for (const QString &g : std::as_const(genres))
        genreCombo->addItem(g, g);
    for (const QString &y : std::as_const(years))
        yearCombo->addItem(y, y);

    layout->addWidget(new QLabel(tr("Автор:"), popup));
    layout->addWidget(authorCombo);
    layout->addWidget(new QLabel(tr("Жанр:"), popup));
    layout->addWidget(genreCombo);
    layout->addWidget(new QLabel(tr("Рік:"), popup));
    layout->addWidget(yearCombo);

    popup->setLayout(layout);

    auto *proxy = qobject_cast<BooksFilterProxyModel*>(m_booksProxyModel);
    if (!proxy) {
        popup->show();
        return;
    }

    connect(authorCombo, &QComboBox::currentIndexChanged,
            popup, [proxy, authorCombo](int){
                QString val = authorCombo->currentData().toString();
                proxy->setAuthorFilter(val);
            });
    connect(genreCombo, &QComboBox::currentIndexChanged,
            popup, [proxy, genreCombo](int){
                QString val = genreCombo->currentData().toString();
                proxy->setGenreFilter(val);
            });
    connect(yearCombo, &QComboBox::currentIndexChanged,
            popup, [proxy, yearCombo](int){
                QString val = yearCombo->currentData().toString();
                proxy->setYearFilter(val);
            });

    QPoint pos = m_filtersButton->mapToGlobal(QPoint(0, m_filtersButton->height()));
    popup->move(pos);
    popup->show();
}

void MainWindow::showAdminCabinet()
{
    if (m_currentUserRole != "admin") {
        QMessageBox::warning(this, tr("Доступ заборонено"),
                             tr("Ця сторінка доступна лише адміністратору"));
        return;
    }

    QString sql =
        "SELECT Loans.id, "
        "       Users.full_name, "
        "       Books.title, "
        "       Loans.issue_date, "
        "       Loans.due_date, "
        "       Loans.return_date, "
        "       Loans.status, "
        "       Loans.book_id "
        "FROM Loans "
        "JOIN Users ON Loans.user_id = Users.id "
        "JOIN Books ON Loans.book_id = Books.id";

    m_loansModel->setQuery(sql, database::db());

    m_loansModel->setHeaderData(1, Qt::Horizontal, tr("Користувач"));
    m_loansModel->setHeaderData(2, Qt::Horizontal, tr("Книга"));
    m_loansModel->setHeaderData(3, Qt::Horizontal, tr("Дата видачі"));
    m_loansModel->setHeaderData(4, Qt::Horizontal, tr("Повернути до"));
    m_loansModel->setHeaderData(5, Qt::Horizontal, tr("Дата повернення"));
    m_loansModel->setHeaderData(6, Qt::Horizontal, tr("Статус"));

    m_adminCabinet->setLoansModel(m_loansModel);
    refreshAdminStats();
    m_stack->setCurrentWidget(m_adminCabinet);
}

void MainWindow::onAdminReturnLoanRequested(const QModelIndex &index)
{
    if (m_currentUserRole != "admin")
        return;

    if (!index.isValid() || !m_loansModel)
        return;

    int row = index.row();

    int loanId = m_loansModel->data(m_loansModel->index(row, 0)).toInt();
    int bookId = m_loansModel->data(m_loansModel->index(row, 7)).toInt();
    QString status = m_loansModel->data(m_loansModel->index(row, 6)).toString();

    if (status == "returned") {
        QMessageBox::information(this, tr("Вже повернуто"),
                                 tr("Ця книга вже позначена як повернена."));
        return;
    }

    QSqlQuery q(database::db());
    q.prepare("UPDATE Loans "
              "SET status = 'returned', return_date = DATE('now') "
              "WHERE id = :id");
    q.bindValue(":id", loanId);
    if (!q.exec()) {
        QMessageBox::warning(this, tr("Помилка"),
                             tr("Не вдалося оновити позику"));
        return;
    }

    QSqlQuery qb(database::db());
    qb.prepare("UPDATE Books SET status = 'available' WHERE id = :id");
    qb.bindValue(":id", bookId);
    if (!qb.exec()) {
        QMessageBox::warning(this, tr("Увага"),
                             tr("Позику оновлено, але статус книги не змінено"));
    }


    m_booksModel->select();
    refreshAdminStats();
    showAdminCabinet();
    m_booksListView->viewport()->update();
}

void MainWindow::refreshAdminStats()
{
    if (!m_adminCabinet)
        return;

    auto getCount = [](const QString &sql) -> int {
        QSqlQuery q(database::db());
        if (!q.exec(sql) || !q.next())
            return 0;
        return q.value(0).toInt();
    };

    int totalBooks     = getCount("SELECT COUNT(*) FROM Books");
    int availableBooks = getCount("SELECT COUNT(*) FROM Books WHERE status = 'available'");
    int loanedBooks    = getCount("SELECT COUNT(*) FROM Books WHERE status = 'loaned'");
    int activeLoans    = getCount("SELECT COUNT(*) FROM Loans WHERE status = 'active'");
    int overdueLoans   = getCount("SELECT COUNT(*) FROM Loans WHERE status = 'overdue'");
    int totalUsers     = getCount("SELECT COUNT(*) FROM Users WHERE role = 'user'");

    m_adminCabinet->setStats(totalBooks,
                             availableBooks,
                             loanedBooks,
                             activeLoans,
                             overdueLoans,
                             totalUsers);
}


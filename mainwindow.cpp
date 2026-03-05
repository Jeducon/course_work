#include "mainwindow.h"
#include "bookcarddelegate.h"
#include "loginwidget.h"
#include "usercabinet.h"
#include "database.h"
#include "addbookdialog.h"
#include "booksmodel.h"

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


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_stack = new QStackedWidget(this);

    m_booksModel = new booksmodel(this, database::db());
    m_booksModel->setTable("Books");
    m_booksModel->select();

    m_booksModel->setHeaderData(1, Qt::Horizontal, tr("Назва"));
    m_booksModel->setHeaderData(2, Qt::Horizontal, tr("Автор"));
    m_booksModel->setHeaderData(3, Qt::Horizontal, tr("Жанр"));
    m_booksModel->setHeaderData(4, Qt::Horizontal, tr("Рік"));
    m_booksModel->setHeaderData(5, Qt::Horizontal, tr("Статус"));

    setCentralWidget(m_stack);

    m_loansModel = new QSqlTableModel(this, database::db());
    m_loansModel -> setTable("Loans");

    m_loginWidget = new LoginWidget(this);
    m_libraryWidget = setupLibraryPage();

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

    m_stack->addWidget(m_loginWidget);
    m_stack->addWidget(m_libraryWidget);

    m_stack->addWidget(m_cabinetWidget);

    m_stack->setCurrentWidget(m_libraryWidget);

    m_stack->addWidget(m_bookDetailsPage);

    connect(m_loginWidget, &LoginWidget::LoginSuccess,
            this, &MainWindow::onLoginSuccess);

    connect(m_cabinetWidget, &usercabinet::backToLibrary, this, &MainWindow::showLibrary);

    connect(m_addBookButton, &QPushButton::clicked, this, &MainWindow::onAddBookClicked);

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

    connect(m_bookCardDelegate, &BookCardDelegate::takeRequested,
            this, &MainWindow::onTakeRequested);

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

    if (m_bookCardDelegate) {
        m_bookCardDelegate->setUserRole(role);
        m_booksListView->viewport()->update();
    }

    m_loansModel -> setFilter(QString("user_id = %1").arg(m_currentUserId));
    m_loansModel -> select();

    m_loansModel -> setHeaderData(0, Qt::Horizontal, tr("ID"));
    m_loansModel->setHeaderData(2, Qt::Horizontal, tr("ID книги"));
    m_loansModel->setHeaderData(3, Qt::Horizontal, tr("Дата видачі"));
    m_loansModel->setHeaderData(4, Qt::Horizontal, tr("Повернути до"));
    m_loansModel->setHeaderData(5, Qt::Horizontal, tr("Дата повернення"));
    m_loansModel->setHeaderData(6, Qt::Horizontal, tr("Статус"));

    m_cabinetWidget -> setLoansModel(m_loansModel);
    m_stack->setCurrentWidget(m_cabinetWidget);

    if(m_authButton){
        m_authButton -> setText("Log out");
    }
}

QWidget *MainWindow::setupLibraryPage()
{
    auto *page = new QWidget(this);

    QPushButton* cabinetButton = new QPushButton(tr("Personal Cabinet"), page);

    m_authButton = new QPushButton(tr("Log in"), page);

    m_booksListView = new QListView(page);
    m_booksListView->setModel(m_booksModel);
    m_booksListView->setViewMode(QListView::IconMode);
    m_booksListView->setResizeMode(QListView::Adjust);
    m_booksListView->setWrapping(true);
    m_booksListView->setSpacing(16);
    m_booksListView->setIconSize(QSize(160, 240));
    m_booksListView->setGridSize(QSize(180, 280));
    m_booksListView->setUniformItemSizes(true);

    m_addBookButton = new QPushButton(tr("Додати книгу"), page);
    m_deleteBooksButton = new QPushButton(tr("Delete Selected"), page);

    auto *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(m_addBookButton);
    buttonsLayout->addWidget(m_deleteBooksButton);
    buttonsLayout->addStretch();

    auto *topLayout = new QHBoxLayout;
    topLayout->addWidget(cabinetButton);
    topLayout->addStretch();
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
    m_cabinetWidget->setUserName(m_currentUserName);
    m_stack->setCurrentWidget(m_cabinetWidget);
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

void MainWindow::onAddBookClicked()
{
    const QString title = m_titleEdit->text().trimmed();
    const QString author = m_authorEdit->text().trimmed();
    const QString genre = m_genreEdit->text().trimmed();
    const int year = m_yearEdit->text().toInt();
    const QString status = m_statusCombo->currentText();

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
        QMessageBox::warning(this, tr("Помилка"), tr("Не вдалося зберегти книгу в базу."));
        m_booksModel->revertAll();
    }

    m_titleEdit->clear();
    m_authorEdit->clear();
    m_genreEdit->clear();
    m_yearEdit->clear();
    m_statusCombo->setCurrentIndex(0);
}

void MainWindow::onRegisterRequested(const QString &login,
                                     const QString &pass,
                                     const QString &fullName,
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
        "VALUES (:u, :p, :r, :fn, '', :ph, :em, '')");
    q.bindValue(":u", login);
    q.bindValue(":p", pass);
    q.bindValue(":r", role);
    q.bindValue(":fn", fullName);
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

    auto *sel = m_booksListView ->selectionModel();
    const auto rows = sel ->selectedRows();

    if (rows.isEmpty()) {
        QMessageBox::information(this, tr("No selection"), tr("Select at least one book to delete"));
        return;
    }

    if (QMessageBox::question(this,
                              tr("Confirm delete"),
                              tr("Delete %1 selected books?").arg(rows.size()))
        != QMessageBox::Yes)
        return;

    QList<int> rowNumbers;
    for (const QModelIndex &idx : rows)
        rowNumbers << idx.row();
    std::sort(rowNumbers.begin(), rowNumbers.end(), std::greater<int>());

    for (int i = 0; i < rowNumbers.size(); i++) {
        int r = rowNumbers.at(i);
        m_booksModel->removeRow(r);
    }

    if (!m_booksModel->submitAll()) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to delete selected books"));
        m_booksModel->revertAll();
    } else {
        m_booksModel->select();
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

    m_detailsTitleLabel->setText(tr("Назва: %1").arg(title));
    m_detailsAuthorLabel->setText(tr("Автор: %1").arg(author));
    m_detailsGenreLabel->setText(tr("Жанр: %1").arg(genre));
    m_detailsYearLabel->setText(tr("Рік: %1").arg(year));
    m_detailsStatusLabel->setText(tr("Статус: %1").arg(status));

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

void MainWindow::onTakeRequested(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    if(m_currentUserId <= 0){
        QMessageBox::warning(this, tr("Помилка"), tr("Користувач не авторизований"));
        return;
    }

    int row = index.row();

    int bookId = m_booksModel -> data(
                m_booksModel -> index(row,0)
                                     ).toInt();

    QString status = m_booksModel -> data(
                m_booksModel -> index(row, 5)
                                     ).toString();

    if(status == "loaned") {
        QMessageBox::information(this, tr("Недоступно"), tr("Книга вже видана"));
        return;
    }

    QDate today = QDate::currentDate();
    QDate due = today.addDays(14);

    QSqlDatabase db = database::db();
    QSqlQuery q(db);

    q.prepare("INSERT INTO Loans (user_id, book_id, issue_date, due_date, status) "
              "VALUES (:u, :b, :issue, :due, 'active')");
    q.bindValue(":u", m_currentUserId);
    q.bindValue(":b", bookId);
    q.bindValue(":issue", today.toString("yyyy-MM-dd"));
    q.bindValue(":due",   due.toString("yyyy-MM-dd"));

    if(!q.exec()){
        QMessageBox::critical(this, tr("Помилка"), tr("Не вдалося створити запис видачі: %1")
                                                       .arg(q.lastError().text()));
        return;
    }

    QSqlQuery qb(db);
    qb.prepare("UPDATE Books SET status = 'loaned' WHERE id = :id");
    qb.bindValue(":id", bookId);
    if(!qb.exec()){
        QMessageBox::warning(this, tr("Увага"), tr("Видачу створено, але статус книги не оновлено: %1")
                                                        .arg(qb.lastError().text()));
    }
    m_booksModel -> select();
    m_loansModel -> select();
}


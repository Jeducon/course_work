#include "mainwindow.h"
#include "loginwidget.h"
#include "usercabinet.h"
#include "database.h"
#include "addbookdialog.h"


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


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_stack = new QStackedWidget(this);

    m_booksModel = new QSqlTableModel(this, database::db());
    m_booksModel -> setTable("Books");
    m_booksModel -> select();

    setCentralWidget(m_stack);

    m_loginWidget   = new LoginWidget(this);
    m_libraryWidget = setupLibraryPage();
    m_addBookPage = new QWidget(this);

    m_titleEdit  = new QLineEdit(m_addBookPage);
    m_authorEdit = new QLineEdit(m_addBookPage);
    m_genreEdit  = new QLineEdit(m_addBookPage);
    m_yearEdit   = new QLineEdit(m_addBookPage);
    m_statusCombo = new QComboBox(m_addBookPage);
    m_statusCombo->addItems({ tr("available"), tr("loaned") });

    m_coverPreviewLabel = new QLabel(m_addBookPage);
    m_coverPreviewLabel->setFixedSize(120, 160);
    m_coverPreviewLabel->setFrameShape(QFrame::Box);
    m_coverPreviewLabel->setAlignment(Qt::AlignCenter);
    m_coverPreviewLabel->setText(tr("No cover"));
    m_coverPreviewLabel->setScaledContents(true);

    m_chooseCoverButton = new QPushButton(tr("Обрати обкладинку"), m_addBookPage);
    m_saveBookButton       = new QPushButton(tr("Зберегти книгу"), m_addBookPage);
    m_cancelAddBookButton  = new QPushButton(tr("Назад до каталогу"), m_addBookPage);

    auto *formLayout = new QFormLayout;
    formLayout->addRow(tr("Назва:"),  m_titleEdit);
    formLayout->addRow(tr("Автор:"),  m_authorEdit);
    formLayout->addRow(tr("Жанр:"),   m_genreEdit);
    formLayout->addRow(tr("Рік:"),    m_yearEdit);
    formLayout->addRow(tr("Статус:"), m_statusCombo);

    auto *coverLayout = new QHBoxLayout;
    coverLayout -> addWidget(m_coverPreviewLabel);
    coverLayout -> addWidget(m_chooseCoverButton);
    coverLayout -> addStretch();

    auto *topLayout = new QHBoxLayout;
    topLayout -> addLayout(formLayout);
    topLayout -> addLayout(coverLayout);

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

    m_stack->setCurrentWidget(m_loginWidget);

    connect(m_loginWidget, &LoginWidget::LoginSuccess,
            this, &MainWindow::onLoginSuccess);

    connect(m_cabinetWidget, &usercabinet::backToLibrary,
            this, &MainWindow::showLibrary);

    connect(m_addBookButton, &QPushButton::clicked,
            this, &MainWindow::onAddBookClicked);

    connect(m_loginWidget, &LoginWidget::RegisterRequested,
            this, &MainWindow::onRegisterRequested);

    connect(m_deleteBooksButton, &QPushButton::clicked,
            this, &MainWindow::onDeleteBooksClicked);

    connect(m_loginWidget, &LoginWidget::RegistrationSucceeded,
            this, &MainWindow::onRegistrationSucceeded);

    connect(m_cabinetWidget, &usercabinet::logoutRequested, this,
            &MainWindow::onLogoutClicked);

    connect(m_saveBookButton, &QPushButton::clicked,
            this, &MainWindow::onSaveBookClicked);

    connect(m_cancelAddBookButton, &QPushButton::clicked,
            this, &MainWindow::showLibrary);

    connect(m_chooseCoverButton, &QPushButton::clicked,
            this, &MainWindow::onChooseCoverClicked);

}

void MainWindow::onLoginSuccess(const QString &username, const QString &role)
{
    m_currentUserName = username;
    m_currentUserRole = role;

    m_cabinetWidget -> setUserName(username);

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
    q.prepare("SELECT full_name, address, phone, email, photo_path "
              "FROM Users WHERE username = :u");
    q.bindValue(":u", username);
    if (q.exec() && q.next()) {
        const QString fullName  = q.value(0).toString();
        const QString address   = q.value(1).toString();
        const QString phone     = q.value(2).toString();
        const QString email     = q.value(3).toString();
        const QString photoPath = q.value(4).toString();

        m_cabinetWidget->setUserInfo(fullName, address, phone, email);
        m_cabinetWidget->setUserPhoto(photoPath);
    }

    m_stack->setCurrentWidget(m_cabinetWidget);
}

QWidget* MainWindow::setupLibraryPage()
{
    auto *page = new QWidget(this);

    QPushButton* cabinetButton = new QPushButton(tr("Personal Cabinet"), page);


    m_booksView = new QTableView(page);
    m_booksView -> setModel(m_booksModel);
    m_booksView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_booksView->verticalHeader()->setVisible(false);

    m_booksView -> setSelectionMode(QAbstractItemView::MultiSelection);
    m_booksView -> setSelectionBehavior(QAbstractItemView::SelectRows);

    m_booksModel->setHeaderData(1, Qt::Horizontal, tr("Назва"));
    m_booksModel->setHeaderData(2, Qt::Horizontal, tr("Автор"));
    m_booksModel->setHeaderData(3, Qt::Horizontal, tr("Жанр"));
    m_booksModel->setHeaderData(4, Qt::Horizontal, tr("Рік"));
    m_booksModel->setHeaderData(5, Qt::Horizontal, tr("Статус"));

    m_addBookButton = new QPushButton(tr("Додати книгу"), page);
    m_deleteBooksButton = new QPushButton(tr("Delete Selected"), page);

    auto *buttonsLayout = new QHBoxLayout();
    buttonsLayout -> addWidget(m_addBookButton);
    buttonsLayout -> addWidget(m_deleteBooksButton);
    buttonsLayout -> addStretch();

    auto *topLayout = new QHBoxLayout;
    topLayout -> addWidget(cabinetButton);
    topLayout -> addStretch();

    auto *layout = new QVBoxLayout(page);
    layout -> addLayout(topLayout);
    layout -> addWidget(m_booksView);
    layout -> addLayout(buttonsLayout);
    page->setLayout(layout);

    connect(m_addBookButton, &QPushButton::clicked,
            this, &MainWindow::onAddBookPageRequested);

    connect(cabinetButton, &QPushButton::clicked,
            this, &MainWindow::showUserCabinet);

    return page;
}

void MainWindow::showUserCabinet()
{
    m_cabinetWidget->setUserName(m_currentUserName);
    m_stack->setCurrentWidget(m_cabinetWidget);
}

void MainWindow::showLibrary()
{
    m_stack->setCurrentWidget(m_libraryWidget);
}

void MainWindow::onAddBookClicked()
{
    const QString title  = m_titleEdit->text().trimmed();
    const QString author = m_authorEdit->text().trimmed();
    const QString genre  = m_genreEdit->text().trimmed();
    const int     year   = m_yearEdit->text().toInt();
    const QString status = m_statusCombo->currentText();

    int row = m_booksModel->rowCount();
    if (!m_booksModel->insertRow(row)) {
        QMessageBox::warning(this, tr("Помилка"),
                             tr("Не вдалося вставити рядок у модель."));
        return;
    }

    m_booksModel->setData(m_booksModel->index(row, 1), title);
    m_booksModel->setData(m_booksModel->index(row, 2), author);
    m_booksModel->setData(m_booksModel->index(row, 3), genre);
    m_booksModel->setData(m_booksModel->index(row, 4), year);
    m_booksModel->setData(m_booksModel->index(row, 5), status);

    if (!m_booksModel->submitAll()) {
        QMessageBox::warning(this, tr("Помилка"),
                             tr("Не вдалося зберегти книгу в базу."));
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
    if (login.isEmpty() || pass.isEmpty() ||
        fullName.isEmpty() || phone.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, tr("Error"),
                             tr("All fields must be filled"));
        return;
    }

    const QString role = "user";

    QSqlQuery q(database::db());
    q.prepare("INSERT INTO Users(username, password, role, full_name, address, phone, email, photo_path) "
              "VALUES (:u, :p, :r, :fn, '', :ph, :em, '')");
    q.bindValue(":u",  login);
    q.bindValue(":p",  pass);
    q.bindValue(":r",  role);
    q.bindValue(":fn", fullName);
    q.bindValue(":ph", phone);
    q.bindValue(":em", email);

    if (!q.exec()) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Can't register, maybe someone took your login"));
        return;
    }

}

void MainWindow::onDeleteBooksClicked(){
    if(m_currentUserRole != "admin"){
        QMessageBox::warning(this, tr("Access Denied"), tr("Only admin can delete books"));
        return;
    }

    auto *sel = m_booksView ->selectionModel();
    const auto rows = sel ->selectedRows();

    if(rows.isEmpty()){
        QMessageBox::information(this, tr("No selection"), tr("Select at least one book to delete"));
        return;
    }

    if(QMessageBox::question(this, tr("Confirm delete"), tr("Delete %1 selected books?").arg(rows.size()))
    != QMessageBox::Yes)
    return;

    QList<int>rowNumbers;
    for(const QModelIndex &idx : rows)
        rowNumbers << idx.row();
    std::sort(rowNumbers.begin(), rowNumbers.end(), std::greater<int>());

    for(int i = 0; i < rowNumbers.size(); i++){
        int r = rowNumbers.at(i);
        m_booksModel->removeRow(r);
    }

    if(!m_booksModel -> submitAll()){
        QMessageBox::warning(this, tr("Error"), tr("Failed to delete selected books"));
        m_booksModel -> revertAll();
    }
    else{
        m_booksModel -> select();
    }
}

void MainWindow::onRegistrationSucceeded()
{
    m_loginWidget->showLoginPage();
    m_loginWidget->setLoginStatusMessage(
        tr("Реєстрація успішна. Увійдіть у свій акаунт.")
        );
}

void MainWindow::onLogoutClicked(){
    m_currentUserName.clear();
    m_currentUserRole.clear();

    m_loginWidget -> clearFields();
    m_loginWidget -> showLoginPage();
    m_stack -> setCurrentWidget(m_loginWidget);
}

void MainWindow::onAddBookPageRequested(){
    m_titleEdit->clear();
    m_authorEdit->clear();
    m_genreEdit->clear();
    m_yearEdit->clear();
    m_statusCombo->setCurrentIndex(0);

    m_stack->setCurrentWidget(m_addBookPage);
    m_currentCoverPath.clear();
    m_coverPreviewLabel -> setPixmap(QPixmap());
    m_coverPreviewLabel -> setText("No cover");
}

void MainWindow::onSaveBookClicked(){
    const QString title  = m_titleEdit->text().trimmed();
    const QString author = m_authorEdit->text().trimmed();
    const QString genre  = m_genreEdit->text().trimmed();
    const int     year   = m_yearEdit->text().toInt();
    const QString status = m_statusCombo->currentText();

    if (title.isEmpty() || author.isEmpty() || genre.isEmpty() || year <= 0) {
        QMessageBox::warning(this, tr("Помилка"),
                             tr("Заповніть усі поля та коректний рік."));
        return;
    }

    int row = m_booksModel->rowCount();
    if (!m_booksModel->insertRow(row)) {
        QMessageBox::warning(this, tr("Помилка"),
                             tr("Не вдалося вставити рядок у модель."));
        return;
    }

    m_booksModel->setData(m_booksModel->index(row, 1), title);
    m_booksModel->setData(m_booksModel->index(row, 2), author);
    m_booksModel->setData(m_booksModel->index(row, 3), genre);
    m_booksModel->setData(m_booksModel->index(row, 4), year);
    m_booksModel->setData(m_booksModel->index(row, 5), status);
    m_booksModel->setData(m_booksModel->index(row, 6), m_currentCoverPath);

    if (!m_booksModel->submitAll()) {
        QMessageBox::warning(this, tr("Помилка"),
                             m_booksModel->lastError().text());
        m_booksModel->revertAll();
        return;
    }

    showLibrary();
}

void MainWindow::onChooseCoverClicked(){
    QString file = QFileDialog::getOpenFileName(
        this,
        tr("Виберіть обкладинку"),
        QString(),
        tr("Images *.png *.jpg *.jpeg *.bmp")
    );
    if(file.isEmpty())
        return;

    QPixmap pix(file);
    if(pix.isNull())
        return;

    m_currentCoverPath = file;
    m_coverPreviewLabel -> setPixmap(pix);
    m_coverPreviewLabel -> setText(QString());
}







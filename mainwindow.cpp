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
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include <QTextStream>
#include <QPrinter>
#include <QPageLayout>
#include <QPageSize>
#include <QTextDocument>
#include <algorithm>
#include <QFrame>
#include <QImageReader>
#include <QImageWriter>
#include <QBuffer>
#include <QInputDialog>
#include <QSqlRecord>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QCompleter>
#include <QStringListModel>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

static void migrateCoverPaths()
{
    QSqlQuery q(database::db());
    if (!q.exec("SELECT id, cover_path FROM Books")) {
        qDebug() << "migrateCoverPaths: select failed" << q.lastError().text();
        return;
    }

    QSqlQuery upd(database::db());
    upd.prepare("UPDATE Books SET cover_path = :rel WHERE id = :id");

    while (q.next()) {
        const int id = q.value(0).toInt();
        const QString path = q.value(1).toString();

        if (path.isEmpty() || !QDir::isAbsolutePath(path))
            continue;

        int idx = path.indexOf("covers/");
        if (idx < 0)
            idx = path.indexOf("covers\\");
        if (idx < 0)
            continue;

        const QString relative = path.mid(idx);

        upd.bindValue(":rel", relative);
        upd.bindValue(":id", id);
        if (!upd.exec()) {
            qDebug() << "migrateCoverPaths: update failed for id" << id
                     << upd.lastError().text();
        }
    }
}

static QString copyCoverToLocalStorage(const QString &sourcePath)
{
    if (sourcePath.isEmpty())
        return QString();

    QFileInfo info(sourcePath);
    if (!info.exists() || !info.isFile())
        return QString();

    QDir appDir(QCoreApplication::applicationDirPath());
    if (!appDir.exists("covers")) {
        appDir.mkpath("covers");
    }

    QImageReader reader(sourcePath);
    reader.setAutoTransform(true);
    QImage image = reader.read();

    if (image.isNull())
        return QString();

    const int maxSide = 600;
    if (image.width() > maxSide || image.height() > maxSide) {
        image = image.scaled(maxSide, maxSide,
                             Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);
    }

    const QString fileName =
        QUuid::createUuid().toString(QUuid::WithoutBraces) + ".jpg";
    const QString relativePath = "covers/" + fileName;
    const QString absolutePath = appDir.filePath(relativePath);
    QImageWriter writer(absolutePath, "jpg");
    writer.setQuality(80);

    if (!writer.write(image))
        return QString();

    return relativePath;
}

static QString resolveCoversPath(const QString &storedPath)
{
    if (storedPath.isEmpty())
        return QString();
    if (QDir::isAbsolutePath(storedPath))
        return storedPath;
    return QDir(QCoreApplication::applicationDirPath()).filePath(storedPath);
}

static void optimizeExistingCovers()
{
    QDir appDir(QCoreApplication::applicationDirPath());
    QDir coversDir(appDir.filePath("covers"));

    if (!coversDir.exists())
        return;

    const QStringList filters = { "*.jpg", "*.jpeg", "*.png", "*.bmp", "*.webp" };
    const QFileInfoList files = coversDir.entryInfoList(filters, QDir::Files);

    const int maxSide = 600;
    const qint64 maxBytes = 300 * 1024;

    for (const QFileInfo &fi : files) {
        if (fi.size() <= maxBytes)
            continue;

        QImageReader reader(fi.absoluteFilePath());
        reader.setAutoTransform(true);
        QImage image = reader.read();

        if (image.isNull())
            continue;

        if (image.width() > maxSide || image.height() > maxSide) {
            image = image.scaled(maxSide, maxSide,
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
        }

        QImageWriter writer(fi.absoluteFilePath(), "jpg");
        writer.setQuality(80);
        writer.write(image);
    }
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    optimizeExistingCovers();
    m_stack = new QStackedWidget(this);

    m_booksModel = new booksmodel(this, database::db());
    m_booksModel->setTable("Books");
    m_booksModel->select();

    m_booksProxyModel = new BooksFilterProxyModel(this);
    m_booksProxyModel->setSourceModel(m_booksModel);
    m_booksProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_booksModel->setHeaderData(booksmodel::TitleColumn,  Qt::Horizontal, tr("Назва"));
    m_booksModel->setHeaderData(booksmodel::AuthorIdColumn, Qt::Horizontal, tr("Автор"));
    m_booksModel->setHeaderData(booksmodel::GenreColumn,  Qt::Horizontal, tr("Жанр"));
    m_booksModel->setHeaderData(booksmodel::YearColumn,   Qt::Horizontal, tr("Рік"));
    m_booksModel->setHeaderData(booksmodel::StatusColumn, Qt::Horizontal, tr("Статус"));
    setCentralWidget(m_stack);

    m_loansModel = new LoansModel(this);
    m_badUsersModel = new QSqlQueryModel(this);
    m_goodUsersModel = new QSqlQueryModel(this);

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

    m_authorCombo = new QComboBox(m_addBookPage);
    m_authorCombo->setEditable(false);
    m_addAuthorButton = new QPushButton(tr("Додати автора"), m_addBookPage);

    m_genreEdit = new QLineEdit(m_addBookPage);
    m_yearEdit = new QLineEdit(m_addBookPage);
    m_statusCombo = new QComboBox(m_addBookPage);
    m_statusCombo->addItems({tr("available"), tr("loaned")});
    m_descriptionEdit = new QTextEdit(m_addBookPage);
    m_descriptionEdit->setPlaceholderText(tr("Опис книги..."));
    m_descriptionEdit->setFixedHeight(100);

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
    m_detailsReadersCountLabel = new QLabel(m_bookDetailsPage);
    m_detailsDescriptionEdit = new QTextEdit(m_bookDetailsPage);
    m_detailsDescriptionEdit->setReadOnly(true);
    m_detailsDescriptionEdit->setMinimumHeight(120);
    m_detailsDescriptionEdit->setPlaceholderText(tr("Опис відсутній."));
    m_detailsTitleLabel->setStyleSheet("font-weight: bold; font-size: 18px;");

    m_detailsBackButton = new QPushButton(tr("Назад до каталогу"), m_bookDetailsPage);

    auto *detailsForm = new QFormLayout;
    detailsForm->addRow(tr("Назва:"),  m_detailsTitleLabel);
    detailsForm->addRow(tr("Автор:"),  m_detailsAuthorLabel);
    detailsForm->addRow(tr("Жанр:"),   m_detailsGenreLabel);
    detailsForm->addRow(tr("Рік:"),    m_detailsYearLabel);
    detailsForm->addRow(tr("Статус:"), m_detailsStatusLabel);
    detailsForm->addRow(tr("Унікальних читачів:"), m_detailsReadersCountLabel);
    detailsForm->addRow(tr("Опис:"), m_detailsDescriptionEdit);

    auto *detailsTopLayout = new QHBoxLayout;
    detailsTopLayout->addWidget(m_detailsCoverLabel);
    detailsTopLayout->addLayout(detailsForm);

    auto *detailsPageLayout = new QVBoxLayout(m_bookDetailsPage);
    detailsPageLayout->addLayout(detailsTopLayout);
    detailsPageLayout->addStretch();
    detailsPageLayout->addWidget(m_detailsBackButton, 0, Qt::AlignRight);
    m_bookDetailsPage->setLayout(detailsPageLayout);

    auto *authorRowLayout = new QHBoxLayout;
    authorRowLayout->addWidget(m_authorCombo, 1);
    authorRowLayout->addWidget(m_addAuthorButton);

    auto *formLayout = new QFormLayout;
    formLayout->addRow(tr("Назва:"), m_titleEdit);
    formLayout->addRow(tr("Жанр:"), m_genreEdit);
    formLayout->addRow(tr("Рік:"), m_yearEdit);
    formLayout->addRow(tr("Автор:"), authorRowLayout);
    formLayout->addRow(tr("Статус:"), m_statusCombo);
    formLayout->addRow(tr("Опис:"), m_descriptionEdit);

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

    reloadAuthors();

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

    connect(m_cancelAddBookButton, &QPushButton::clicked, this, [this]() {
        m_isEditMode = false;
        m_editBookId = -1;
        m_saveBookButton->setText(tr("Зберегти книгу"));
        showLibrary();
    });

    connect(m_chooseCoverButton, &QPushButton::clicked, this, &MainWindow::onChooseCoverClicked);

    connect(m_bookCardDelegate, &BookCardDelegate::bookInfoRequested,
            this, &MainWindow::onBookInfoRequested);

    connect(m_booksListView, &QListView::doubleClicked,
            this, &MainWindow::onBookDoubleClicked);

    connect(m_detailsBackButton, &QPushButton::clicked,
            this, &MainWindow::onDetailsBackClicked);

    bool ok = connect(m_bookCardDelegate, &BookCardDelegate::takeRequested,
            this, &MainWindow::onTakeRequested);

    qDebug() << "connect takeRequested:" << ok;

    connect(m_adminCabinet, &AdminCabinet::backToLibrary,
            this, &MainWindow::showLibrary);

    connect(m_adminCabinet, &AdminCabinet::returnLoanRequested,
            this, &MainWindow::onAdminReturnLoanRequested);

    connect(m_adminCabinet, &AdminCabinet::chartTypeChanged,
            this, &MainWindow::onChartTypeChanged);

    connect(m_cabinetWidget, &usercabinet::chartTypeChanged,
            this, &MainWindow::onUserChartTypeChanged);

    connect(m_adminCabinet, &AdminCabinet::exportReportRequested,
            this, &MainWindow::onExportAdminReportRequested);

    connect(m_addAuthorButton, &QPushButton::clicked, this, &MainWindow::onAddAuthorClicked);
}

void MainWindow::onLoginSuccess(const QString &username, const QString &role)
{
    m_currentUserName = username;
    m_currentUserRole = role;

    resetLibraryFilters();

    if (m_cabinetWidget)
        m_cabinetWidget->resetToHomePage();

    if (m_adminCabinet)
        m_adminCabinet->resetToHomePage();

    m_cabinetWidget->setUserName(username);

    if (role == "admin") {
        m_addBookButton->setVisible(true);
        m_addBookButton->setEnabled(true);

        m_editBookButton->setVisible(true);
        m_editBookButton->setEnabled(true);

        m_deleteBooksButton->setVisible(true);
        m_deleteBooksButton->setEnabled(true);
    } else {
        m_addBookButton->setVisible(false);
        m_addBookButton->setEnabled(false);

        m_editBookButton->setVisible(false);
        m_editBookButton->setEnabled(false);

        m_deleteBooksButton->setVisible(false);
        m_deleteBooksButton->setEnabled(false);
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
    refreshReaderStats();
    refreshCurrentUserChart();
    m_cabinetWidget->setLoansModel(m_loansModel);
    m_stack->setCurrentWidget(m_cabinetWidget);

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
        m_authButton -> setText("Вийти");
    }

    if (role == "admin") {
        showAdminCabinet();
    } else {
        m_cabinetWidget->setLoansModel(m_loansModel);
        m_stack->setCurrentWidget(m_cabinetWidget);
    }
}

QWidget* MainWindow::setupLibraryPage()
{
    auto page = new QWidget(this);

    auto cabinetButton = new QPushButton(tr("Персональний кабінет"), page);
    cabinetButton->setProperty("nav", true);

    m_authButton = new QPushButton(tr("Увійти"), page);
    m_authButton->setProperty("nav", true);

    m_filtersButton = new QToolButton(page);
    m_filtersButton->setText(tr("Фільтри"));
    m_filtersButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_filtersButton->setProperty("nav", true);

    cabinetButton->style()->unpolish(cabinetButton);
    cabinetButton->style()->polish(cabinetButton);
    m_authButton->style()->unpolish(m_authButton);
    m_authButton->style()->polish(m_authButton);
    m_filtersButton->style()->unpolish(m_filtersButton);
    m_filtersButton->style()->polish(m_filtersButton);

    m_searchEdit = new QLineEdit(page);
    m_searchEdit->setPlaceholderText(tr("Пошук..."));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMinimumHeight(36);

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
    m_booksListView->setLayoutMode(QListView::Batched);
    m_booksListView->setBatchSize(20);

    m_bookCoverLabel = new QLabel(page);
    m_bookCoverLabel->setVisible(false);

    m_addBookButton = new QPushButton(tr("Додати книгу"), page);
    m_editBookButton = new QPushButton(tr("Редагувати книгу"), page);
    m_deleteBooksButton = new QPushButton(tr("Видалити книгу"), page);

    m_addBookButton->setVisible(false);
    m_addBookButton->setEnabled(false);
    m_editBookButton->setVisible(false);
    m_editBookButton->setEnabled(false);
    m_deleteBooksButton->setVisible(false);
    m_deleteBooksButton->setEnabled(false);

    m_filtersPanel = new QFrame(page);
    m_filtersPanel->setFrameShape(QFrame::StyledPanel);
    m_filtersPanel->setFixedWidth(260);
    m_filtersPanel->setVisible(false);
    m_filtersPanel->setStyleSheet(
        "QFrame {"
        "  background: #f8faf8;"
        "  border: 1px solid #d7e1db;"
        "  border-radius: 10px;"
        "}"
        "QLabel {"
        "  border: none;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "  font-weight: 500;"
        "}"
        "QComboBox, QLineEdit {"
        "  background: white;"
        "  border: 1px solid #cfd8d3;"
        "  border-radius: 8px;"
        "  padding: 6px 10px;"
        "  min-height: 34px;"
        "}"
        "QPushButton {"
        "  background-color: #1f5c42;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  min-height: 36px;"
        "  padding: 0 12px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "  background-color: #174532;"
        "}"
        );

    m_authorFilterCombo = new QComboBox(m_filtersPanel);
    m_genreFilterCombo  = new QComboBox(m_filtersPanel);
    m_yearFilterEdit    = new QLineEdit(m_filtersPanel);
    m_resetFiltersButton = new QPushButton(tr("Скинути фільтри"), m_filtersPanel);

    m_authorFilterCombo->setEditable(true);
    m_genreFilterCombo->setEditable(true);
    m_authorFilterCombo->setInsertPolicy(QComboBox::NoInsert);
    m_genreFilterCombo->setInsertPolicy(QComboBox::NoInsert);

    if (auto *le = m_authorFilterCombo->lineEdit()) {
        le->setPlaceholderText(tr("Всі автори"));
        le->setClearButtonEnabled(true);
    }
    if (auto *le = m_genreFilterCombo->lineEdit()) {
        le->setPlaceholderText(tr("Всі жанри"));
        le->setClearButtonEnabled(true);
    }

    m_yearFilterEdit->setPlaceholderText(tr("Наприклад 1ххх або 2ххх"));
    m_yearFilterEdit->setClearButtonEnabled(true);
    m_yearFilterEdit->setMaxLength(4);
    m_yearFilterEdit->setValidator(
        new QRegularExpressionValidator(QRegularExpression("\\d{0,4}"),
                                        m_yearFilterEdit));

    QSet<QString> authors, genres;
    for (int r = 0; r < m_booksModel->rowCount(); ++r) {
        authors.insert(m_booksModel->data(
                                       m_booksModel->index(r, booksmodel::AuthorIdColumn)).toString());
        genres.insert(m_booksModel->data(
                                      m_booksModel->index(r, booksmodel::GenreColumn)).toString());
    }

    QStringList authorList = QStringList(authors.begin(), authors.end());
    authorList.sort(Qt::CaseInsensitive);
    QStringList genreList = QStringList(genres.begin(), genres.end());
    genreList.sort(Qt::CaseInsensitive);

    m_authorFilterCombo->addItem(tr(""), QVariant());
    for (const QString &a : authorList)
        m_authorFilterCombo->addItem(a, a);

    m_genreFilterCombo->addItem(tr(""), QVariant());
    for (const QString &g : genreList)
        m_genreFilterCombo->addItem(g, g);

    auto *authorModel = new QStringListModel(authorList, m_filtersPanel);
    auto *genreModel  = new QStringListModel(genreList,  m_filtersPanel);

    auto *authorCompleter = new QCompleter(authorModel, m_filtersPanel);
    authorCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    authorCompleter->setFilterMode(Qt::MatchContains);
    authorCompleter->setCompletionMode(QCompleter::PopupCompletion);

    auto *genreCompleter = new QCompleter(genreModel, m_filtersPanel);
    genreCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    genreCompleter->setFilterMode(Qt::MatchContains);
    genreCompleter->setCompletionMode(QCompleter::PopupCompletion);

    m_authorFilterCombo->setCompleter(authorCompleter);
    m_genreFilterCombo->setCompleter(genreCompleter);

    auto filtersLayout = new QVBoxLayout(m_filtersPanel);
    filtersLayout->setContentsMargins(12, 12, 12, 12);
    filtersLayout->setSpacing(8);
    filtersLayout->addWidget(new QLabel(tr("Автор:"), m_filtersPanel));
    filtersLayout->addWidget(m_authorFilterCombo);
    filtersLayout->addWidget(new QLabel(tr("Жанр:"), m_filtersPanel));
    filtersLayout->addWidget(m_genreFilterCombo);
    filtersLayout->addWidget(new QLabel(tr("Рік:"), m_filtersPanel));
    filtersLayout->addWidget(m_yearFilterEdit);
    filtersLayout->addSpacing(6);
    filtersLayout->addWidget(m_resetFiltersButton);
    filtersLayout->addStretch();

    auto buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(m_addBookButton);
    buttonsLayout->addWidget(m_editBookButton);
    buttonsLayout->addWidget(m_deleteBooksButton);
    buttonsLayout->addStretch();

    auto topLayout = new QHBoxLayout;
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(10);
    topLayout->addWidget(cabinetButton);
    topLayout->addWidget(m_searchEdit, 1);
    topLayout->addWidget(m_filtersButton);
    topLayout->addWidget(m_authButton);

    auto centerLayout = new QHBoxLayout;
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(12);
    centerLayout->addWidget(m_booksListView, 1);
    centerLayout->addWidget(m_filtersPanel, 0, Qt::AlignTop);

    auto layout = new QVBoxLayout(page);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(10);
    layout->addLayout(topLayout);
    layout->addLayout(centerLayout, 1);
    layout->addLayout(buttonsLayout);

    page->setLayout(layout);

    connect(m_addBookButton, &QPushButton::clicked, this, &MainWindow::onAddBookPageRequested);
    connect(m_editBookButton, &QPushButton::clicked, this, &MainWindow::onEditBookClicked);
    connect(cabinetButton, &QPushButton::clicked, this, &MainWindow::showUserCabinet);
    connect(m_authButton, &QPushButton::clicked, this, &MainWindow::showLoginDialog);
    connect(m_booksListView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onCurrentBookChanged);

    auto proxy = qobject_cast<BooksFilterProxyModel*>(m_booksProxyModel);
    if (proxy) {
        connect(m_searchEdit, &QLineEdit::textChanged, this, [proxy](const QString &text) {
            proxy->setSearchText(text);
        });

        connect(m_authorFilterCombo, &QComboBox::editTextChanged,
                this, [proxy](const QString &text) {
                    proxy->setAuthorFilter(text.trimmed());
                });

        connect(m_genreFilterCombo, &QComboBox::editTextChanged,
                this, [proxy](const QString &text) {
                    proxy->setGenreFilter(text.trimmed());
                });

        connect(m_yearFilterEdit, &QLineEdit::textChanged,
                this, [proxy](const QString &text) {
                    proxy->setYearFilter(text.trimmed());
                });

        connect(m_resetFiltersButton, &QPushButton::clicked, this, [this, proxy]() {
            m_authorFilterCombo->setCurrentIndex(0);
            m_genreFilterCombo->setCurrentIndex(0);

            if (auto *le = m_authorFilterCombo->lineEdit())
                le->clear();
            if (auto *le = m_genreFilterCombo->lineEdit())
                le->clear();
            m_yearFilterEdit->clear();

            proxy->setAuthorFilter(QString());
            proxy->setGenreFilter(QString());
            proxy->setYearFilter(QString());
        });
    }

    connect(m_filtersButton, &QToolButton::clicked, this, [this]() {
        if (!m_filtersPanel)
            return;
        m_filtersPanel->setVisible(!m_filtersPanel->isVisible());
    });

    return page;
}

void MainWindow::showLoginDialog()
{
    if (m_loginWidget) {
        m_loginWidget->clearFields();
        m_loginWidget->showLoginPage();
    }
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
        if (m_cabinetWidget)
            m_cabinetWidget->resetToHomePage();
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

    resetLibraryFilters();

    if (m_cabinetWidget)
        m_cabinetWidget->resetToHomePage();

    if (m_adminCabinet)
        m_adminCabinet->resetToHomePage();

    if(m_authButton){
        m_authButton -> setText("Увійти");
    }

    m_loginWidget->clearFields();
    m_loginWidget->showLoginPage();
    m_stack->setCurrentWidget(m_libraryWidget);

    m_addBookButton->setVisible(false);
    m_addBookButton->setEnabled(false);

    m_editBookButton->setVisible(false);
    m_editBookButton->setEnabled(false);

    m_deleteBooksButton->setVisible(false);
    m_deleteBooksButton->setEnabled(false);
}

void MainWindow::onAddBookPageRequested()
{
    m_isEditMode = false;
    m_editBookId = -1;

    m_titleEdit->clear();
    reloadAuthors();
    if (m_authorCombo->count() > 0)
        m_authorCombo->setCurrentIndex(0);
    m_genreEdit->clear();
    m_yearEdit->clear();
    m_descriptionEdit->clear();
    m_statusCombo->setCurrentIndex(0);


    m_currentCoverPath.clear();
    m_coverPreviewLabel->setPixmap(QPixmap());
    m_coverPreviewLabel->setText("No cover");

    m_saveBookButton->setText(tr("Зберегти книгу"));
    m_stack->setCurrentWidget(m_addBookPage);
}

void MainWindow::onSaveBookClicked()
{
    const QString title = m_titleEdit->text().trimmed();
    const int authorId = selectedAuthorId();
    const QString genre = m_genreEdit->text().trimmed();
    const int year = m_yearEdit->text().toInt();
    const QString description = m_descriptionEdit->toPlainText().trimmed();
    const QString status = m_statusCombo->currentText();

    if (title.isEmpty() || authorId <= 0 || genre.isEmpty() || year <= 0) {
        QMessageBox::warning(this, tr("Помилка"), tr("Заповніть усі поля та коректний рік."));
        return;
    }

    int row = -1;

    if (m_isEditMode) {
        for (int r = 0; r < m_booksModel->rowCount(); ++r) {
            if (m_booksModel->bookIdAtRow(r) == m_editBookId) {
                row = r;
                break;
            }
        }

        if (row == -1) {
            QMessageBox::warning(this, tr("Помилка"), tr("Не вдалося знайти книгу для редагування."));
            return;
        }
    } else {
        row = m_booksModel->rowCount();
        if (!m_booksModel->insertRow(row)) {
            QMessageBox::warning(this, tr("Помилка"), tr("Не вдалося вставити рядок у модель."));
            return;
        }
    }

    m_booksModel->setData(m_booksModel->index(row, booksmodel::TitleColumn), title);
    m_booksModel->setData(m_booksModel->index(row, booksmodel::AuthorIdColumn), authorId);
    m_booksModel->setData(m_booksModel->index(row, booksmodel::GenreColumn), genre);
    m_booksModel->setData(m_booksModel->index(row, booksmodel::YearColumn), year);
    m_booksModel->setData(m_booksModel->index(row, booksmodel::StatusColumn), status);
    m_booksModel->setData(m_booksModel->index(row, booksmodel::DescriptionColumn), description);
    m_booksModel->setData(m_booksModel->index(row, booksmodel::CoverPathColumn), m_currentCoverPath);

    if (!m_booksModel->submitAll()) {
        QMessageBox::warning(this, tr("Помилка"), m_booksModel->lastError().text());
        m_booksModel->revertAll();
        return;
    }

    QSqlQuery q(database::db());
    q.prepare("SELECT description, cover_path FROM Books WHERE title = :title");
    q.bindValue(":title", title);
    if (q.exec() && q.next()) {
        qDebug() << "DB description after save =" << q.value(0).toString();
        qDebug() << "DB cover_path after save =" << q.value(1).toString();
    }

    m_booksModel->select();
    m_booksModel->refreshAuthorCache();
    reloadAuthors();

    m_isEditMode = false;
    m_editBookId = -1;
    m_saveBookButton->setText(tr("Зберегти книгу"));
    showLibrary();
    m_booksListView->viewport()->update();
}

void MainWindow::onChooseCoverClicked()
{
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Виберіть обкладинку"),
                                                QString(),
                                                tr("Images (*.png *.jpg *.jpeg *.bmp)"));
    if (file.isEmpty())
        return;

    QPixmap pix(file);
    if (pix.isNull()) {
        QMessageBox::warning(this, tr("Помилка"), tr("Не вдалося відкрити зображення."));
        return;
    }

    const QString localCoverPath = copyCoverToLocalStorage(file);
    if (localCoverPath.isEmpty()) {
        QMessageBox::warning(this, tr("Помилка"), tr("Не вдалося скопіювати файл обкладинки."));
        return;
    }

    m_currentCoverPath = localCoverPath;

    m_coverPreviewLabel->setPixmap(
        pix.scaled(m_coverPreviewLabel->size(),
                   Qt::KeepAspectRatio,
                   Qt::SmoothTransformation));
    m_coverPreviewLabel->setText(QString());
}

void MainWindow::onCurrentBookChanged(const QModelIndex &current,
                                      const QModelIndex &previous)
{
    Q_UNUSED(previous);

    if (!current.isValid())
        return;

    auto *proxy = qobject_cast<BooksFilterProxyModel*>(m_booksProxyModel);
    if (!proxy)
        return;

    QModelIndex sourceIndex = proxy->mapToSource(current);
    if (!sourceIndex.isValid())
        return;

    QString title = m_booksModel->data(
                                    m_booksModel->index(sourceIndex.row(), booksmodel::TitleColumn)
                                    ).toString();
    qDebug() << "Selected book" << title;
}

void MainWindow::showBookDetails(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    auto *proxy = qobject_cast<QSortFilterProxyModel*>(m_booksProxyModel);
    if (!proxy) {
        QMessageBox::warning(this, tr("Помилка"), tr("Proxy model is not available."));
        return;
    }

    QModelIndex sourceIndex = proxy->mapToSource(index);
    if (!sourceIndex.isValid()) {
        QMessageBox::warning(this, tr("Помилка"), tr("Некоректний індекс книги."));
        return;
    }

    int row = sourceIndex.row();

    QString title = m_booksModel->data(m_booksModel->index(row, booksmodel::TitleColumn)).toString();
    QString author = m_booksModel->authorNameAtRow(row);
    QString genre = m_booksModel->data(m_booksModel->index(row, booksmodel::GenreColumn)).toString();
    QString year = m_booksModel->data(m_booksModel->index(row, booksmodel::YearColumn)).toString();
    QString status = m_booksModel->data(m_booksModel->index(row, booksmodel::StatusColumn)).toString();
    QString description = m_booksModel->data(m_booksModel->index(row, booksmodel::DescriptionColumn)).toString();
    QString cover = m_booksModel->data(m_booksModel->index(row, booksmodel::CoverPathColumn)).toString();

    m_detailsTitleLabel->setText(title);
    m_detailsAuthorLabel->setText(author);
    m_detailsGenreLabel->setText(genre);
    m_detailsYearLabel->setText(year);
    m_detailsStatusLabel->setText(status);
    m_detailsDescriptionEdit->setPlainText(
        description.isEmpty() ? tr("Опис відсутній.") : description
        );

    int readersCount = 0;
    {
        int bookId = m_booksModel->bookIdAtRow(row);

        QSqlQuery q(database::db());
        q.prepare(
            "SELECT COUNT(DISTINCT user_id) "
            "FROM Loans "
            "WHERE book_id = :b");
        q.bindValue(":b", bookId);
        if (q.exec() && q.next()) {
            readersCount = q.value(0).toInt();
        }
    }
    m_detailsReadersCountLabel->setText(
        tr("%1 користувач(ів)").arg(readersCount));

    const QString coverPath = resolveCoversPath(cover);
    QPixmap pix(coverPath);
    if (!pix.isNull()) {
        m_detailsCoverLabel->setPixmap(
            pix.scaled(m_detailsCoverLabel->size(),
                       Qt::KeepAspectRatio,
                       Qt::SmoothTransformation));
        m_detailsCoverLabel->setText(QString());
    } else {
        m_detailsCoverLabel->setPixmap(QPixmap());
        m_detailsCoverLabel->setText(tr("No cover"));
    }

    m_stack->setCurrentWidget(m_bookDetailsPage);
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

    if (m_currentUserId < 0) {
        QMessageBox::warning(this, tr("Помилка"), tr("Спочатку увійдіть у систему."));
        return;
    }

    auto proxy = qobject_cast<BooksFilterProxyModel*>(m_booksProxyModel);
    if (!proxy) {
        QMessageBox::warning(this, tr("Помилка"), tr("Proxy model is not available."));
        return;
    }

    QModelIndex sourceIndex = proxy->mapToSource(proxyIndex);
    if (!sourceIndex.isValid())
        return;

    int row = sourceIndex.row();
    int bookId = m_booksModel->bookIdAtRow(row);
    QString title = m_booksModel->data(m_booksModel->index(row, booksmodel::TitleColumn)).toString();
    QString status = m_booksModel->data(m_booksModel->index(row, booksmodel::StatusColumn)).toString();

    if (status == "loaned") {
        QMessageBox::information(this, tr("Недоступно"), tr("Ця книга вже видана."));
        return;
    }

    const int maxActiveLoans = 5;

    QSqlQuery q(database::db());
    q.prepare("SELECT COUNT(*) FROM Loans WHERE user_id = :u AND status = 'active'");
    q.bindValue(":u", m_currentUserId);

    if (!q.exec() || !q.next()) {
        QMessageBox::warning(this, tr("Помилка"), tr("Не вдалося перевірити кількість активних книг."));
        return;
    }

    int activeCount = q.value(0).toInt();
    if (activeCount >= maxActiveLoans) {
        QMessageBox::information(this,
                                 tr("Ліміт"),
                                 tr("Ви вже взяли %1 книг.").arg(maxActiveLoans));
        return;
    }

    QSqlQuery q2(database::db());
    q2.prepare("SELECT COUNT(*) FROM Loans "
               "WHERE user_id = :u AND book_id = :b AND status IN ('active', 'overdue')");
    q2.bindValue(":u", m_currentUserId);
    q2.bindValue(":b", bookId);

    if (q2.exec() && q2.next() && q2.value(0).toInt() > 0) {
        QMessageBox::information(this,
                                 tr("Уже видано"),
                                 tr("У вас уже є ця книга на руках."));
        return;
    }

    if (QMessageBox::question(this,
                              tr("Підтвердження"),
                              tr("Взяти книгу \"%1\"?").arg(title))
        != QMessageBox::Yes) {
        return;
    }

    bool ok = database::takeBook(m_currentUserId, bookId);
    if (!ok) {
        QMessageBox::warning(this, tr("Помилка"), tr("Не вдалося видати книгу."));
        return;
    }

    QMessageBox::information(this,
                             tr("Успіх"),
                             tr("Книгу \"%1\" успішно видано.\nПовернути до: %2")
                                 .arg(title, QDate::currentDate().addDays(14).toString("dd.MM.yyyy")));

    m_booksModel->select();
    m_booksListView->viewport()->update();
    refreshLoans();
    refreshReaderStats();
    refreshCurrentUserChart();
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

    m_loansModel->setHeaderData(1, Qt::Horizontal, tr("Назва"));
    m_loansModel->setHeaderData(3, Qt::Horizontal, tr("Дата видачі"));
    m_loansModel->setHeaderData(4, Qt::Horizontal, tr("Повернути до"));
    m_loansModel->setHeaderData(5, Qt::Horizontal, tr("Дата повернення"));
    m_loansModel->setHeaderData(6, Qt::Horizontal, tr("Статус"));

    m_cabinetWidget->setLoansModel(m_loansModel);

}

void MainWindow::refreshReaderStats()
{
    if (m_currentUserId <= 0 || !m_cabinetWidget)
        return;

    int total = 0;
    int active = 0;
    int returned = 0;
    int overdue = 0;

    {
        QSqlQuery q(database::db());

        if (q.exec(QStringLiteral("SELECT COUNT(*) FROM Loans WHERE user_id = %1")
                       .arg(m_currentUserId)) && q.next()) {
            total = q.value(0).toInt();
        }

        if (q.exec(QStringLiteral("SELECT COUNT(*) FROM Loans "
                                  "WHERE user_id = %1 AND status = 'active'")
                       .arg(m_currentUserId)) && q.next()) {
            active = q.value(0).toInt();
        }

        if (q.exec(QStringLiteral("SELECT COUNT(*) FROM Loans "
                                  "WHERE user_id = %1 AND status = 'returned'")
                       .arg(m_currentUserId)) && q.next()) {
            returned = q.value(0).toInt();
        }

        if (q.exec(QStringLiteral("SELECT COUNT(*) FROM Loans "
                                  "WHERE user_id = %1 AND status = 'overdue'")
                       .arg(m_currentUserId)) && q.next()) {
            overdue = q.value(0).toInt();
        }
    }

    QString topGenres;
    {
        QSqlQuery q(database::db());
        q.prepare(
            "SELECT Books.genre, COUNT(*) AS cnt "
            "FROM Loans "
            "JOIN Books ON Loans.book_id = Books.id "
            "WHERE Loans.user_id = :u "
            "GROUP BY Books.genre "
            "ORDER BY cnt DESC "
            "LIMIT 3");
        q.bindValue(":u", m_currentUserId);

        if (q.exec()) {
            QStringList items;
            while (q.next()) {
                const QString genre = q.value(0).toString();
                const int cnt = q.value(1).toInt();
                items << QString("%1 (%2)").arg(genre).arg(cnt);
            }
            topGenres = items.isEmpty()
                            ? tr("Немає даних")
                            : items.join(", ");
        } else {
            topGenres = tr("Помилка запиту");
        }
    }

    QString topAuthors;
    {
        QSqlQuery q(database::db());
        q.prepare(
            "SELECT Authors.name, COUNT(*) AS cnt "
            "FROM Loans "
            "JOIN Books ON Loans.book_id = Books.id "
            "JOIN Authors ON Books.author_id = Authors.id "
            "WHERE Loans.user_id = :uid "
            "GROUP BY Authors.id, Authors.name "
            "ORDER BY cnt DESC, Authors.name ASC "
            "LIMIT 3");
        q.bindValue(":uid", m_currentUserId);

        if (q.exec()) {
            QStringList items;
            while (q.next()) {
                const QString author = q.value(0).toString();
                const int cnt = q.value(1).toInt();
                items << QString("%1 (%2)").arg(author).arg(cnt);
            }
            topAuthors = items.isEmpty()
                             ? tr("Немає даних")
                             : items.join(", ");
        } else {
            topAuthors = tr("Помилка запиту");
        }
    }

    m_cabinetWidget->setReaderStats(
        total,
        active,
        returned,
        overdue,
        topGenres,
        topAuthors);
}

void MainWindow::showAdminCabinet()
{
    if (m_currentUserRole != "admin") {
        QMessageBox::warning(this, tr("Доступ заборонено"),
                             tr("Ця сторінка доступна лише адміністратору"));
        return;
    }

    if (m_adminCabinet)
        m_adminCabinet->resetToHomePage();
    refreshAllOverdues();

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
    if (m_badUsersModel) {
        QString badSql =
            "SELECT Users.full_name AS 'ПІБ', "
            "       Users.email AS 'E-mail', "
            "       COUNT(*) AS 'К-сть боргів', "
            "       MIN(Loans.due_date) AS 'Найдавніший борг' "
            "FROM Loans "
            "JOIN Users ON Loans.user_id = Users.id "
            "WHERE Loans.status IN ('active', 'overdue') "
            "  AND Loans.due_date IS NOT NULL "
            "  AND Loans.due_date < DATE('now') "
            "GROUP BY Users.id "
            "ORDER BY COUNT(*) DESC, MIN(Loans.due_date) ASC";

        m_badUsersModel->setQuery(badSql, database::db());
        if (m_badUsersModel->lastError().isValid()) {
            qDebug() << "bad users query error:" << m_badUsersModel->lastError().text();
        }
        qDebug() << "bad users rows =" << m_badUsersModel->rowCount();
        m_adminCabinet->setBadUsersModel(m_badUsersModel);
    }

    QString goodUsersSql =
        "SELECT Users.full_name AS 'ПІБ', "
        "       Users.email AS 'E-mail', "
        "       SUM(CASE WHEN Loans.status = 'returned' THEN 1 ELSE 0 END) AS 'Повернено', "
        "       SUM(CASE WHEN Loans.status = 'overdue' THEN 1 ELSE 0 END) AS 'Прострочено' "
        "FROM Users "
        "LEFT JOIN Loans ON Loans.user_id = Users.id "
        "WHERE Users.role = 'user' "
        "GROUP BY Users.id "
        "ORDER BY "
        "       SUM(CASE WHEN Loans.status = 'overdue' THEN 1 ELSE 0 END) ASC, "
        "       SUM(CASE WHEN Loans.status = 'returned' THEN 1 ELSE 0 END) DESC, "
        "       Users.full_name ASC "
        "LIMIT 10";

    m_goodUsersModel->setQuery(goodUsersSql, database::db());

    if (m_goodUsersModel->lastError().isValid()) {
        qDebug() << "good users query error:" << m_goodUsersModel->lastError().text();
    }
    qDebug() << "good users rows =" << m_goodUsersModel->rowCount();

    m_adminCabinet->setGoodUsersModel(m_goodUsersModel);

    refreshAdminStats();
    onChartTypeChanged(m_adminCabinet->currentChartIndex());
    refreshPeakStats();
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
                             tr("Не вдалося оновити видачу"));
        return;
    }

    QSqlQuery qb(database::db());
    qb.prepare("UPDATE Books SET status = 'available' WHERE id = :id");
    qb.bindValue(":id", bookId);
    if (!qb.exec()) {
        QMessageBox::warning(this, tr("Увага"),
                             tr("Видачу оновлено, але статус книги не змінено"));
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

void MainWindow::onEditBookClicked()
{
    if (m_currentUserRole != "admin") {
        QMessageBox::warning(this,
                             tr("Доступ заборонено"),
                             tr("Редагувати книги може лише адміністратор."));
        return;
    }

    QModelIndex proxyIndex = m_booksListView->currentIndex();
    if (!proxyIndex.isValid()) {
        QMessageBox::information(this,
                                 tr("Немає вибору"),
                                 tr("Оберіть книгу для редагування."));
        return;
    }

    openBookFormForEdit(proxyIndex);
}

void MainWindow::openBookFormForEdit(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    auto *proxy = qobject_cast<BooksFilterProxyModel*>(m_booksProxyModel);
    if (!proxy) {
        QMessageBox::warning(this, tr("Помилка"), tr("Proxy model is not available."));
        return;
    }

    QModelIndex sourceIndex = proxy->mapToSource(index);
    if (!sourceIndex.isValid()) {
        QMessageBox::warning(this, tr("Помилка"), tr("Некоректний індекс книги."));
        return;
    }

    int row = sourceIndex.row();

    m_isEditMode = true;
    m_editBookId = m_booksModel->bookIdAtRow(row);

    QSqlRecord rec = m_booksModel->record(row);

    const QString title       = rec.value("title").toString();
    const int     authorId    = rec.value("author_id").toInt();        // ← нове
    const QString genre       = rec.value("genre").toString();
    const QString year        = rec.value("year").toString();
    const QString status      = rec.value("status").toString();
    const QString description = rec.value("description").toString();
    const QString coverPath   = rec.value("cover_path").toString();

    m_titleEdit->setText(title);
    m_genreEdit->setText(genre);
    m_yearEdit->setText(year);
    m_descriptionEdit->setPlainText(description);
    m_currentCoverPath = coverPath;

    if (m_authorCombo) {
        int idx = -1;
        for (int i = 0; i < m_authorCombo->count(); ++i) {
            if (m_authorCombo->itemData(i).toInt() == authorId) {
                idx = i;
                break;
            }
        }
        if (idx >= 0)
            m_authorCombo->setCurrentIndex(idx);
    }

    int statusIndex = m_statusCombo->findText(status);
    m_statusCombo->setCurrentIndex(statusIndex >= 0 ? statusIndex : 0);

    const QString absCoverPath = resolveCoversPath(coverPath);
    QPixmap pix(absCoverPath);
    if (!pix.isNull()) {
        m_coverPreviewLabel->setPixmap(
            pix.scaled(m_coverPreviewLabel->size(),
                       Qt::KeepAspectRatio,
                       Qt::SmoothTransformation));
        m_coverPreviewLabel->setText(QString());
    } else {
        m_coverPreviewLabel->setPixmap(QPixmap());
        m_coverPreviewLabel->setText(tr("No cover"));
    }

    m_saveBookButton->setText(tr("Зберегти зміни"));
    m_stack->setCurrentWidget(m_addBookPage);
}

void MainWindow::onBookDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (m_currentUserRole == "admin") {
        openBookFormForEdit(index);
    } else {
        onBookInfoRequested(index);
    }
}

void MainWindow::refreshAllOverdues()
{
    QSqlQuery q(database::db());

    const QString sql =
        "UPDATE Loans "
        "SET status = 'overdue' "
        "WHERE status = 'active' "
        "AND due_date IS NOT NULL "
        "AND due_date < DATE('now')";

    if (!q.exec(sql)) {
        qDebug() << "refreshAllOverdues failed:" << q.lastError().text();
        qDebug() << "SQL:" << sql;
        return;
    }

    qDebug() << "refreshAllOverdues ok, rows affected =" << q.numRowsAffected();
}

void MainWindow::refreshPeakStats()
{
    if (!m_adminCabinet)
        return;

    QString peakDay = tr("-");
    QString peakMonth = tr("-");
    QString topGenre = tr("-");

    QString topBookTitle = tr("Назва книги");
    QString topBookMeta = tr("Рік видання");
    QPixmap topBookCover;
    int topBookLoans = 0;

    {
        QSqlQuery q(database::db());
        const QString sql =
            "SELECT issue_date, COUNT(*) AS cnt "
            "FROM Loans "
            "GROUP BY issue_date "
            "ORDER BY cnt DESC, issue_date ASC "
            "LIMIT 1";
        if (q.exec(sql) && q.next()) {
            peakDay = QString("%1 (%2)")
            .arg(q.value(0).toString())
                .arg(q.value(1).toInt());
        }
    }

    {
        QSqlQuery q(database::db());
        const QString sql =
            "SELECT substr(issue_date, 1, 7) AS month, COUNT(*) AS cnt "
            "FROM Loans "
            "GROUP BY substr(issue_date, 1, 7) "
            "ORDER BY cnt DESC, month ASC "
            "LIMIT 1";
        if (q.exec(sql) && q.next()) {
            peakMonth = QString("%1 (%2)")
            .arg(q.value(0).toString())
                .arg(q.value(1).toInt());
        }
    }

    {
        QSqlQuery q(database::db());
        const QString sql =
            "SELECT Books.genre, COUNT(*) AS cnt "
            "FROM Loans "
            "JOIN Books ON Loans.book_id = Books.id "
            "GROUP BY Books.genre "
            "ORDER BY cnt DESC, Books.genre ASC "
            "LIMIT 1";
        if (q.exec(sql) && q.next()) {
            topGenre = QString("%1 (%2)")
            .arg(q.value(0).toString())
                .arg(q.value(1).toInt());
        }
    }

    {
        QSqlQuery q(database::db());
        const QString sql =
            "SELECT Books.title, Books.genre, Books.year, Books.cover_path, COUNT(*) AS cnt "
            "FROM Loans "
            "JOIN Books ON Loans.book_id = Books.id "
            "GROUP BY Books.id, Books.title, Books.genre, Books.year, Books.cover_path "
            "ORDER BY cnt DESC, Books.title ASC "
            "LIMIT 1";

        if (q.exec(sql) && q.next()) {
            topBookTitle = q.value(0).toString();
            const QString genre = q.value(1).toString();
            const QString year = q.value(2).toString();
            const QString coverPath = q.value(3).toString();
            topBookLoans = q.value(4).toInt();

            qDebug() << "[TOP BOOK] title =" << topBookTitle;
            qDebug() << "[TOP BOOK] genre =" << genre;
            qDebug() << "[TOP BOOK] year =" << year;
            qDebug() << "[TOP BOOK] coverPath =" << coverPath;
            qDebug() << "[TOP BOOK] loans =" << topBookLoans;

            QFileInfo fi(coverPath);
            qDebug() << "[TOP BOOK] exists =" << fi.exists();
            qDebug() << "[TOP BOOK] abs path =" << fi.absoluteFilePath();

            if (!genre.trimmed().isEmpty() && !year.trimmed().isEmpty())
                topBookMeta = QString("%1 • %2").arg(genre, year);
            else if (!year.trimmed().isEmpty())
                topBookMeta = year;
            else if (!genre.trimmed().isEmpty())
                topBookMeta = genre;
            else
                topBookMeta = tr("Без додаткових даних");

            if (!coverPath.isEmpty()) {
                const QString absCoverPath = resolveCoversPath(coverPath);
                QPixmap pix(absCoverPath);
                qDebug() << "[TOP BOOK] pix is null =" << pix.isNull();
                qDebug() << "[TOP BOOK] pix size =" << pix.size();

                if (!pix.isNull())
                    topBookCover = pix;
            } else {
                qDebug() << "[TOP BOOK] coverPath is empty";
            }
        } else {
            qDebug() << "[TOP BOOK] query failed or no rows";
            qDebug() << "[TOP BOOK] sql error =" << q.lastError().text();
        }
    }

    m_adminCabinet->setPeakStats(peakDay, peakMonth, topGenre);
    m_adminCabinet->setTopBookCard(topBookTitle, topBookMeta, topBookCover, topBookLoans);
}

void MainWindow::refreshLoansChart()
{
    if (!m_adminCabinet)
        return;

    QStringList categories;
    QList<int> values;

    QSqlQuery q(database::db());
    QString sql =
        "SELECT substr(issue_date, 1, 7) AS month, COUNT(*) AS cnt "
        "FROM Loans "
        "GROUP BY substr(issue_date, 1, 7) "
        "ORDER BY month ASC";

    if (!q.exec(sql)) {
        qDebug() << "refreshLoansChart: query failed exec:" << q.lastError().text();
        return;
    }

    while (q.next()) {
        categories << q.value(0).toString();
        values << q.value(1).toInt();
    }

    qDebug() << "refreshLoansChart: months =" << categories
             << "counts =" << values;

    QChart *chart = new QChart();
    chart->setTitle(tr("Кількість видач по місяцях"));

    if (categories.isEmpty()) {
        m_adminCabinet->setLoansChart(chart);
        return;
    }

    QBarSet *set = new QBarSet(tr("Видачі"));
    for (int v : values)
        *set << v;

    QBarSeries *series = new QBarSeries();
    series->append(set);

    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText(tr("Кількість"));
    axisY->setLabelFormat("%d");
    axisY->applyNiceNumbers();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    m_adminCabinet->setLoansChart(chart);
}

void MainWindow::onChartTypeChanged(int index)
{
    if (index == 0)
        refreshLoansChart();
    else if (index == 1)
        refreshGenreChart();
    else if (index == 2)
        refreshStatusChart();
    else if (index == 3)
        refreshTopBooksChart();
}


void MainWindow::refreshGenreChart()
{
    if (!m_adminCabinet)
        return;

    QStringList categories;
    QList<int> values;

    QSqlQuery q(database::db());
    const QString sql =
        "SELECT genre, COUNT(*) AS cnt "
        "FROM Books "
        "GROUP BY genre "
        "ORDER BY cnt DESC, genre ASC";

    if (!q.exec(sql)) {
        qDebug() << "refreshGenreChart failed:" << q.lastError().text();
        return;
    }

    while (q.next()) {
        const QString genre = q.value(0).toString().trimmed().isEmpty()
        ? tr("Без жанру")
        : q.value(0).toString();
        categories << genre;
        values << q.value(1).toInt();
    }

    QChart *chart = new QChart();
    chart->setTitle(tr("Книги за жанрами"));

    if (categories.isEmpty()) {
        m_adminCabinet->setLoansChart(chart);
        return;
    }

    QBarSet *set = new QBarSet(tr("Жанри"));
    for (int v : values)
        *set << v;

    QBarSeries *series = new QBarSeries();
    series->append(set);
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsAngle(-35);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText(tr("Кількість"));
    axisY->setLabelFormat("%d");
    axisY->applyNiceNumbers();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(false);

    m_adminCabinet->setLoansChart(chart);
}

void MainWindow::refreshCurrentUserChart()
{
    if (!m_cabinetWidget)
        return;

    int idx = m_cabinetWidget->currentChartIndex();
    if (idx == 0)
        refreshUserGenreChart();
    else
        refreshUserAuthorChart();
}

void MainWindow::onUserChartTypeChanged(int index)
{
    Q_UNUSED(index);
    refreshCurrentUserChart();
}

void MainWindow::refreshUserGenreChart()
{
    if (!m_cabinetWidget || m_currentUserId <= 0)
        return;

    QStringList categories;
    QList<int> values;

    QSqlQuery q(database::db());
    q.prepare(
        "SELECT Books.genre, COUNT(*) AS cnt "
        "FROM Loans "
        "JOIN Books ON Loans.book_id = Books.id "
        "WHERE Loans.user_id = :uid "
        "GROUP BY Books.genre "
        "ORDER BY cnt DESC, Books.genre ASC"
        );
    q.bindValue(":uid", m_currentUserId);

    if (!q.exec()) {
        qDebug() << "refreshUserGenreChart failed:" << q.lastError().text();
        return;
    }

    while (q.next()) {
        categories << q.value(0).toString();
        values << q.value(1).toInt();
    }

    QChart *chart = new QChart();
    chart->setTitle(tr("Мої книги за жанрами"));

    if (categories.isEmpty()) {
        m_cabinetWidget->setUserChart(chart);
        return;
    }

    QBarSet *set = new QBarSet(tr("Жанри"));
    for (int v : values)
        *set << v;

    QBarSeries *series = new QBarSeries();
    series->append(set);
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsAngle(-35);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText(tr("Кількість"));
    axisY->setLabelFormat("%d");
    axisY->applyNiceNumbers();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(false);

    m_cabinetWidget->setUserChart(chart);
}

void MainWindow::refreshUserAuthorChart()
{
    if (!m_cabinetWidget || m_currentUserId <= 0)
        return;

    QStringList categories;
    QList<int> values;

    QSqlQuery q(database::db());
    q.prepare(
        "SELECT Authors.name, COUNT(*) AS cnt "
        "FROM Loans "
        "JOIN Books ON Loans.book_id = Books.id "
        "JOIN Authors ON Books.author_id = Authors.id "
        "WHERE Loans.user_id = :uid "
        "GROUP BY Authors.id, Authors.name "
        "ORDER BY cnt DESC, Authors.name ASC"
        );
    q.bindValue(":uid", m_currentUserId);

    if (!q.exec()) {
        qDebug() << "refreshUserAuthorChart failed:" << q.lastError().text();
        return;
    }

    while (q.next()) {
        categories << q.value(0).toString();
        values << q.value(1).toInt();
    }

    QChart *chart = new QChart();
    chart->setTitle(tr("Мої книги за авторами"));

    if (categories.isEmpty()) {
        m_cabinetWidget->setUserChart(chart);
        return;
    }

    QBarSet *set = new QBarSet(tr("Автори"));
    for (int v : values)
        *set << v;

    QBarSeries *series = new QBarSeries();
    series->append(set);
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsAngle(-35);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText(tr("Кількість"));
    axisY->setLabelFormat("%d");
    axisY->applyNiceNumbers();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(false);

    m_cabinetWidget->setUserChart(chart);
}

void MainWindow::onExportAdminReportRequested()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Зберегти звіт у PDF"),
        QString("library_report_%1.pdf")
            .arg(QDate::currentDate().toString("yyyy-MM-dd")),
        tr("PDF files (*.pdf)")
        );

    if (fileName.isEmpty())
        return;

    auto getCount = [](const QString &sql) -> int {
        QSqlQuery q(database::db());
        if (!q.exec(sql) || !q.next())
            return 0;
        return q.value(0).toInt();
    };

    const int totalBooks     = getCount("SELECT COUNT(*) FROM Books");
    const int availableBooks = getCount("SELECT COUNT(*) FROM Books WHERE status = 'available'");
    const int loanedBooks    = getCount("SELECT COUNT(*) FROM Books WHERE status = 'loaned'");
    const int activeLoans    = getCount("SELECT COUNT(*) FROM Loans WHERE status = 'active'");
    const int overdueLoans   = getCount("SELECT COUNT(*) FROM Loans WHERE status = 'overdue'");
    const int totalUsers     = getCount("SELECT COUNT(*) FROM Users WHERE role = 'user'");

    QString peakDay("-");
    QString peakMonth("-");
    QString topBook("-");
    QString topGenre("-");

    {
        QSqlQuery q(database::db());
        if (q.exec(
                "SELECT issue_date, COUNT(*) AS cnt "
                "FROM Loans "
                "GROUP BY issue_date "
                "ORDER BY cnt DESC "
                "LIMIT 1") && q.next()) {
            peakDay = q.value(0).toString();
        }
    }

    {
        QSqlQuery q(database::db());
        if (q.exec(
                "SELECT substr(issue_date,1,7) AS month, COUNT(*) AS cnt "
                "FROM Loans "
                "GROUP BY month "
                "ORDER BY cnt DESC "
                "LIMIT 1") && q.next()) {
            peakMonth = q.value(0).toString();
        }
    }

    {
        QSqlQuery q(database::db());
        if (q.exec(
                "SELECT Books.title, COUNT(*) AS cnt "
                "FROM Loans "
                "JOIN Books ON Loans.book_id = Books.id "
                "GROUP BY Books.title "
                "ORDER BY cnt DESC "
                "LIMIT 1") && q.next()) {
            topBook = q.value(0).toString();
        }
    }

    {
        QSqlQuery q(database::db());
        if (q.exec(
                "SELECT Books.genre, COUNT(*) AS cnt "
                "FROM Loans "
                "JOIN Books ON Loans.book_id = Books.id "
                "GROUP BY Books.genre "
                "ORDER BY cnt DESC "
                "LIMIT 1") && q.next()) {
            topGenre = q.value(0).toString();
        }
    }

    QString html;
    QTextStream s(&html);
    s.setEncoding(QStringConverter::Utf8);

    s << "<html><head><meta charset='utf-8'>"
      << "<style>"
      << "body{font-family:'DejaVu Sans',Arial,sans-serif;"
      << "     font-size:10pt;color:#222;margin:40px;}"
      << "h1{font-size:20pt;margin-bottom:8px;}"
      << "p.meta{font-size:9pt;color:#555;margin-top:0;margin-bottom:20px;}"
      << ".section{margin-bottom:20px;}"
      << ".section-title{font-weight:bold;font-size:12pt;margin-bottom:4px;}"
      << "table{border-collapse:collapse;width:80%;margin-top:4px;}"
      << "th,td{border:1px solid #444;padding:4px 6px;font-size:9pt;}"
      << "th{background:#f0f0f0;font-weight:bold;text-align:left;}"
      << "tbody tr:nth-child(odd){background:#fafafa;}"
      << "table.loans{width:100%;}"
      << "table.loans th, table.loans td{font-size:8.5pt;}"
      << "</style></head><body>";

    s << "<h1>Звіт бібліотеки</h1>";
    s << "<p class='meta'>"
      << tr("Дата створення: ")
      << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
      << "</p>";

    s << "<div class='section'>";
    s << "<div class='section-title'>" << tr("Загальна статистика") << "</div>";
    s << "<table><thead><tr>"
      << "<th>" << tr("Показник") << "</th>"
      << "<th>" << tr("Значення") << "</th>"
      << "</tr></thead><tbody>";
    s << "<tr><td>" << tr("Усього книг") << "</td><td>" << totalBooks << "</td></tr>";
    s << "<tr><td>" << tr("Доступні") << "</td><td>" << availableBooks << "</td></tr>";
    s << "<tr><td>" << tr("Видані") << "</td><td>" << loanedBooks << "</td></tr>";
    s << "<tr><td>" << tr("Активні видачі") << "</td><td>" << activeLoans << "</td></tr>";
    s << "<tr><td>" << tr("Прострочені") << "</td><td>" << overdueLoans << "</td></tr>";
    s << "<tr><td>" << tr("Користувачі") << "</td><td>" << totalUsers << "</td></tr>";
    s << "</tbody></table></div>";

    s << "<div class='section'>";
    s << "<div class='section-title'>" << tr("Пікові показники") << "</div>";
    s << "<table><thead><tr>"
      << "<th>" << tr("Показник") << "</th>"
      << "<th>" << tr("Значення") << "</th>"
      << "</tr></thead><tbody>";
    s << "<tr><td>" << tr("Піковий день") << "</td><td>" << peakDay.toHtmlEscaped() << "</td></tr>";
    s << "<tr><td>" << tr("Піковий місяць") << "</td><td>" << peakMonth.toHtmlEscaped() << "</td></tr>";
    s << "<tr><td>" << tr("Найпопулярніша книга") << "</td><td>" << topBook.toHtmlEscaped() << "</td></tr>";
    s << "<tr><td>" << tr("Найпопулярніший жанр") << "</td><td>" << topGenre.toHtmlEscaped() << "</td></tr>";
    s << "</tbody></table></div>";

    s << "<div class='section'>";
    s << "<div class='section-title'>" << tr("Усі видачі") << "</div>";
    s << "<table class='loans'><thead><tr>"
      << "<th>" << tr("ID") << "</th>"
      << "<th>" << tr("Користувач") << "</th>"
      << "<th>" << tr("Книга") << "</th>"
      << "<th>" << tr("Дата видачі") << "</th>"
      << "<th>" << tr("Повернути до") << "</th>"
      << "<th>" << tr("Дата повернення") << "</th>"
      << "<th>" << tr("Статус") << "</th>"
      << "</tr></thead><tbody>";

    QSqlQuery q(database::db());
    if (!q.exec(
            "SELECT Loans.id, Users.full_name, Books.title, "
            "       Loans.issue_date, Loans.due_date, "
            "       Loans.return_date, Loans.status "
            "FROM Loans "
            "JOIN Users ON Loans.user_id = Users.id "
            "JOIN Books ON Loans.book_id = Books.id "
            "ORDER BY Loans.id ASC")) {
        qDebug() << "export report loans query failed:" << q.lastError().text();
    } else {
        while (q.next()) {
            s << "<tr>";
            for (int i = 0; i < 7; ++i) {
                s << "<td>" << q.value(i).toString().toHtmlEscaped() << "</td>";
            }
            s << "</tr>";
        }
    }
    s << "</tbody></table></div>";

    s << "</body></html>";

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(15, 15, 15, 15),
                           QPageLayout::Millimeter);

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, tr("Готово"),
                             tr("PDF-звіт успішно збережено."));
}

void MainWindow::showFiltersPopup()
{
}

void MainWindow::refreshStatusChart()
{
    if (!m_adminCabinet)
        return;

    QStringList categories;
    QList<int> values;

    QSqlQuery q(database::db());
    const QString sql =
        "SELECT status, COUNT(*) AS cnt "
        "FROM Books "
        "GROUP BY status "
        "ORDER BY cnt DESC, status ASC";

    if (!q.exec(sql)) {
        qDebug() << "refreshStatusChart failed:" << q.lastError().text();
        return;
    }

    while (q.next()) {
        QString status = q.value(0).toString().trimmed();
        if (status == "available")
            status = tr("Доступні");
        else if (status == "loaned")
            status = tr("Видані");
        else if (status.isEmpty())
            status = tr("Невідомо");

        categories << status;
        values << q.value(1).toInt();
    }

    QChart *chart = new QChart();
    chart->setTitle(tr("Книги за статусом"));

    if (categories.isEmpty()) {
        m_adminCabinet->setLoansChart(chart);
        return;
    }

    QBarSet *set = new QBarSet(tr("Статуси"));
    for (int v : values)
        *set << v;

    QBarSeries *series = new QBarSeries();
    series->append(set);
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText(tr("Кількість"));
    axisY->setLabelFormat("%d");
    axisY->applyNiceNumbers();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(false);

    m_adminCabinet->setLoansChart(chart);
}

void MainWindow::refreshTopBooksChart()
{
    if (!m_adminCabinet)
        return;

    QStringList categories;
    QList<int> values;

    QSqlQuery q(database::db());
    const QString sql =
        "SELECT Books.title, COUNT(*) AS cnt "
        "FROM Loans "
        "JOIN Books ON Loans.book_id = Books.id "
        "GROUP BY Books.id, Books.title "
        "ORDER BY cnt DESC, Books.title ASC "
        "LIMIT 5";

    if (!q.exec(sql)) {
        qDebug() << "refreshTopBooksChart failed:" << q.lastError().text();
        return;
    }

    while (q.next()) {
        categories << q.value(0).toString();
        values << q.value(1).toInt();
    }

    QChart *chart = new QChart();
    chart->setTitle(tr("Топ-5 книг"));

    if (categories.isEmpty()) {
        m_adminCabinet->setLoansChart(chart);
        return;
    }

    QBarSet *set = new QBarSet(tr("Видачі"));
    for (int v : values)
        *set << v;

    QBarSeries *series = new QBarSeries();
    series->append(set);
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsAngle(-35);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText(tr("Кількість видач"));
    axisY->setLabelFormat("%d");
    axisY->applyNiceNumbers();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(false);

    m_adminCabinet->setLoansChart(chart);
}

void MainWindow::reloadAuthors()
{
    if (!m_authorCombo)
        return;

    const QVariant currentId = m_authorCombo->currentData();

    m_authorCombo->clear();
    QSqlQuery q(database::db());
    if (!q.exec("SELECT id, name FROM Authors ORDER BY name COLLATE NOCASE ASC"))
        return;

    while (q.next()) {
        m_authorCombo->addItem(q.value(1).toString(), q.value(0));
    }

    int idx = m_authorCombo->findData(currentId);
    if (idx >= 0)
        m_authorCombo->setCurrentIndex(idx);
}

int MainWindow::selectedAuthorId() const
{
    if (!m_authorCombo || m_authorCombo->currentIndex() < 0)
        return -1;

    return m_authorCombo->currentData().toInt();
}

void MainWindow::selectAuthorById(int authorId)
{
    if (!m_authorCombo)
        return;

    const int index = m_authorCombo->findData(authorId);
    if (index >= 0)
        m_authorCombo->setCurrentIndex(index);
}

void MainWindow::onAddAuthorClicked()
{
    bool ok = false;
    const QString name = QInputDialog::getText(this,
                                               tr("Новий автор"),
                                               tr("Ім'я автора:"),
                                               QLineEdit::Normal,
                                               QString(),
                                               &ok).trimmed();

    if (!ok || name.isEmpty())
        return;

    const int authorId = database::ensureAuthorExists(name);
    if (authorId <= 0) {
        QMessageBox::warning(this, tr("Помилка"), tr("Не вдалося додати автора."));
        return;
    }

    reloadAuthors();
    selectAuthorById(authorId);
}

void MainWindow::resetLibraryFilters()
{
    if (m_searchEdit)
        m_searchEdit->clear();

    if (m_authorFilterCombo) {
        m_authorFilterCombo->setCurrentIndex(0);
        if (auto *le = m_authorFilterCombo->lineEdit())
            le->clear();
    }

    if (m_genreFilterCombo) {
        m_genreFilterCombo->setCurrentIndex(0);
        if (auto *le = m_genreFilterCombo->lineEdit())
            le->clear();
    }

    if (m_yearFilterEdit)
        m_yearFilterEdit->clear();

    if (m_filtersPanel)
        m_filtersPanel->setVisible(false);

    if (m_booksListView) {
        m_booksListView->clearSelection();
        m_booksListView->setCurrentIndex(QModelIndex());
    }

    auto *proxy = qobject_cast<BooksFilterProxyModel*>(m_booksProxyModel);
    if (proxy) {
        proxy->setSearchText(QString());
        proxy->setAuthorFilter(QString());
        proxy->setGenreFilter(QString());
        proxy->setYearFilter(QString());
        proxy->invalidate();
    }
}

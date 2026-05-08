#include "usercabinet.h"
#include "database.h"

#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QComboBox>
#include <QTabWidget>

static QFrame *createKpiCard(const QString &title, QLabel *valueLabel, QWidget *parent = nullptr)
{
    auto *card = new QFrame(parent);
    card->setObjectName("kpiCard");

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(6);

    auto *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName("kpiTitle");

    valueLabel->setParent(card);
    valueLabel->setObjectName("kpiValue");
    valueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);

    return card;
}

static QFrame *createInfoCard(const QString &title, QLabel *valueLabel, QWidget *parent = nullptr)
{
    auto *card = new QFrame(parent);
    card->setObjectName("infoCard");

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(6);

    auto *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName("infoTitle");

    valueLabel->setParent(card);
    valueLabel->setObjectName("infoValue");
    valueLabel->setWordWrap(true);

    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);

    return card;
}

usercabinet::usercabinet(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("userCabinet");

    m_welcomeLabel = new QLabel(tr("Вітаємо,"), this);
    m_welcomeLabel->setObjectName("pageTitle");

    m_photoLabel = new QLabel(this);
    m_photoLabel->setFixedSize(120, 160);
    m_photoLabel->setFrameShape(QFrame::Box);
    m_photoLabel->setAlignment(Qt::AlignCenter);
    m_photoLabel->setText(tr("Немає фото"));
    m_photoLabel->setScaledContents(true);

    m_changePhotoButton = new QPushButton(tr("Змінити фото"), this);

    m_nameLabel = new QLabel(tr("ПІБ: -"), this);
    m_addressLabel = new QLabel(tr("Адреса: -"), this);
    m_phoneLabel = new QLabel(tr("Телефон: -"), this);
    m_emailLabel = new QLabel(tr("E-mail: -"), this);

    m_totalBooksLabel = new QLabel("0", this);
    m_activeBooksLabel = new QLabel("0", this);
    m_returnedBooksLabel = new QLabel("0", this);
    m_overdueBooksLabel = new QLabel("0", this);
    m_topGenresLabel = new QLabel("-", this);
    m_topAuthorsLabel = new QLabel("-", this);

    m_chartTypeCombo = new QComboBox(this);
    m_chartTypeCombo->addItem(tr("За жанрами"));
    m_chartTypeCombo->addItem(tr("За авторами"));

    m_userChartView = new QChartView(this);
    m_userChartView->setMinimumHeight(320);
    m_userChartView->setRenderHint(QPainter::Antialiasing);

    m_loansView = new QTableView(this);
    m_loansView->horizontalHeader()->setStretchLastSection(true);
    m_loansView->verticalHeader()->setVisible(false);
    m_loansView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_loansView->setAlternatingRowColors(true);
    m_loansView->setShowGrid(false);
    m_loansView->verticalHeader()->setDefaultSectionSize(100);

    m_backButton = new QPushButton(tr("До каталогу"), this);
    m_logoutButton = new QPushButton(tr("Вийти"), this);

    m_tabs = new QTabWidget(this);
    m_profileTab = new QWidget(this);
    m_statsTab = new QWidget(this);
    m_historyTab = new QWidget(this);

    auto *profileCard = new QFrame(this);
    profileCard->setObjectName("topCard");

    auto *infoLayout = new QVBoxLayout;
    infoLayout->setSpacing(6);
    infoLayout->addWidget(m_nameLabel);
    infoLayout->addWidget(m_addressLabel);
    infoLayout->addWidget(m_phoneLabel);
    infoLayout->addWidget(m_emailLabel);
    infoLayout->addStretch();

    auto *leftLayout = new QVBoxLayout;
    leftLayout->setSpacing(10);
    leftLayout->addWidget(m_photoLabel, 0, Qt::AlignTop);
    leftLayout->addWidget(m_changePhotoButton);
    leftLayout->addStretch();

    auto *profileCardLayout = new QHBoxLayout(profileCard);
    profileCardLayout->setContentsMargins(16, 16, 16, 16);
    profileCardLayout->setSpacing(20);
    profileCardLayout->addLayout(leftLayout, 0);
    profileCardLayout->addLayout(infoLayout, 1);

    auto *profileLayout = new QVBoxLayout(m_profileTab);
    profileLayout->setContentsMargins(20, 20, 20, 20);
    profileLayout->setSpacing(16);
    profileLayout->addWidget(m_welcomeLabel);
    profileLayout->addWidget(profileCard);
    profileLayout->addStretch();

    auto *statsGrid = new QGridLayout;
    statsGrid->setHorizontalSpacing(12);
    statsGrid->setVerticalSpacing(12);
    statsGrid->addWidget(createKpiCard(tr("Усього взято"), m_totalBooksLabel, this), 0, 0);
    statsGrid->addWidget(createKpiCard(tr("Активні позики"), m_activeBooksLabel, this), 0, 1);
    statsGrid->addWidget(createKpiCard(tr("Повернено"), m_returnedBooksLabel, this), 1, 0);
    statsGrid->addWidget(createKpiCard(tr("Прострочено"), m_overdueBooksLabel, this), 1, 1);

    auto *favoritesGrid = new QGridLayout;
    favoritesGrid->setHorizontalSpacing(12);
    favoritesGrid->setVerticalSpacing(12);
    favoritesGrid->addWidget(createInfoCard(tr("Улюблені жанри"), m_topGenresLabel, this), 0, 0);
    favoritesGrid->addWidget(createInfoCard(tr("Улюблені автори"), m_topAuthorsLabel, this), 0, 1);

    auto *chartHeaderLayout = new QHBoxLayout;
    chartHeaderLayout->addWidget(new QLabel(tr("Графік читання:"), this));
    chartHeaderLayout->addWidget(m_chartTypeCombo);
    chartHeaderLayout->addStretch();

    auto *statsLayout = new QVBoxLayout(m_statsTab);
    statsLayout->setContentsMargins(20, 20, 20, 20);
    statsLayout->setSpacing(16);
    statsLayout->addWidget(new QLabel(tr("Моя статистика"), this));
    statsLayout->addLayout(statsGrid);
    statsLayout->addWidget(new QLabel(tr("Читацькі вподобання"), this));
    statsLayout->addLayout(favoritesGrid);
    statsLayout->addLayout(chartHeaderLayout);
    statsLayout->addWidget(m_userChartView);
    statsLayout->addStretch();

    auto *historyLayout = new QVBoxLayout(m_historyTab);
    historyLayout->setContentsMargins(20, 20, 20, 20);
    historyLayout->setSpacing(12);
    historyLayout->addWidget(new QLabel(tr("Історія позик:"), this));
    historyLayout->addWidget(m_loansView);

    m_tabs->addTab(m_profileTab, tr("Профіль"));
    m_tabs->addTab(m_statsTab, tr("Статистика"));
    m_tabs->addTab(m_historyTab, tr("Позики"));

    auto *buttonsLayout = new QHBoxLayout;
    buttonsLayout->setContentsMargins(20, 0, 20, 10);
    buttonsLayout->setSpacing(10);
    buttonsLayout->addWidget(m_backButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(m_logoutButton);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_tabs);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    connect(m_backButton, &QPushButton::clicked, this, &usercabinet::backToLibrary);
    connect(m_changePhotoButton, &QPushButton::clicked, this, &usercabinet::onChangePhotoClicked);
    connect(m_logoutButton, &QPushButton::clicked, this, &usercabinet::logoutRequested);
    connect(m_chartTypeCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &usercabinet::onChartTypeChanged);
}

void usercabinet::setUserName(const QString &name)
{
    m_username = name;
    m_welcomeLabel->setText(tr("Вітаємо, %1").arg(name));
}

void usercabinet::setUserInfo(const QString &fullName,
                              const QString &address,
                              const QString &phone,
                              const QString &email)
{
    m_nameLabel->setText(tr("ПІБ: %1").arg(fullName));
    m_addressLabel->setText(tr("Адреса: %1").arg(address));
    m_phoneLabel->setText(tr("Телефон: %1").arg(phone));
    m_emailLabel->setText(tr("E-mail: %1").arg(email));
}

void usercabinet::setUserPhoto(const QString &photoPath)
{
    QPixmap pix(photoPath);
    if (pix.isNull()) {
        m_photoLabel->setText(tr("Немає фото"));
        m_photoLabel->setPixmap(QPixmap());
    } else {
        QPixmap scaled = pix.scaled(m_photoLabel->size(),
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);
        m_photoLabel->setPixmap(scaled);
        m_photoLabel->setText(QString());
    }
}

void usercabinet::onChangePhotoClicked()
{
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Виберіть фото"),
                                                QString(),
                                                tr("Images (*.png *.jpg *.jpeg *.bmp)"));
    if (file.isEmpty())
        return;

    QPixmap pix(file);
    if (pix.isNull())
        return;

    setUserPhoto(file);

    if (m_username.isEmpty())
        return;

    QSqlQuery q(database::db());
    q.prepare("UPDATE Users SET photo_path = :p WHERE username = :u");
    q.bindValue(":p", file);
    q.bindValue(":u", m_username);

    if (!q.exec())
        qDebug() << "Update photo failed:" << q.lastError().text();
}

void usercabinet::setLoansModel(QAbstractItemModel *model)
{
    m_loansView->setModel(model);
    if (!model)
        return;

    m_loansView->setColumnHidden(0, true);
    m_loansView->setColumnHidden(2, true);

    m_loansView->setColumnWidth(1, 250);
    m_loansView->setColumnWidth(3, 120);
    m_loansView->setColumnWidth(4, 120);
    m_loansView->setColumnWidth(5, 140);
    m_loansView->setColumnWidth(6, 120);
}

void usercabinet::setReaderStats(int total,
                                 int active,
                                 int returned,
                                 int overdue,
                                 const QString &topGenres,
                                 const QString &topAuthors)
{
    m_totalBooksLabel->setText(QString::number(total));
    m_activeBooksLabel->setText(QString::number(active));
    m_returnedBooksLabel->setText(QString::number(returned));
    m_overdueBooksLabel->setText(QString::number(overdue));
    m_topGenresLabel->setText(topGenres);
    m_topAuthorsLabel->setText(topAuthors);
}

void usercabinet::setUserChart(QChart *chart)
{
    m_userChartView->setChart(chart);
}

int usercabinet::currentChartIndex() const
{
    return m_chartTypeCombo->currentIndex();
}

void usercabinet::onChartTypeChanged(int index)
{
    emit chartTypeChanged(index);
}

void usercabinet::resetToHomePage()
{
    if (m_tabs)
        m_tabs->setCurrentIndex(0);
}

#include "admincabinet.h"

#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QComboBox>
#include <QTabWidget>
#include <QFrame>

static QFrame* createKpiCard(const QString &title,
                             QLabel *valueLabel,
                             QWidget *parent)
{
    auto *card = new QFrame(parent);
    card->setObjectName("kpiCard");

    auto *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName("kpiTitle");

    valueLabel->setParent(card);
    valueLabel->setObjectName("kpiValue");

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(4);
    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);

    return card;
}

static QFrame* createInfoCard(const QString &title,
                              QLabel *valueLabel,
                              QWidget *parent)
{
    auto *card = new QFrame(parent);
    card->setObjectName("infoCard");

    auto *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName("infoLabel");

    valueLabel->setParent(card);
    valueLabel->setObjectName("infoValue");
    valueLabel->setWordWrap(true);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 10, 14, 10);
    layout->setSpacing(2);
    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);

    return card;
}

AdminCabinet::AdminCabinet(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("adminCabinet");

    m_exportReportButton = new QPushButton(tr("Експортувати звіт"), this);

    m_welcomeLabel = new QLabel(tr("Панель адміністратора"), this);
    m_welcomeLabel->setObjectName("pageTitle");

    auto *subtitleLabel = new QLabel(
        tr("Керування бібліотекою, користувачами та видачами"),
        this);
    subtitleLabel->setObjectName("pageSubtitle");

    m_nameLabel  = new QLabel(tr("ПІБ: -"), this);
    m_emailLabel = new QLabel(tr("E-mail: -"), this);

    m_totalBooksLabel     = new QLabel("0", this);
    m_availableBooksLabel = new QLabel("0", this);
    m_loanedBooksLabel    = new QLabel("0", this);
    m_activeLoansLabel    = new QLabel("0", this);
    m_overdueLoansLabel   = new QLabel("0", this);
    m_totalUsersLabel     = new QLabel("0", this);

    m_peakDayLabel   = new QLabel("-", this);
    m_peakMonthLabel = new QLabel("-", this);
    m_topBookLabel   = new QLabel("0", this);
    m_topGenreLabel  = new QLabel("-", this);

    m_chartTypeCombo = new QComboBox(this);
    m_chartTypeCombo->addItem(tr("Видачі по місяцях"));
    m_chartTypeCombo->addItem(tr("Книги за жанрами"));
    m_chartTypeCombo->addItem(tr("Книги за статусом"));
    m_chartTypeCombo->addItem(tr("Топ-5 книг"));

    m_loansChartView = new QChartView(this);
    m_loansChartView->setMinimumHeight(300);
    m_loansChartView->setRenderHint(QPainter::Antialiasing);

    m_loansView = new QTableView(this);
    m_loansView->horizontalHeader()->setStretchLastSection(true);
    m_loansView->verticalHeader()->setVisible(false);
    m_loansView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_loansView->setAlternatingRowColors(true);
    m_loansView->setShowGrid(false);

    m_badUsersView = new QTableView(this);
    m_badUsersView->horizontalHeader()->setStretchLastSection(true);
    m_badUsersView->verticalHeader()->setVisible(false);
    m_badUsersView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_badUsersView->setSelectionMode(QAbstractItemView::NoSelection);
    m_badUsersView->setAlternatingRowColors(true);
    m_badUsersView->setShowGrid(false);

    m_goodUsersView = new QTableView(this);
    m_goodUsersView->horizontalHeader()->setStretchLastSection(true);
    m_goodUsersView->verticalHeader()->setVisible(false);
    m_goodUsersView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_goodUsersView->setSelectionMode(QAbstractItemView::NoSelection);
    m_goodUsersView->setAlternatingRowColors(true);
    m_goodUsersView->setShowGrid(false);

    m_backButton   = new QPushButton(tr("До каталогу"), this);
    m_returnButton = new QPushButton(tr("Повернути книгу"), this);
    auto *loansReturnButton = new QPushButton(tr("Повернути книгу"), this);

    m_tabs         = new QTabWidget(this);
    m_overviewTab  = new QWidget(this);
    m_analyticsTab = new QWidget(this);
    m_loansTab     = new QWidget(this);

    auto *titleBlock = new QVBoxLayout;
    titleBlock->setSpacing(4);
    titleBlock->addWidget(m_welcomeLabel);
    titleBlock->addWidget(subtitleLabel);

    auto *profileCard = new QFrame(this);
    profileCard->setObjectName("topCard");

    auto *profileLayout = new QVBoxLayout(profileCard);
    profileLayout->setContentsMargins(16, 12, 16, 12);
    profileLayout->setSpacing(4);
    profileLayout->addWidget(m_nameLabel);
    profileLayout->addWidget(m_emailLabel);

    auto *topLayout = new QHBoxLayout;
    topLayout->setSpacing(16);
    topLayout->addLayout(titleBlock, 1);
    topLayout->addWidget(profileCard, 0);

    auto *statsLayout = new QGridLayout;
    statsLayout->setHorizontalSpacing(12);
    statsLayout->setVerticalSpacing(12);

    statsLayout->addWidget(createKpiCard(tr("Усього книг"),
                                         m_totalBooksLabel, this), 0, 0);
    statsLayout->addWidget(createKpiCard(tr("Доступні"),
                                         m_availableBooksLabel, this), 0, 1);
    statsLayout->addWidget(createKpiCard(tr("Видані"),
                                         m_loanedBooksLabel, this), 0, 2);
    statsLayout->addWidget(createKpiCard(tr("Активні видачі"),
                                         m_activeLoansLabel, this), 1, 0);
    statsLayout->addWidget(createKpiCard(tr("Прострочені"),
                                         m_overdueLoansLabel, this), 1, 1);
    statsLayout->addWidget(createKpiCard(tr("Користувачі"),
                                         m_totalUsersLabel, this), 1, 2);

    auto *peakCard = new QFrame(this);
    peakCard->setObjectName("peakCard");

    auto *peakCardLayout = new QHBoxLayout(peakCard);
    peakCardLayout->setContentsMargins(20, 20, 20, 20);
    peakCardLayout->setSpacing(24);

    auto *leftSection = new QVBoxLayout;
    leftSection->setSpacing(10);

    auto *topBookSectionTitle = new QLabel(tr("Найпопулярніша книга"), peakCard);
    topBookSectionTitle->setObjectName("sectionTitle");
    leftSection->addWidget(topBookSectionTitle);

    auto *topBookRow = new QHBoxLayout;
    topBookRow->setSpacing(18);
    topBookRow->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_topBookCard = new QFrame(peakCard);
    m_topBookCard->setObjectName("topBookCard");
    m_topBookCard->setMinimumWidth(250);
    m_topBookCard->setMaximumWidth(290);
    m_topBookCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto *topBookCardLayout = new QVBoxLayout(m_topBookCard);
    topBookCardLayout->setContentsMargins(0, 0, 0, 0);
    topBookCardLayout->setSpacing(0);

    m_topBookCoverLabel = new QLabel(m_topBookCard);
    m_topBookCoverLabel->setObjectName("topBookCover");
    m_topBookCoverLabel->setMinimumHeight(250);
    m_topBookCoverLabel->setAlignment(Qt::AlignCenter);
    m_topBookCoverLabel->setText(tr("Обкладинка"));
    m_topBookCoverLabel->setScaledContents(true);

    auto *bookInfoPanel = new QFrame(m_topBookCard);
    bookInfoPanel->setObjectName("topBookInfoPanel");

    auto *bookInfoLayout = new QVBoxLayout(bookInfoPanel);
    bookInfoLayout->setContentsMargins(16, 14, 16, 16);
    bookInfoLayout->setSpacing(4);

    m_topBookTitleValueLabel = new QLabel(tr("Назва книги"), bookInfoPanel);
    m_topBookTitleValueLabel->setObjectName("topBookTitleText");
    m_topBookTitleValueLabel->setAlignment(Qt::AlignCenter);
    m_topBookTitleValueLabel->setWordWrap(true);

    m_topBookMetaValueLabel = new QLabel(tr("Рік видання"), bookInfoPanel);
    m_topBookMetaValueLabel->setObjectName("topBookMetaText");
    m_topBookMetaValueLabel->setAlignment(Qt::AlignCenter);

    bookInfoLayout->addWidget(m_topBookTitleValueLabel);
    bookInfoLayout->addWidget(m_topBookMetaValueLabel);

    topBookCardLayout->addWidget(m_topBookCoverLabel);
    topBookCardLayout->addWidget(bookInfoPanel);

    topBookRow->addWidget(m_topBookCard, 0, Qt::AlignTop);

    auto *countBlock = new QVBoxLayout;
    countBlock->setSpacing(4);
    countBlock->setAlignment(Qt::AlignCenter);

    auto *countTitle = new QLabel(tr("Видач"), peakCard);
    countTitle->setObjectName("infoLabel");
    countTitle->setAlignment(Qt::AlignCenter);

    m_topBookLabel->setParent(peakCard);
    m_topBookLabel->setObjectName("topBookCountLabel");
    m_topBookLabel->setAlignment(Qt::AlignCenter);

    countBlock->addStretch();
    countBlock->addWidget(countTitle);
    countBlock->addWidget(m_topBookLabel);
    countBlock->addStretch();

    topBookRow->addLayout(countBlock, 1);
    leftSection->addLayout(topBookRow);

    auto *rightSection = new QVBoxLayout;
    rightSection->setSpacing(12);

    auto *rightSectionTitle = new QLabel(tr("Пікові показники"), peakCard);
    rightSectionTitle->setObjectName("sectionTitle");
    rightSection->addWidget(rightSectionTitle);

    auto *peakDayCard = createInfoCard(tr("Піковий день"), m_peakDayLabel, peakCard);
    peakDayCard->setObjectName("infoCard");

    auto *peakMonthCard = createInfoCard(tr("Піковий місяць"), m_peakMonthLabel, peakCard);
    peakMonthCard->setObjectName("infoCard");

    auto *topGenreCard = createInfoCard(tr("Найпопулярніший жанр"), m_topGenreLabel, peakCard);
    topGenreCard->setObjectName("infoCard");

    rightSection->addWidget(peakDayCard);
    rightSection->addWidget(peakMonthCard);
    rightSection->addWidget(topGenreCard);
    rightSection->addStretch();

    peakCardLayout->addLayout(leftSection, 3);
    peakCardLayout->addLayout(rightSection, 2);

    auto *overviewLayout = new QVBoxLayout(m_overviewTab);
    overviewLayout->setContentsMargins(20, 20, 20, 20);
    overviewLayout->setSpacing(16);
    overviewLayout->addLayout(topLayout);
    overviewLayout->addSpacing(8);
    overviewLayout->addWidget(new QLabel(tr("Загальна статистика"), this));
    overviewLayout->addLayout(statsLayout);
    overviewLayout->addSpacing(8);
    overviewLayout->addWidget(peakCard);
    overviewLayout->addStretch();

    auto *chartHeaderLayout = new QHBoxLayout;
    chartHeaderLayout->addWidget(new QLabel(tr("Графік:"), this));
    chartHeaderLayout->addWidget(m_chartTypeCombo);
    chartHeaderLayout->addStretch();

    auto *chartCard = new QFrame(this);
    chartCard->setObjectName("chartCard");

    auto *chartCardLayout = new QVBoxLayout(chartCard);
    chartCardLayout->setContentsMargins(16, 16, 16, 16);
    chartCardLayout->setSpacing(8);
    chartCardLayout->addWidget(m_loansChartView);

    auto *analyticsLayout = new QVBoxLayout(m_analyticsTab);
    analyticsLayout->setContentsMargins(20, 20, 20, 20);
    analyticsLayout->setSpacing(12);
    analyticsLayout->addLayout(chartHeaderLayout);
    analyticsLayout->addWidget(chartCard);
    analyticsLayout->addStretch();

    auto *loansLayout = new QVBoxLayout(m_loansTab);
    loansLayout->setContentsMargins(20, 20, 20, 20);
    loansLayout->setSpacing(12);
    loansLayout->addWidget(new QLabel(tr("Усі видачі:"), this));
    loansLayout->addWidget(m_loansView);
    loansLayout->addWidget(new QLabel(tr("Проблемні користувачі:"), this));
    loansLayout->addWidget(m_badUsersView);
    loansLayout->addWidget(new QLabel(tr("Рейтинг сумлінних читачів:"), this));
    loansLayout->addWidget(m_goodUsersView);

    auto *loansButtonsLayout = new QHBoxLayout;
    loansButtonsLayout->addStretch();
    loansButtonsLayout->addWidget(loansReturnButton);
    loansLayout->addLayout(loansButtonsLayout);

    m_tabs->addTab(m_overviewTab,  tr("Огляд"));
    m_tabs->addTab(m_analyticsTab, tr("Аналітика"));
    m_tabs->addTab(m_loansTab,     tr("Видачі"));

    auto *buttonsLayout = new QHBoxLayout;
    buttonsLayout->setContentsMargins(20, 0, 20, 10);
    buttonsLayout->setSpacing(10);
    buttonsLayout->addWidget(m_exportReportButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(m_returnButton);
    buttonsLayout->addWidget(m_backButton);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_tabs);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    connect(m_backButton, &QPushButton::clicked,
            this, &AdminCabinet::backToLibrary);
    connect(m_returnButton, &QPushButton::clicked,
            this, &AdminCabinet::onReturnClicked);
    connect(loansReturnButton, &QPushButton::clicked,
            this, &AdminCabinet::onReturnClicked);
    connect(m_chartTypeCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdminCabinet::onChartTypeChanged);
    connect(m_exportReportButton, &QPushButton::clicked,
            this, &AdminCabinet::exportReportRequested);
}

void AdminCabinet::setAdminInfo(const QString &fullName,
                                const QString &email)
{
    m_nameLabel->setText(tr("ПІБ: %1").arg(fullName));
    m_emailLabel->setText(tr("E-mail: %1").arg(email));
}

void AdminCabinet::setStats(int totalBooks,
                            int availableBooks,
                            int loanedBooks,
                            int activeLoans,
                            int overdueLoans,
                            int totalUsers)
{
    m_totalBooksLabel->setText(QString::number(totalBooks));
    m_availableBooksLabel->setText(QString::number(availableBooks));
    m_loanedBooksLabel->setText(QString::number(loanedBooks));
    m_activeLoansLabel->setText(QString::number(activeLoans));
    m_overdueLoansLabel->setText(QString::number(overdueLoans));
    m_totalUsersLabel->setText(QString::number(totalUsers));
}

void AdminCabinet::setLoansModel(QAbstractItemModel *model)
{
    m_loansView->setModel(model);
    if (!model)
        return;

    m_loansView->setColumnHidden(0, true);
    m_loansView->setColumnHidden(7, true);
}

void AdminCabinet::onReturnClicked()
{
    QModelIndex idx = m_loansView->currentIndex();
    if (!idx.isValid())
        return;

    emit returnLoanRequested(idx);
}

void AdminCabinet::setBadUsersModel(QAbstractItemModel *model)
{
    if (!m_badUsersView)
        return;

    m_badUsersView->setModel(model);
    if (!model)
        return;

    m_badUsersView->horizontalHeader()->setStretchLastSection(true);
}

void AdminCabinet::setGoodUsersModel(QAbstractItemModel *model)
{
    m_goodUsersView->setModel(model);
    if (!model)
        return;

    m_goodUsersView->horizontalHeader()->setStretchLastSection(true);
}

void AdminCabinet::setPeakStats(const QString &peakDay,
                                const QString &peakMonth,
                                const QString &topGenre)
{
    m_peakDayLabel->setText(peakDay);
    m_peakMonthLabel->setText(peakMonth);
    m_topGenreLabel->setText(topGenre);

    qDebug() << "setPeakStats peakDay =" << peakDay;
    qDebug() << "setPeakStats peakMonth =" << peakMonth;
    qDebug() << "setPeakStats topGenre =" << topGenre;
}

void AdminCabinet::setLoansChart(QChart *chart)
{
    m_loansChartView->setChart(chart);
}

int AdminCabinet::currentChartIndex() const
{
    return m_chartTypeCombo->currentIndex();
}

void AdminCabinet::onChartTypeChanged(int index)
{
    emit chartTypeChanged(index);
}

void AdminCabinet::resetToHomePage()
{
    if (m_tabs)
        m_tabs->setCurrentIndex(0);
}

void AdminCabinet::setTopBookCard(const QString &title,
                                  const QString &meta,
                                  const QPixmap &cover,
                                  int loansCount)
{
    if (m_topBookTitleValueLabel)
        m_topBookTitleValueLabel->setText(title);

    if (m_topBookMetaValueLabel)
        m_topBookMetaValueLabel->setText(meta);

    if (m_topBookCoverLabel) {
        if (!cover.isNull()) {
            m_topBookCoverLabel->setPixmap(cover);
            m_topBookCoverLabel->setScaledContents(true);
            m_topBookCoverLabel->setText(QString());
        } else {
            m_topBookCoverLabel->setPixmap(QPixmap());
            m_topBookCoverLabel->setText(tr("Немає обкладинки"));
        }
    }

    if (m_topBookLabel)
        m_topBookLabel->setText(QString::number(loansCount));
}

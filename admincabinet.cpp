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

AdminCabinet::AdminCabinet(QWidget *parent)
    : QWidget(parent)
{
    m_exportReportButton = new QPushButton(tr("Експортувати звіт"), this);

    m_welcomeLabel = new QLabel(tr("Admin panel"), this);
    m_nameLabel  = new QLabel(tr("ПІБ: -"), this);
    m_emailLabel = new QLabel(tr("E-mail: -"), this);

    m_totalBooksLabel     = new QLabel(tr("Усього книг: 0"), this);
    m_availableBooksLabel = new QLabel(tr("Доступні: 0"), this);
    m_loanedBooksLabel    = new QLabel(tr("Видані: 0"), this);
    m_activeLoansLabel    = new QLabel(tr("Активні позики: 0"), this);
    m_overdueLoansLabel   = new QLabel(tr("Прострочені: 0"), this);
    m_totalUsersLabel     = new QLabel(tr("Користувачі: 0"), this);

    m_peakDayLabel   = new QLabel(tr("Піковий день: -"), this);
    m_peakMonthLabel = new QLabel(tr("Піковий місяць: -"), this);
    m_topBookLabel   = new QLabel(tr("Найпопулярніша книга: -"), this);
    m_topGenreLabel  = new QLabel(tr("Найпопулярніший жанр: -"), this);

    m_chartTypeCombo = new QComboBox(this);
    m_chartTypeCombo->addItem(tr("Видачі по місяцях"));
    m_chartTypeCombo->addItem(tr("Розподіл за жанрами"));

    m_loansChartView = new QChartView(this);
    m_loansChartView->setMinimumHeight(300);
    m_loansChartView->setRenderHint(QPainter::Antialiasing);

    m_loansView = new QTableView(this);
    m_loansView->horizontalHeader()->setStretchLastSection(true);
    m_loansView->verticalHeader()->setVisible(false);
    m_loansView->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_badUsersView = new QTableView(this);
    m_badUsersView->horizontalHeader()->setStretchLastSection(true);
    m_badUsersView->verticalHeader()->setVisible(false);
    m_badUsersView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_badUsersView->setSelectionMode(QAbstractItemView::NoSelection);

    m_goodUsersView = new QTableView(this);
    m_goodUsersView->horizontalHeader()->setStretchLastSection(true);
    m_goodUsersView->verticalHeader()->setVisible(false);
    m_goodUsersView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_goodUsersView->setSelectionMode(QAbstractItemView::NoSelection);

    m_backButton   = new QPushButton(tr("To Catalogue"), this);
    m_returnButton = new QPushButton(tr("Повернути книгу"), this);

    m_tabs = new QTabWidget(this);
    m_overviewTab = new QWidget(this);
    m_analyticsTab = new QWidget(this);
    m_loansTab = new QWidget(this);

    auto *infoLayout = new QVBoxLayout;
    infoLayout->addWidget(m_nameLabel);
    infoLayout->addWidget(m_emailLabel);
    infoLayout->addStretch();

    auto *topLayout = new QHBoxLayout;
    topLayout->addWidget(m_welcomeLabel);
    topLayout->addStretch();
    topLayout->addLayout(infoLayout);

    auto *statsLayout = new QGridLayout;
    statsLayout->addWidget(m_totalBooksLabel,     0, 0);
    statsLayout->addWidget(m_availableBooksLabel, 0, 1);
    statsLayout->addWidget(m_loanedBooksLabel,    0, 2);
    statsLayout->addWidget(m_activeLoansLabel,    1, 0);
    statsLayout->addWidget(m_overdueLoansLabel,   1, 1);
    statsLayout->addWidget(m_totalUsersLabel,     1, 2);

    auto *peakLayout = new QGridLayout;
    peakLayout->addWidget(m_peakDayLabel,   0, 0);
    peakLayout->addWidget(m_peakMonthLabel, 0, 1);
    peakLayout->addWidget(m_topBookLabel,   1, 0);
    peakLayout->addWidget(m_topGenreLabel,  1, 1);

    auto *overviewLayout = new QVBoxLayout(m_overviewTab);
    overviewLayout->addLayout(topLayout);
    overviewLayout->addWidget(new QLabel(tr("Загальна статистика:"), this));
    overviewLayout->addLayout(statsLayout);
    overviewLayout->addSpacing(12);
    overviewLayout->addWidget(new QLabel(tr("Пікові показники:"), this));
    overviewLayout->addLayout(peakLayout);
    overviewLayout->addStretch();

    auto *chartHeaderLayout = new QHBoxLayout;
    chartHeaderLayout->addWidget(new QLabel(tr("Графік:"), this));
    chartHeaderLayout->addWidget(m_chartTypeCombo);
    chartHeaderLayout->addStretch();

    auto *analyticsLayout = new QVBoxLayout(m_analyticsTab);
    analyticsLayout->addLayout(chartHeaderLayout);
    analyticsLayout->addWidget(m_loansChartView);
    analyticsLayout->addStretch();

    auto *loansLayout = new QVBoxLayout(m_loansTab);
    loansLayout->addWidget(new QLabel(tr("Усі позики:"), this));
    loansLayout->addWidget(m_loansView);
    loansLayout->addWidget(new QLabel(tr("Проблемні користувачі:"), this));
    loansLayout->addWidget(m_badUsersView);
    loansLayout->addWidget(new QLabel(tr("Рейтинг сумлінних читачів:"), this));
    loansLayout->addWidget(m_goodUsersView);
    loansLayout->addWidget(m_returnButton, 0, Qt::AlignRight);

    m_tabs->addTab(m_overviewTab, tr("Огляд"));
    m_tabs->addTab(m_analyticsTab, tr("Аналітика"));
    m_tabs->addTab(m_loansTab, tr("Позики"));

    auto *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(m_exportReportButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(m_returnButton);
    buttonsLayout->addWidget(m_backButton);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_tabs);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    connect(m_backButton, &QPushButton::clicked,
            this, &AdminCabinet::backToLibrary);
    connect(m_returnButton, &QPushButton::clicked,
            this, &AdminCabinet::onReturnClicked);
    connect(m_chartTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
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
    m_totalBooksLabel->setText(tr("Усього книг: %1").arg(totalBooks));
    m_availableBooksLabel->setText(tr("Доступні: %1").arg(availableBooks));
    m_loanedBooksLabel->setText(tr("Видані: %1").arg(loanedBooks));
    m_activeLoansLabel->setText(tr("Активні позики: %1").arg(activeLoans));
    m_overdueLoansLabel->setText(tr("Прострочені: %1").arg(overdueLoans));
    m_totalUsersLabel->setText(tr("Користувачі: %1").arg(totalUsers));
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
                                const QString &topBook,
                                const QString &topGenre)
{
    m_peakDayLabel->setText(tr("Піковий день: %1").arg(peakDay));
    m_peakMonthLabel->setText(tr("Піковий місяць: %1").arg(peakMonth));
    m_topBookLabel->setText(tr("Найпопулярніша книга: %1").arg(topBook));
    m_topGenreLabel->setText(tr("Найпопулярніший жанр: %1").arg(topGenre));
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

#include "admincabinet.h"

#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>

AdminCabinet::AdminCabinet(QWidget *parent)
    : QWidget(parent)
{
    m_welcomeLabel = new QLabel(tr("Admin panel"), this);
    m_nameLabel  = new QLabel(tr("ПІБ: -"), this);
    m_emailLabel = new QLabel(tr("E-mail: -"), this);

    auto *infoLayout = new QVBoxLayout;
    infoLayout->addWidget(m_nameLabel);
    infoLayout->addWidget(m_emailLabel);
    infoLayout->addStretch();

    auto *topLayout = new QHBoxLayout;
    topLayout->addWidget(m_welcomeLabel);
    topLayout->addStretch();
    topLayout->addLayout(infoLayout);

    m_totalBooksLabel     = new QLabel(tr("Усього книг: 0"), this);
    m_availableBooksLabel = new QLabel(tr("Доступні: 0"), this);
    m_loanedBooksLabel    = new QLabel(tr("Видані: 0"), this);
    m_activeLoansLabel    = new QLabel(tr("Активні позики: 0"), this);
    m_overdueLoansLabel   = new QLabel(tr("Прострочені: 0"), this);
    m_totalUsersLabel     = new QLabel(tr("Користувачі: 0"), this);

    auto *statsLayout = new QGridLayout;
    statsLayout->addWidget(m_totalBooksLabel,     0, 0);
    statsLayout->addWidget(m_availableBooksLabel, 0, 1);
    statsLayout->addWidget(m_loanedBooksLabel,    0, 2);
    statsLayout->addWidget(m_activeLoansLabel,    1, 0);
    statsLayout->addWidget(m_overdueLoansLabel,   1, 1);
    statsLayout->addWidget(m_totalUsersLabel,     1, 2);

    m_loansView = new QTableView(this);
    m_loansView->horizontalHeader()->setStretchLastSection(true);
    m_loansView->verticalHeader()->setVisible(false);
    m_loansView->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_backButton   = new QPushButton(tr("To Catalogue"), this);
    m_returnButton = new QPushButton(tr("Повернути книгу"), this);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(statsLayout);
    mainLayout->addWidget(new QLabel(tr("Усі позики:"), this));
    mainLayout->addWidget(m_loansView);
    mainLayout->addWidget(m_returnButton);
    mainLayout->addWidget(m_backButton);
    setLayout(mainLayout);

    connect(m_backButton, &QPushButton::clicked,
            this, &AdminCabinet::backToLibrary);
    connect(m_returnButton, &QPushButton::clicked,
            this, &AdminCabinet::onReturnClicked);
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


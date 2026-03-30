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

    m_loansView = new QTableView(this);
    m_loansView->horizontalHeader()->setStretchLastSection(true);
    m_loansView->verticalHeader()->setVisible(false);
    m_loansView->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_backButton   = new QPushButton(tr("To Catalogue"), this);
    m_returnButton = new QPushButton(tr("Повернути книгу"), this);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
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


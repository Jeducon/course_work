#include "usercabinet.h"
#include "database.h"

#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSqlQuery>
#include <QTableView>
#include <QVBoxLayout>

usercabinet::usercabinet(QWidget *parent)
    : QWidget(parent)
{
    m_welcomeLabel = new QLabel(tr("Welcome, user"), this);

    m_photoLabel = new QLabel(this);
    m_photoLabel->setFixedSize(120, 160);
    m_photoLabel->setFrameShape(QFrame::Box);
    m_photoLabel->setAlignment(Qt::AlignCenter);
    m_photoLabel->setText(tr("No photo"));
    m_photoLabel->setScaledContents(true);

    m_changePhotoButton = new QPushButton(tr("Change Photo"), this);

    m_nameLabel = new QLabel(tr("ПІБ: -"), this);
    m_addressLabel = new QLabel(tr("Адреса: -"), this);
    m_phoneLabel = new QLabel(tr("Телефон: -"), this);
    m_emailLabel = new QLabel(tr("E-mail: -"), this);

    auto *infoLayout = new QVBoxLayout;
    infoLayout->addWidget(m_nameLabel);
    infoLayout->addWidget(m_addressLabel);
    infoLayout->addWidget(m_phoneLabel);
    infoLayout->addWidget(m_emailLabel);
    infoLayout->addStretch();

    auto *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(m_photoLabel);
    leftLayout->addWidget(m_changePhotoButton);
    leftLayout->addStretch();

    auto *topLayout = new QHBoxLayout;
    topLayout->addLayout(leftLayout);
    topLayout->addLayout(infoLayout);
    topLayout->addStretch();

    m_loansView = new QTableView(this);
    m_loansView->horizontalHeader()->setStretchLastSection(true);
    m_loansView->verticalHeader()->setVisible(false);
    m_loansView->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_loansView ->verticalHeader()->setDefaultSectionSize(100);

    m_backButton = new QPushButton(tr("To Catalogue"), this);
    m_logoutButton = new QPushButton(tr("Log out"), this);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_welcomeLabel);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(new QLabel(tr("History of loans:"), this));
    mainLayout->addWidget(m_loansView);
    mainLayout->addWidget(m_backButton);
    mainLayout->addWidget(m_logoutButton);
    setLayout(mainLayout);

    connect(m_backButton, &QPushButton::clicked, this, &usercabinet::backToLibrary);

    connect(m_changePhotoButton, &QPushButton::clicked, this, &usercabinet::onChangePhotoClicked);

    connect(m_logoutButton, &QPushButton::clicked, this, &usercabinet::logoutRequested);
}

void usercabinet::setUserName(const QString &name)
{
    m_username = name;
    m_welcomeLabel->setText(tr("Welcome, %1").arg(name));
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
        m_photoLabel->setText(tr("No photo"));
        m_photoLabel->setPixmap(QPixmap());
    } else {
        m_photoLabel->setPixmap(pix);
        m_photoLabel->setText(QString());
    }
}

void usercabinet::onChangePhotoClicked()
{
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Choose Photo"),
                                                QString(),
                                                tr("Images (*.png *.jpg *.jpeg *.bmp"));
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
    q.exec();
}

void usercabinet::setLoansModel(QAbstractItemModel* model){
    m_loansView -> setModel(model);
    if(!model)
        return;
    m_loansView -> setColumnHidden(0, true);
    m_loansView -> setColumnHidden(2, true);

    m_loansView -> setColumnWidth(1, 250);
    m_loansView -> setColumnWidth(3, 120);
    m_loansView -> setColumnWidth(4, 120);
    m_loansView -> setColumnWidth(5, 140);
    m_loansView -> setColumnWidth(6, 120);
}

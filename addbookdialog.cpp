#include "addbookdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>

addbookdialog::addbookdialog(QWidget *parent)
    : QDialog(parent)
{
    m_titleEdit = new QLineEdit(this);
    m_authorEdit = new QLineEdit(this);
    m_genreEdit = new QLineEdit(this);
    m_yearEdit = new QLineEdit(this);
    m_statusCombo = new QComboBox(this);

    m_statusCombo->addItem("available");
    m_statusCombo->addItem("loaned");

    auto *form = new QFormLayout;
    form->addRow(tr("Назва:"), m_titleEdit);
    form->addRow(tr("Автор:"), m_authorEdit);
    form->addRow(tr("Жанр:"), m_genreEdit);
    form->addRow(tr("Рік:"), m_yearEdit);
    form->addRow(tr("Статус:"), m_statusCombo);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addWidget(buttons);
    setLayout(mainLayout);

    setWindowTitle(tr("Додати книгу"));
}

QString addbookdialog::title() const
{
    return m_titleEdit->text().trimmed();
}
QString addbookdialog::author() const
{
    return m_authorEdit->text().trimmed();
}
QString addbookdialog::genre() const
{
    return m_genreEdit->text().trimmed();
}
int addbookdialog::year() const
{
    return m_yearEdit->text().toInt();
}
QString addbookdialog::status() const
{
    return m_statusCombo->currentText();
}

#pragma once

#ifndef ADDBOOKDIALOG_H
#define ADDBOOKDIALOG_H

#include <QDialog>

class QLineEdit;
class QComboBox;

class addbookdialog : public QDialog
{
    Q_OBJECT
public:
    explicit addbookdialog(QWidget *parent = nullptr);

    QString title() const;
    QString author() const;
    QString genre() const;
    int year() const;
    QString status() const;

private:
    QLineEdit *m_titleEdit;
    QLineEdit *m_authorEdit;
    QLineEdit *m_genreEdit;
    QLineEdit *m_yearEdit;
    QComboBox *m_statusCombo;
};

#endif

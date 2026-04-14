#pragma once

#ifndef USERCABINET_H
#define USERCABINET_H

#include <QWidget>
#include <QAbstractItemModel>
#include <QChart>

class QChart;
class QLabel;
class QTableView;
class QPushButton;
class QChartView;
class QComboBox;
class QTabWidget;

class usercabinet : public QWidget
{
    Q_OBJECT
public:
    explicit usercabinet(QWidget *parent = nullptr);

    void setUserName(const QString &name);
    void setUserInfo(const QString &fullName,
                     const QString &address,
                     const QString &phone,
                     const QString &email);
    void setUserPhoto(const QString &photoPath);
    void setLoansModel(QAbstractItemModel *model);

    void setReaderStats(int total,
                        int active,
                        int returned,
                        int overdue,
                        const QString &topGenres,
                        const QString &topAuthors);

    void setUserChart(QChart *chart);
    int currentChartIndex() const;


signals:
    void backToLibrary();
    void logoutRequested();
    void chartTypeChanged(int index);
private slots:
    void onChangePhotoClicked();
    void onChartTypeChanged(int index);
private:
    QString m_username;

    QLabel *m_welcomeLabel;
    QLabel *m_photoLabel;
    QLabel *m_nameLabel;
    QLabel *m_addressLabel;
    QLabel *m_phoneLabel;
    QLabel *m_emailLabel;

    QLabel *m_totalBooksLabel;
    QLabel *m_activeBooksLabel;
    QLabel *m_returnedBooksLabel;
    QLabel *m_overdueBooksLabel;
    QLabel *m_topGenresLabel;
    QLabel *m_topAuthorsLabel;

    QTableView *m_loansView;
    QPushButton *m_backButton;
    QPushButton *m_changePhotoButton;
    QPushButton *m_logoutButton;

    QChartView *m_userChartView;
    QComboBox *m_chartTypeCombo;

    QTabWidget *m_tabs;

    QWidget *m_profileTab;
    QWidget *m_statsTab;
    QWidget *m_historyTab;
};

#endif

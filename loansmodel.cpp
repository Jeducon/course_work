#include "loansmodel.h"
#include "QPixmap"
#include <QDate>

QVariant LoansModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const QVariant baseValue = QSqlQueryModel::data(index, role);

    const int statusColumn = 6;
    const int dueDateColumn = 4;

    if (role == Qt::BackgroundRole) {
        const QString status = QSqlQueryModel::data(this->index(index.row(), statusColumn)).toString();
        const QString dueDateStr = QSqlQueryModel::data(this->index(index.row(), dueDateColumn)).toString();
        const QDate dueDate = QDate::fromString(dueDateStr, "yyyy-MM-dd");
        const QDate today = QDate::currentDate();

        if (status == "overdue") {
            return QColor("#f8d7da");
        }

        if (status == "returned") {
            return QColor("#e6f4ea");
        }

        if (status == "active" && dueDate.isValid()) {
            const int daysLeft = today.daysTo(dueDate);
            if (daysLeft >= 0 && daysLeft <= 3) {
                return QColor("#fff3cd");
            }
        }
    }

    if (role == Qt::ForegroundRole) {
        const QString status = QSqlQueryModel::data(this->index(index.row(), statusColumn)).toString();
        if (status == "overdue")
            return QColor("#842029");
        if (status == "returned")
            return QColor("#1e4620");
    }

    return baseValue;
}






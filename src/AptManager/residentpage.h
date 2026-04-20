#ifndef RESIDENTPAGE_H
#define RESIDENTPAGE_H

#include <QWidget>
#include "residentmanager.h"

class QTableView;
class QLineEdit;
class QComboBox;

class ResidentPage : public QWidget
{
    Q_OBJECT
public:
    explicit ResidentPage(ResidentManager *mgr, QWidget *parent = nullptr);

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onSearch();
    void onClear();
    void onRowDoubleClicked(const QModelIndex &index);

private:
    ResidentManager *m_mgr;

    QTableView *m_table;
    QComboBox  *m_searchField;
    QLineEdit  *m_searchEdit;

    int  selectedResidentId() const;
    void showResidentDialog(bool isEdit);
};

#endif // RESIDENTPAGE_H

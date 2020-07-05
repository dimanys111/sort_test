#ifndef SORTWINDOW_H
#define SORTWINDOW_H

#include <QFuture>
#include <QMainWindow>
#include <sortfile.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class SortWindow;
}
QT_END_NAMESPACE

class SortWindow : public QMainWindow {
    Q_OBJECT

public:
    SortWindow(QWidget* parent = nullptr);
    ~SortWindow();

private slots:
    void on_act_create_triggered();

    void on_act_open_sort_file_triggered();

    void on_act_open_view_file_triggered();

    void on_verticalScrollBar_valueChanged(int value);
    void set_pb(int pos);
    void set_fin();

    void set_fin_();

private:
    QFuture<void> fut_create_file;
    QFuture<void> fut_run;
    QFile file;
    SortFile sf;
    Ui::SortWindow* ui;
    void create_file(QString& name_file, qint64 size_file);
    volatile bool stop{false};
signals:
    void emit_send_pos(int pos);
    void emit_fin();
};
#endif // SORTWINDOW_H

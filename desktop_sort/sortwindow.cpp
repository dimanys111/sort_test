#include "sortwindow.h"
#include "ui_sortwindow.h"

#include <QFileDialog>
#include <QtConcurrent>
#include <random>

#include <QMessageBox>
#include <QTableWidgetItem>

int send_pb = 0;

qint64 size_memori = 100 * std::pow(10, 6); // размер памяти
qint64 size_file = 10 * std::pow(10, 9); //размер выходного файла

void SortWindow::create_file(QString& name_file, qint64 size_file)
{
    QFile f;
    f.setFileName(name_file);
    if (f.open(QFile::WriteOnly)) {
        std::uniform_int_distribution<> dist(0, INT_MAX);
        std::mt19937 gen;
        gen.seed(time(0));

        auto count = size_file / sizeof(double); // количество значений
        auto count_d_mem = size_memori / sizeof(double);
        auto d_vec = std::vector<double>(count_d_mem);
        decltype(count_d_mem) j = 0;
        for (decltype(count) i = 0; i < count; i++) {
            if (stop) {
                f.close();
                return;
            }
            d_vec[j] = dist(gen) / static_cast<double>(INT_MAX);
            j++;
            if (j >= count_d_mem) {
                f.write(reinterpret_cast<char*>(d_vec.data()), j * sizeof(double));
                j = 0;
                emit emit_send_pos(f.pos() / send_pb);
            }
        }
        f.close();
    }
    emit emit_fin();
}

SortWindow::SortWindow(QWidget* parent)
    : QMainWindow(parent)
    , sf(size_memori)
    , ui(new Ui::SortWindow)
{
    ui->setupUi(this);
    ui->tableWidget->verticalScrollBar()->setVisible(false);
    ui->tableWidget->verticalScrollBar()->setDisabled(true);
    ui->tableWidget->verticalHeader()->setVisible(false);

    connect(this, &SortWindow::emit_send_pos, this, &SortWindow::set_pb);
    connect(this, &SortWindow::emit_fin, this, &SortWindow::set_fin_);
}

SortWindow::~SortWindow()
{
    delete ui;
    sf.stop = true;
    stop = true;
}

void SortWindow::on_act_create_triggered()
{
    auto fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "/home", tr("All (*)"));
    if (fileName != "") {
        send_pb = size_file / 100;
        ui->progressBar->setMaximum(size_file / send_pb);
        ui->progressBar->setValue(0);
        ui->label->setText("Создание рандомного файла");
        while (send_pb % 8 != 0) {
            send_pb--;
        }
        fut_create_file = QtConcurrent::run(this, &SortWindow::create_file, std::move(fileName), size_file);
    }
}

void SortWindow::set_pb(int pos)
{
    ui->progressBar->setValue(pos);
}

void SortWindow::set_fin_()
{
    ui->label->setText("Рандомный файл создан");
    ui->progressBar->setValue(0);
}

void SortWindow::set_fin()
{
    ui->progressBar->setValue(0);
    ui->label->setText("Файл отсортирован");
}

void SortWindow::on_act_open_sort_file_triggered()
{
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);
    connect(&sf, &SortFile::emit_progress, this, &SortWindow::set_pb);
    connect(&sf, &SortFile::emit_fin, this, &SortWindow::set_fin);
    auto s1 = QFileDialog::getOpenFileName(this, tr("Открыть входной файл"), "/home", tr("All (*)"));
    auto s2 = QFileDialog::getSaveFileName(this, tr("Сохранить отсортированный файл"), "/home", tr("All (*)"));
    if (s1 != "" && s2 != "") {
        ui->label->setText("Создание отсортированного файла");
        fut_run = QtConcurrent::run(&sf, &SortFile::run,
            s1, s2);
    } else {
        QMessageBox msgBox;
        msgBox.setText("В ходной или выходной фал не был задан!");
        msgBox.exec();
    }
}

qint64 sb_del = 1000000;
auto row_tabs = 50;

void SortWindow::on_act_open_view_file_triggered()
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Сохранить рандомный файл"), "/home", tr("All (*)"));
    if (fileName != "") {
        file.setFileName(fileName);
        if (file.open(QFile::ReadOnly)) {
            sb_del = file.size() / row_tabs;
            ui->verticalScrollBar->setMaximum(file.size() / sb_del);
            ui->verticalScrollBar->setMinimum(0);
            ui->tableWidget->setRowCount(row_tabs);
            for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
                delete ui->tableWidget->takeItem(i, 0);
            }
            for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
                double d;
                file.read(reinterpret_cast<char*>(&d), sizeof(double));
                QTableWidgetItem* newItem = new QTableWidgetItem(tr("%1").arg(d));
                ui->tableWidget->setItem(i, 0, newItem);
            }
        }
    }
}

void SortWindow::on_verticalScrollBar_valueChanged(int value)
{
    qint64 pos64 = value * sb_del;
    while (pos64 % sizeof(double) != 0) {
        pos64++;
    }

    if ((file.size() - pos64) < sizeof(double) * row_tabs) {
        pos64 = file.size() - sizeof(double) * (row_tabs - 1);
    }

    file.seek(pos64);

    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        delete ui->tableWidget->takeItem(i, 0);
    }
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        double d;
        file.read(reinterpret_cast<char*>(&d), sizeof(double));
        QTableWidgetItem* newItem = new QTableWidgetItem(tr("%1").arg(d));
        ui->tableWidget->setItem(i, 0, newItem);
    }
}

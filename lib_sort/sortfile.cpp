#include "sortfile.h"

#include <iostream>
#include <math.h>

SortFile::SortFile(qint64 size_memori)
    : size_memori(size_memori)
{
}

std::vector<std::shared_ptr<QFile>> SortFile::create_sort_files_buf(const QString& name_file)
{
    std::vector<std::shared_ptr<QFile>> fo_vec;
    QDir dir_sort("dir_sort");
    if (!dir_sort.exists()) {
        QDir dir;
        dir.mkdir("dir_sort");
    }
    if (dir_sort.exists()) {
        QFile f(name_file);
        if (f.open(QFile::ReadOnly)) {
            auto length = size_memori / sizeof(double);
            auto size_read = length * sizeof(double);
            auto darr = std::vector<double>(length);
            int i = 0;

            size_file = f.size();
            auto num_files = size_file / size_read;

            while (!f.atEnd()) {
                auto size = f.read(reinterpret_cast<char*>(darr.data()), size_read);
                length = size / sizeof(double);
                std::sort(darr.begin(), darr.begin() + length);

                fo_vec.emplace_back(std::make_shared<QFile>("dir_sort/" + QString::number(i)));
                if (fo_vec.back()->open(QFile::WriteOnly)) {
                    fo_vec.back()->write(reinterpret_cast<char*>(darr.data()), size);
                    fo_vec.back()->close();
                }
                emit emit_progress(i * 50 / num_files);
                std::cout << i * 50 / num_files << "/100" << std::endl;
                i++;
                if(stop){
                    return fo_vec;
                }
            }
        }
    }
    return fo_vec;
}

std::vector<std::shared_ptr<QDataStream>> SortFile::create_date_stream(std::vector<std::shared_ptr<QFile>>& fo_vec)
{
    std::vector<std::shared_ptr<QDataStream>> ds_out_vec;
    for (auto fo : fo_vec) {
        if (fo->open(QFile::ReadOnly)) {
            ds_out_vec.emplace_back(std::make_shared<QDataStream>(fo.get()));
            ds_out_vec.back()->setByteOrder(QDataStream::LittleEndian);
        }
    }
    return ds_out_vec;
}

bool SortFile::creat_out_file(std::vector<std::shared_ptr<QFile>>& fo_vec, const QString& save_file_name)
{
    QFile fo_all(save_file_name);
    if (fo_all.open(QFile::WriteOnly)) {
        QDataStream ds_out_all(&fo_all);
        ds_out_all.setByteOrder(QDataStream::LittleEndian);

        auto ds_out_vec = create_date_stream(fo_vec);
        std::vector<double> double_vec(ds_out_vec.size());

        int i = 0;
        for (auto ds : ds_out_vec) {
            *ds >> double_vec[i];
            i++;
        }

        auto d_min = double_vec.begin();
        int i_min;
        auto sort = [&]() {
            d_min = std::min_element(double_vec.begin(), double_vec.end());
            i_min = std::distance(double_vec.begin(), d_min);
        };

        auto sent_size = size_file / 50;
        while (sent_size % 8 != 0) {
            sent_size--;
        }

        do {
            sort();
            ds_out_all << *d_min;
            auto pos = ds_out_all.device()->pos();
            if (pos % sent_size == 0) {
                if(stop){
                    for (auto fo : fo_vec) {
                        fo->close();
                        fo->remove();
                    }
                    fo_all.close();
                    return false;
                }
                emit emit_progress(50 + ds_out_all.device()->pos() * 50 / size_file);
                std::cout << 50 + ds_out_all.device()->pos() * 50 / size_file << "/100" << std::endl;
            }
            if (!ds_out_vec[i_min]->atEnd()) {
                *ds_out_vec[i_min] >> *d_min;
            } else {
                ds_out_vec[i_min]->device()->close();
                ds_out_vec.erase(ds_out_vec.begin() + i_min);
                double_vec.erase(double_vec.begin() + i_min);
            }
        } while (double_vec.size() > 0);

        for (auto fo : fo_vec) {
            fo->remove();
        }
        QDir dir_sort;
        dir_sort.remove("dir_sort");

        fo_all.close();
        emit emit_fin();
        std::cout << "fin" << std::endl;
        return true;
    }
    emit emit_fin();
    return false;
}

bool SortFile::run(QString open_file_name, QString save_file_name)
{
    auto fo_vec = create_sort_files_buf(open_file_name);
    if (fo_vec.size() > 0) {
        return creat_out_file(fo_vec, save_file_name);
    }
    emit emit_fin();
    return false;
}

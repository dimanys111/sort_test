#ifndef SORTFILE_H
#define SORTFILE_H

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <algorithm>
#include <memory>

#include "lib_global.h"

class LIB_EXPORT SortFile : public QObject {
    Q_OBJECT
public:
    SortFile(qint64 size_memori);
    bool run(QString open_file_name, QString save_file_name);
    volatile bool stop{ false };
signals:
    void emit_progress(int pos);
    void emit_fin();

private:
    qint64 size_file;
    qint64 size_memori;
    std::vector<std::shared_ptr<QDataStream>> create_date_stream(std::vector<std::shared_ptr<QFile>>& fo_vec);
    bool creat_out_file(std::vector<std::shared_ptr<QFile>>& fo_vec, const QString& save_file_name);
    std::vector<std::shared_ptr<QFile>> create_sort_files_buf(const QString& name_file);
};

#endif // SORTFILE_H

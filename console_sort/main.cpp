#include "sortfile.h"

#include <iostream>
#include <random>

qint64 size_memori = 100 * std::pow(10, 6); // размер памяти
qint64 size_file = 10 * std::pow(10, 9); //размер выходного файла

void create_file(QString&& name_file)
{
    QFile f;
    f.setFileName(name_file);
    if (f.open(QFile::WriteOnly)) {
        int send_pb = size_file / 100;
        while (send_pb % 8 != 0) {
            send_pb--;
        }
        std::uniform_int_distribution<> dist(0, INT_MAX);
        std::mt19937 gen;
        gen.seed(time(0));

        auto count = size_file / sizeof(double); // количество значений
        QDataStream ds(&f);
        ds.setByteOrder(QDataStream::LittleEndian);
        for (decltype(count) i = 0; i < count; i++) {
            double d = dist(gen) / static_cast<double>(INT_MAX);
            ds << d;
            if (f.pos() % send_pb == 0) {
                std::cout << f.pos() / send_pb << "/100" << std::endl;
            }
        }
        f.close();
        std::cout << "fin" << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if (argc > 1) {
        std::string com(argv[1]);
        if (com == "-create_file") {
            if (argc > 2) {
                create_file(argv[2]);
            } else {
                create_file("auto.file");
            }
            return 0;
        }
        if (com == "-run_file") {
            SortFile sf(size_memori);
            if (argc > 2) {
                if (argc > 3) {
                    sf.run(argv[2], argv[3]);
                } else {
                    sf.run(argv[2], "out.file");
                }
            } else {
                sf.run("auto.file", "out.file");
            }
            return 0;
        }
    }
    std::cout << "Введите команду" << std::endl;
    return -1;
}

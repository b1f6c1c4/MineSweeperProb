#include <fstream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <map>

class ReadWriteLock
{
public:
    void read_lock();
    void read_unlock();

    void write_lock();
    void write_unlock();
private:
    int readers;
    bool writer;
    std::mutex mtx;
    std::condition_variable unlocked;
};

void ReadWriteLock::read_lock()
{
    std::unique_lock<std::mutex> lck(mtx);
    while (writer)
        unlocked.wait(lck);
    readers++;
}

void ReadWriteLock::read_unlock()
{
    std::unique_lock<std::mutex> lck(mtx);
    readers--;
    if (readers == 0)
        unlocked.notify_all();
}

void ReadWriteLock::write_lock()
{
    std::unique_lock<std::mutex> lck(mtx);
    while (writer || (readers > 0))
        unlocked.wait(lck);
    writer = true;
}

void ReadWriteLock::write_unlock()
{
    std::unique_lock<std::mutex> lck(mtx);
    writer = false;
    unlocked.notify_all();
}

std::map<std::string, size_t *> dic;

int main(int argc, char *argv[])
{
    for (auto i = 1; i < argc; ++i)
    {
        std::ifstream fin(argv[i]);
        while (!fin.eof())
        {
            size_t mns, v, c;
            std::string str;
            fin >> mns >> str >> v >> c;
            if (fin.eof())
                break;

            auto r = dic.emplace(str, nullptr);
            if (r.second)
            {
                r.first->second = new size_t[479 * 478];
                memset(r.first->second, 0, 479 * 478 * sizeof(size_t));
            }

            r.first->second[(mns - 1) * 479 + v] += c;
        }
        fin.close();
    }

    for (auto p : dic)
    {
        std::ifstream fin(p.first + ".csv");
        if (fin.good())
        {
            for (auto mns = 1; mns <= 478; ++mns)
            {
                for (auto v = 0; v <= 478; ++v)
                {
                    int c;
                    char ch;
                    if (v > 0)
                        fin >> ch;
                    fin >> c;
                    p.second[(mns - 1) * 479 + v] += c;
                }
            }
            fin.close();
        }
    }

    for (auto p : dic)
    {
        std::ofstream fout(p.first + ".csv");
        for (auto mns = 1; mns <= 478; ++mns)
        {
            for (auto v = 0; v <= 478; ++v)
            {
                if (v > 0)
                    fout << ',';
                fout << p.second[(mns - 1) * 479 + v];
            }
            fout << std::endl;
        }
        fout.close();
    }

    return 0;
}

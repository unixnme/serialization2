#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <chrono>

#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/serialization/vector.hpp"

#include <bitsery/bitsery.h>
#include <bitsery/adapter/stream.h>
#include <bitsery/brief_syntax.h>
#include <bitsery/brief_syntax/vector.h>


class Data {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & f;
        ar & i;
        ar & d;
    }

    float f;
    int i;
    double d;

public:
    explicit Data(float f = 0.0f, int i = 0, double d = 0.0)
            : f{f}, i{i}, d{d} {}

    bool operator==(const Data &that) const {
        return i == that.i && f == that.f && d == that.d;
    }

    template <typename S>
    void serialize(S& s) {
        s(f, i, d);
    }
};

std::vector<std::vector<Data>> CreateData(int n, int m) {
    std::vector<std::vector<Data>> aad;
    for (int i = 0; i < n; ++i) {
        std::vector<Data> ad;
        for (int j = 0; j < m; ++j) {
            const auto x = i*m + j;
            ad.emplace_back(log(static_cast<float>(x)), x, static_cast<double>(x));
        }
        aad.push_back(std::move(ad));
    }
    return aad;
}

template<typename T>
bool TestBoost(const T &data, const std::string &filename) {
    {
        std::ofstream ofs(filename);
        boost::archive::binary_oarchive oa{ofs};
        oa << data;
    }

    T new_data;
    {
        std::ifstream ifs(filename);
        boost::archive::binary_iarchive ia{ifs};
        ia >> new_data;
    }

    return data == new_data;
}

template<typename T>
bool TestBitsery(const T &data, const std::string &filename) {
    T new_data;
    unsigned long writtenSize;
    {
        std::ofstream ofs{filename};
        bitsery::Serializer<bitsery::OutputBufferedStreamAdapter> ser{ofs};
        ser.object(data);
        ser.adapter().flush();
    }
    {
        std::ifstream ifs{filename};
        auto state = bitsery::quickDeserialization<bitsery::InputStreamAdapter>(ifs, new_data);
    }
    return data == new_data;
}

int main() {
    const auto aad = CreateData(1000, 1000);
    const auto start = std::chrono::high_resolution_clock::now();
    assert(TestBoost(aad, "boost.bin"));
    const auto end1 = std::chrono::high_resolution_clock::now();
    const auto t1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start);
    assert(TestBitsery(aad, "bitsery.bin"));
    const auto end2 = std::chrono::high_resolution_clock::now();
    const auto t2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - end1);

    std::cout << "boost: " << t1.count() << "ms" << std::endl;
    std::cout << "bitsery: " << t2.count() << "ms" << std::endl;

    return 0;
}

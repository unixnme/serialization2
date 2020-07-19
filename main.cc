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
bool SaveBoost(const T &data, const std::string &filename) {
    std::ofstream ofs(filename);
    if (!ofs) return false;

    boost::archive::binary_oarchive oa{ofs};
    oa << data;
    return true;
}

template<typename T>
bool LoadBoost(T &data, const std::string &filename) {
    std::ifstream ifs(filename);
    if (!ifs) return false;

    boost::archive::binary_iarchive ia{ifs};
    ia >> data;

    return true;
}

template<typename T>
bool SaveBitsery(const T &data, const std::string &filename) {
    std::ofstream ofs{filename};
    if (!ofs) return false;

    bitsery::Serializer<bitsery::OutputBufferedStreamAdapter> ser{ofs};
    ser.object(data);
    ser.adapter().flush();
    return true;
}

template<typename T>
bool LoadBitsery(T &data, const std::string &filename) {
    std::ifstream ifs{filename};
    if (!ifs) return false;

    auto state = bitsery::quickDeserialization<bitsery::InputStreamAdapter>(ifs, data);
    return true;
}

int main() {
    const auto aad = CreateData(1000, 1000);
    const auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<Data>> aad_boost, aad_bitsery;
    assert(SaveBoost(aad, "boost.bin"));
    const auto end1 = std::chrono::high_resolution_clock::now();
    assert(LoadBoost(aad_boost, "boost.bin"));
    assert(aad == aad_boost);
    const auto end2 = std::chrono::high_resolution_clock::now();

    assert(SaveBitsery(aad, "bitsery.bin"));
    const auto end3 = std::chrono::high_resolution_clock::now();
    assert(LoadBitsery(aad_bitsery, "bitsery.bin"));
    assert(aad == aad_bitsery);
    const auto end4 = std::chrono::high_resolution_clock::now();

    const auto t1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start);
    const auto t2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - end1);
    const auto t3 = std::chrono::duration_cast<std::chrono::milliseconds>(end3 - end2);
    const auto t4 = std::chrono::duration_cast<std::chrono::milliseconds>(end4 - end3);

    std::cout << "boost save: " << t1.count() << "ms" << std::endl;
    std::cout << "boost load: " << t2.count() << "ms" << std::endl;
    std::cout << "bitsery save: " << t3.count() << "ms" << std::endl;
    std::cout << "bitsery load: " << t4.count() << "ms" << std::endl;

    return 0;
}

#include <iostream>
#include <fstream>
#include <cmath>

#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/serialization/vector.hpp"

#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/brief_syntax.h>
#include <bitsery/brief_syntax/vector.h>

using Buffer = std::vector<uint8_t>;
using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
using InputAdapter = bitsery::InputBufferAdapter<Buffer>;

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
    Buffer buffer;
    T new_data;
    unsigned long writtenSize;
    {
        writtenSize = bitsery::quickSerialization<OutputAdapter>(buffer, data);
    }
    {
        auto state = bitsery::quickDeserialization<InputAdapter>({buffer.begin(), writtenSize}, new_data);
    }
    return data == new_data;
}

int main() {
    const auto aad = CreateData(100, 1000);
    assert(TestBoost(aad, "boost.bin"));
    assert(TestBitsery(aad, "bitsery.bin"));

    return 0;
}

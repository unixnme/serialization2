#include <iostream>
#include <fstream>
#include <cmath>
#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/serialization/vector.hpp"

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

int main() {
    const auto filename = "data.bin";
    auto aad = CreateData(100, 1000);
    {
        std::ofstream ofs(filename);
        boost::archive::binary_oarchive oa{ofs};
        oa << aad;
    }

    std::vector<std::vector<Data>> new_aad;
    {
        std::ifstream ifs(filename);
        boost::archive::binary_iarchive ia{ifs};
        ia >> new_aad;
    }

    assert(aad == new_aad);
    return 0;
}

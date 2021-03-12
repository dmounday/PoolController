
#ifndef i2c_hpp
#define i2c_hpp

#include <string>

namespace GPIO {

class I2C {
public:
    explicit I2C(const std::string& interface, unsigned int device_id);
    I2C()= delete;
    //I2C() {}
    ~I2C()
    {
        close();
    }

    void open(const std::string& interface, unsigned int device_id);
    void close();

    int read(uint8_t* data, const int size);
    int write(const uint8_t* data, const int size);
    int regwrite8(uint8_t reg, int value) const;
    int regwrite16(uint8_t reg, int value) const;
    int regread8(uint8_t reg) const;
    int regread16(uint8_t reg) const;
private:
    int m_fd = -1;
};

}

#endif /* i2c_hpp */

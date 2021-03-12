//
//  i2c.cpp


#include <cinttypes>
#include <cstring>
#include <cerrno>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <linux/i2c-dev.h>
extern "C" {
#include <i2c/smbus.h>
}

#include <cppgpio/i2c.hpp>


using namespace GPIO;

class I2CError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

I2C::I2C(const std::string& interface, unsigned int device_id)
{
    open(interface, device_id);
}
int I2C::read(uint8_t* data, const int size){
	return ::read(m_fd, data, size);
}

int I2C::write(const uint8_t* data, const int size){
	return ::write(m_fd, data, size);
}

int I2C::regread8(uint8_t reg) const
{
	return i2c_smbus_read_byte_data(m_fd, (__u8)reg);

}

int I2C::regread16(uint8_t reg) const
{
	return i2c_smbus_read_word_data(m_fd, (__u8)reg);
}
int I2C::regwrite8(uint8_t reg, int value) const
{
	return i2c_smbus_write_byte_data(m_fd, (__u8)reg, (__u8) value);

}

int I2C::regwrite16(uint8_t reg, int value) const
{
	return i2c_smbus_write_word_data(m_fd, reg, (__u16) value);
}
void I2C::open(const std::string& interface, unsigned int device_id)
{
    if ((m_fd = ::open(interface.c_str(), O_RDWR)) < 0) {}
    if (::ioctl(m_fd, I2C_SLAVE, device_id)) throw I2CError(interface + ": cannot open i2c/smbus: " + std::strerror(errno));
}

void I2C::close()
{
    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
}


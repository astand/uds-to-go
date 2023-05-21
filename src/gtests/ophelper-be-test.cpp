#include <gtest/gtest.h>
#include <stdint.h>

#define SYS_MEMORY_BE

#include <etc/helpers/ophelper.h>

// Demonstrate some basic assertions.
TEST(BigEndianessTest, GeneralTests) {

  EXPECT_EQ(ophelper::from_be_u16(0xcdab), 0xcdab);
  EXPECT_EQ(ophelper::to_be_u16(0xcdab), 0xcdab);
  EXPECT_EQ(ophelper::from_le_u16(0xcdab), 0xabcd);
  EXPECT_EQ(ophelper::to_le_u16(0xcdab), 0xabcd);

  EXPECT_EQ(ophelper::from_be_u32(0x11223344), 0x11223344);
  EXPECT_EQ(ophelper::to_be_u32(0x11223344), 0x11223344);
  EXPECT_EQ(ophelper::from_le_u32(0x11223344), 0x44332211);
  EXPECT_EQ(ophelper::to_le_u32(0x11223344), 0x44332211);
}

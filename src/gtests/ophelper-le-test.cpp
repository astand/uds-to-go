#include <gtest/gtest.h>
#include <stdint.h>

// run tests only if the GeneralTests body changes
// disable BE for ophelper-be-test at the same time
// after tests successfully done, disable LE back again
#define ONLY_LITTLE_ENDIAN 0

#if ONLY_LITTLE_ENDIAN
#define SYS_MEMORY_LE
#endif

#include <etc/helpers/ophelper.h>

// Demonstrate some basic assertions.
TEST(LittleEndianessTest, GeneralTests) {

#if ONLY_LITTLE_ENDIAN
  EXPECT_EQ(ophelper::from_le_u16(0xcdab), 0xcdab);
  EXPECT_EQ(ophelper::to_le_u16(0xcdab), 0xcdab);
  EXPECT_EQ(ophelper::from_be_u16(0xcdab), 0xabcd);
  EXPECT_EQ(ophelper::to_be_u16(0xcdab), 0xabcd);

  EXPECT_EQ(ophelper::from_le_u32(0x11223344), 0x11223344);
  EXPECT_EQ(ophelper::to_le_u32(0x11223344), 0x11223344);
  EXPECT_EQ(ophelper::from_be_u32(0x11223344), 0x44332211);
  EXPECT_EQ(ophelper::to_be_u32(0x11223344), 0x44332211);
#endif
}

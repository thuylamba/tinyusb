#include <inttypes.h>
#include <stdlib.h>
#include "bsp/board.h"
#include "tusb.h"
#include "printf.h"
#include "sdcard.h"
#include "sec_disk.h"

uint8_t 
security_init(void) {
  return 0;
}
uint8_t 
security_p_key_read(Security_t *uData, uint8_t *key, uint32_t *size) {
  return 0;
}
uint8_t 
security_p_key_write(Security_t *uData, uint8_t *key, uint32_t size) {
  return 0;
}
uint8_t 
security_m_key_read(Security_t *uData, uint8_t *key, uint32_t *size) {
  return 0;
}
uint8_t 
security_m_key_write(Security_t *uData, uint8_t *key, uint32_t size) {
  return 0;
}
uint8_t 
security_e_key_read(Security_t *uData, uint8_t *key, uint32_t *size) {
  return 0;
}
uint8_t 
security_e_key_write(Security_t *uData, uint8_t *key, uint32_t size) {
	return 0;
}

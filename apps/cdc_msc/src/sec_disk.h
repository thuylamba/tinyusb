#ifndef __SEC_DISK_H__
#define __SEC_DISK_H__
#define SECURITY_AUTHEN_P_KEY_OFFSET	0
#define SECURITY_AUTHEN_P_KEY_SIZE	32
#define SECURITY_AUTHEN_M_KEY_OFFSET	(SECURITY_AUTHEN_P_KEY_OFFSET + SECURITY_AUTHEN_P_KEY_SIZE)
#define SECURITY_AUTHEN_M_KEY_SIZE	32
#define SECURITY_AUTHEN_E_KEY_OFFSET	(SECURITY_AUTHEN_M_KEY_OFFSET + SECURITY_AUTHEN_M_KEY_SIZE)
#define SECURITY_AUTHEN_E_KEY_SIZE	32

typedef struct 
{
	uint32_t SPIBase;	// offset of spi
	uint8_t sec_passwd_key[SECURITY_AUTHEN_P_KEY_SIZE];	// key password
	uint8_t sec_master_key[SECURITY_AUTHEN_M_KEY_SIZE];	// key master
	uint8_t sec_encrpt_key[SECURITY_AUTHEN_E_KEY_SIZE];	// key encrypt
} Security_t;

uint8_t security_init(void);
uint8_t security_p_key_read(Security_t *uData, uint8_t *key, uint32_t *size);
uint8_t security_p_key_write(Security_t *uData, uint8_t *key, uint32_t size);
uint8_t security_m_key_read(Security_t *uData, uint8_t *key, uint32_t *size);
uint8_t security_m_key_write(Security_t *uData, uint8_t *key, uint32_t size);
uint8_t security_e_key_read(Security_t *uData, uint8_t *key, uint32_t *size);
uint8_t security_e_key_write(Security_t *uData, uint8_t *key, uint32_t size);
#endif	//	__SEC_DISK_H__

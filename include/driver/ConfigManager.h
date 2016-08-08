/*
 * ConfigManager.h
 *
 *  Created on: Dec 30, 2015
 *      Author: tsnow
 */

#ifndef INCLUDE_DRIVER_CONFIGMANAGER_H_
#define INCLUDE_DRIVER_CONFIGMANAGER_H_

extern "C" {
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <spi_flash.h>
}
#include <driver/utils.h>

#define ESP_PARAM_START_SEC   0x3C

#define ESP_PARAM_SAVE_0    1
#define ESP_PARAM_SAVE_1    2


template<class T>
class ConfigManager : public T
{
private:
	uint32_t m_cnt;
	bool m_first_sector;
	bool m_saved;
	uint32_t cksum;

public:
	ConfigManager() : T()
	{
		m_saved = false;
		LoadData();
	}

	void SetDefaultData(T *def)
		{
			T *p = (T *)this;
			*p = *def;
			SaveData();
		}


	bool IsSaved() { return m_saved; };

	void LoadData()
	{
		uint32_t cnt0;
		uint32_t cnt1;

		 spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0) * SPI_FLASH_SEC_SIZE,
		                   (uint32 *)this, sizeof(*this));

		 cnt0 = m_cnt;

		 spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1) * SPI_FLASH_SEC_SIZE,
				 	 	   (uint32 *)this, sizeof(*this));

		 cnt1 = m_cnt;
		 if(cnt0 == 0xffffffff && cnt1 == 0xffffffff)
		 {
			 m_saved = false;
			 m_first_sector = false;
			 m_cnt = 0;
			 memset(this,0,sizeof(*this));
		 }
		 else if(cnt0 > cnt1 || cnt1 == 0xffffffff) // reread 0 if last saved
		 {
			 spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0) * SPI_FLASH_SEC_SIZE,
			                   (uint32 *)this, sizeof(*this));
			 m_first_sector = true;
		 }
		 else
			 m_first_sector = false;
		 if(m_saved)
			 {
				 T *p = (T *)this;
				 uint32_t sum = GetCheckSum(p,sizeof(T));
				 if(sum != cksum)	// corruption
					 {
						 os_memset(p,0,sizeof(T));
						 m_saved = false;
					 }
			 }
	}

	void SaveData()
	{
		m_cnt++;
		m_saved = true;
		T *p = (T *)this;
		cksum = GetCheckSum(p,sizeof(T));
		if(m_first_sector)	// got from 0?  Write to 1
		{
	        spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1);
	        spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1) * SPI_FLASH_SEC_SIZE,
	                        (uint32 *)this, sizeof(*this));
		}
		else
		{
	        spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0);
	        spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0) * SPI_FLASH_SEC_SIZE,
	                        (uint32 *)this, sizeof(*this));
		}

		m_first_sector = m_first_sector ? false : true;
	}

};


#endif /* INCLUDE_DRIVER_CONFIGMANAGER_H_ */

#pragma once
/**
 * @file AESWrapper.hpp
 * @author Course Lecturers
 * @brief 
 * @version 0.1
 * @date 2025-02-23
 * 
 */

#include <string>


class AESWrapper
{
public:
	static const unsigned int DEFAULT_KEYLENGTH = 16;
private:
	unsigned char _key[DEFAULT_KEYLENGTH];
	AESWrapper(const AESWrapper& aes);
public:
	static unsigned char* GenerateKey(unsigned char* buffer, unsigned int length);

	AESWrapper();
	AESWrapper(const unsigned char* key, unsigned int size);
	~AESWrapper();

	const unsigned char* getKey() const;

	std::string encrypt(const char* plain, unsigned int length);
	std::string encrypt(std::string plain);
	std::string decrypt(const char* cipher, unsigned int length);
};
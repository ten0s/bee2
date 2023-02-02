/*
*******************************************************************************
\file hex.h
\brief Hexadecimal strings
\project bee2 [cryptographic library]
\created 2015.10.29
\version 2023.02.02
\license This program is released under the GNU General Public License 
version 3. See Copyright Notices in bee2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file hex.h
\brief Шестнадцатеричные строки
*******************************************************************************
*/

#ifndef __BEE2_HEX_H
#define __BEE2_HEX_H

#include "bee2/defs.h"
#include "bee2/core/safe.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
*******************************************************************************
\file hex.h

Реализовано управление шестнадцатеричными строками, которые представляют 
буферы памяти. Формально шестнадцатеричный формат задан в RFC 4648 как Base16.

Шестнадцатеричная строка -- это слово в алфавите 
	{'0',..,'9','A',...,'F','a',...,'f'}. 
Символы 'A' и 'a', 'B' и 'b',... считаются эквивалентными. 
В соответствии с RFC 4648 преимущество отдается прописным символам.

\pre Во все функции, кроме возможно hexIsValid(), передаются корректные 
шестнадцатеричные строки и буферы памяти.
*******************************************************************************
*/

/*!	\brief Корректная шестнадцатеричная строка?

	Проверяется корректность шестнадцатеричной строки hex. Строка считается
	корректной, если она состоит из четного числа символов алфавита
	{'0',..,'9','A',...,'F','a',...,'f'}.
	\return Признак корректности.
	\safe Функция нерегулярна.
*/
bool_t hexIsValid(
	const char* hex		/*!< [in] шестнадцатеричная строка */
);

/*!	\brief К верхнему регистру

	Строчные символы строки hex преобразуются в прописные.
*/
void hexUpper(
	char* hex			/*!< [in,out] шестнадцатеричная строка */
);

/*!	\brief К нижнему регистру

	Прописные символы строки hex преобразуются в строчные.
*/
void hexLower(
	char* hex			/*!< [in,out] шестнадцатеричная строка */
);

/*!	\brief Совпадает с шестнадцатеричной строкой?

	Буфер [strLen(hex) / 2]buf сравнивается с буфером, заданым 
	шестнадцатеричной строкой hex. Первая пара символов hex преобразуется 
	в октет, который сравнивается с первым октетом buf, вторая пара 
	сравнивается со вторым октетом buf и т. д.
	\pre hexIsValid(hex) == TRUE.
	\return Признак совпадения.
	\remark Сравнение задается следующим псевдокодом:
	\code
		octet tmp[strLen(hex) / 2];
		hexTo(tmp, hex);
		return memEq(buf, tmp, strLen(hex) / 2);
	\endcode
	\safe Имеется ускоренная нерегулярная редакция.
*/
bool_t hexEq(
	const void* buf,	/*!< [in] буфер */
	const char* hex		/*!< [in] шестнадцатеричная строка */
);

bool_t SAFE(hexEq)(const void* buf, const char* hex);
bool_t FAST(hexEq)(const void* buf, const char* hex);

/*!	\brief Совпадает с обратной шестнадцатеричной строкой?

	Буфер [strLen(hex) / 2]buf сравнивается с буфером, заданым 
	шестнадцатеричной строкой hex. Первая пара символов hex преобразуется 
	в октет, который сравнивается с последним октетом buf, вторая пара 
	сравнивается с предпоследним октетом buf и т.д.
	\pre hexIsValid(hex) == TRUE.
	\return Признак совпадения.
	\remark Сравнение задается следующим псевдокодом:
	\code
		octet tmp[strLen(hex) / 2];
		hexTo(tmp, hex);
		memRev(tmp, strLen(hex) / 2);
		return memEq(buf, tmp, strLen(hex) / 2);
	\endcode
	\safe Имеется ускоренная нерегулярная редакция.
*/
bool_t hexEqRev(
	const void* buf,	/*!< [in] буфер */
	const char* hex		/*!< [in] шестнадцатеричная строка */
);

bool_t SAFE(hexEqRev)(const void* buf, const char* hex);
bool_t FAST(hexEqRev)(const void* buf, const char* hex);

/*!	\brief Кодирование буфера памяти

	Буфер [count]src кодируется шестнадцатеричной строкой
	{2 * count + 1}dest. Первому октету src соответствует первая пара 
	символов dest, второму октету -- вторая пара и т.д.
	\pre Буферы dest и src не пересекаются.
*/
void hexFrom(
	char* dest,			/*!< [out] строка-приемник */
	const void* src,	/*!< [in] память-источник */
	size_t count		/*!< [in] число октетов */
);

/*!	\brief Обратное кодирование буфера памяти

	Буфер [count]src кодируется шестнадцатеричной строкой 
	[2 * count + 1]dest. Первому октету src соответствует последняя пара 
	символов dest, второму октету -- предпоследняя пара и т.д.
	\pre Буферы dest и src не пересекаются.
*/
void hexFromRev(
	char* dest,			/*!< [out] строка-приемник */
	const void* src,	/*!< [in] память-источник */
	size_t count		/*!< [in] число октетов */
);

/*!	\brief Декодирование буфера памяти

	Шестнадцатеричная строка src преобразуется в строку октетов 
	[strLen(src) / 2]dest. По первой паре символов src определяется первый 
	октет dest, по второй паре -- второй октет и т.д.
	\pre hexIsValid(hex) == TRUE.
*/
void hexTo(
	void* dest,			/*!< [out] память-приемник */
	const char* src		/*!< [in] строка-источник */
);

/*!	\brief Обратное декодирование буфера памяти

	Шестнадцатеричная строка src преобразуется в строку октетов 
	[strLen(src) / 2]dest. По последней паре символов src определяется первый 
	октет dest, по предпоследней паре -- второй октет и т. д.
	\pre hexIsValid(hex) == TRUE.
*/
void hexToRev(
	void* dest,			/*!< [out] память-приемник */
	const char* src		/*!< [in] строка-источник */
);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __BEE2_HEX_H */

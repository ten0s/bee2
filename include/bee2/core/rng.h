/*
*******************************************************************************
\file rng.h
\brief Entropy sources and random number generators
\project bee2 [cryptographic library]
\author Sergey Agievich [agievich@{bsu.by|gmail.com}]
\created 2014.10.13
\version 2021.05.18
\license This program is released under the GNU General Public License 
version 3. See Copyright Notices in bee2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file rng.h
\brief Источники случайности и генераторы случайных чисел
*******************************************************************************
*/


#ifndef __BEE2_RNG_H
#define __BEE2_RNG_H

#include "bee2/defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
*******************************************************************************
\file rng.h

\section rng-source Источники случайности

В системе может быть несколько источников случайности. Получение данных от
источников реализуется через интерфейс read_i. Внутренние источники 
поддерживаются функцией rngReadSource().

При обработке запроса на чтение данных источники обрабатывает свои наблюдения, 
учитывая оценки энтропии. При необходимости наблюдения сжимаются. В результате 
обработки запроса будут возвращены высокоэнтропийные данные. Их объем может 
быть меньше запрошенного.

При обработке запроса может произойти сбой источника или время снятия 
наблюдений может оказаться неприемлемо большим. В таких случаях также будет 
возвращено меньше данных, чем запрашивалось.
*******************************************************************************
*/

/*!	\brief Получение данных от источников случайности
	
	Данные от источника случайности с именем source_name записываются в буфер 
	[count]buf. По адресу read возвращается число полученных октетов. 
	Поддерживаются следующие имена источников:
	-	"trng": физический генератор случайных чисел;
	-	"trng2": дополнительный физический генератор случайных чисел;
	-	"timer": высокоточный таймер. Наблюдениями являются разности между
		показаниями таймера до и после передачи управления ядру операционной
		системы;
	-	"sys": источник операционной системы.
	.
	\pre Буфер buf корректен.
	\pre Указатель read корректен.
	\return ERR_OK, если получено определенное число октетов (возможно 
	меньшее count и возможно нулевое) и источник сохранил работоспособность,
	ERR_MAX, если получено меньше чем count октетов и источник отказал, или 
	другой код ошибки.
	\warning Если при работе с источниками "trng", "trng2" длина выходного
	буфера меньше длины машинного слова, то функция возвратит ERR_OK, но ни
	одного октета данных сгенерировано не будет.
	\remark Ошибкой не считается ситуация, когда сгенерировано меньше чем count 
	октетов. Данная ситуация может быть связана с ожиданием накопления наблюдений.
	\remark Передавая count == 0, можно проверить наличие источника.
	\remark Поддержан интерфес read_i.
*/
err_t rngReadSource(
	void* buf,				/*!< [out] прочитанные данные */
	size_t* read,			/*!< [out] число прочитанных октетов */
	size_t count,			/*!< [in] длина buf в октетах */
	const char* source_name	/*!< [in/out] имя источника */
);

/*!
*******************************************************************************
\file rng.h

\section rng-stat Статистическое тестирование

Реализованы статистические тесты стандарта FIPS 140-2. В этих тестах 
обрабатывает последовательность двоичных символов длины 20000 (2500 байтов).

Уровень значимости тестов p = 0.01. С такой вероятностью истинно случайная 
последовательность не пройдет тест. Известно, что тесты FIPS слабо зависимы.
Поэтому истинно случайная последовательность не пройдет батарею из n тестов
с вероятностью 1 - (1 - p)^n \approx n p.

\safe Обрабатываемые в тестах данные не считаются секретными. Эти данные 
не должны использовать для построения ключей.
*******************************************************************************
*/

/*!	\brief Тест знаков FIPS 140

	К последовательности buf применяется тест знаков FIPS.
	\pre Буфер buf корректен.
	\return Признак успешного прохождения теста.
	\remark Определяется величина S -- число единиц в последовательности.
	Тест пройден, если 9725 < S < 10275.
*/
bool_t rngTestFIPS1(
	const octet buf[2500]	/*!< [in] тестируемая последовательность */
);

/*!	\brief Покер-тест FIPS 140

	К последовательности buf применяется покер-тест FIPS.
	\pre Буфер buf корректен.
	\return Признак успешного прохождения теста.
	\remark Последовательность разбивается на 5000 тетрад. Тетрады 
	интерпретируются как числа от 0 до 15. Определяется статистика 
	S = 16 \sum _{i=0}^15 S_i^2 - (5000)^2, где S_i -- количество появлений 
	числа i среди тетрад. Тест пройден, если 10800 < S < 230850.
*/
bool_t rngTestFIPS2(
	const octet buf[2500]	/*!< [in] тестируемая последовательность */
);

/*!	\brief Тест серий FIPS 140

	К последовательности buf применяется тест серий FIPS.
	\pre Буфер buf корректен.
	\return Признак успешного прохождения теста.
	\remark Определяются серии (максимальные последовательности 
	повторяющихся соседних битов) различных длин. 
	Тест пройден, если и для серий из нулей, и для серий из единиц 
	выполняется: S_1 \in [2315, 2685], S_2 \in [1114, 1386], 
	S_3 \in [527, 723], S_4 \in [240, 384], S_5, S_6+ \in [103, 209].
	Здесь S_i -- количество серий длины i = 1, 2, ..., S_6+ = S_6 + S_7 + .... 
	\remark Бит с номером i: wwTestBit((const word*)buf, i). 
*/
bool_t rngTestFIPS3(
	const octet buf[2500]	/*!< [in] тестируемая последовательность */
);

/*!	\brief Тест длинных серий FIPS 140

	К последовательности buf применяется тест серий FIPS.
	\pre Буфер buf корректен.
	\return Признак успешного прохождения теста.
	\remark Тест пройден, если в последовательности отсутствуют серии 
	длины 26 и больше.
	\remark Бит с номером i: wwTestBit((const word*)buf, i). 
*/
bool_t rngTestFIPS4(
	const octet buf[2500]	/*!< [in] тестируемая последовательность */
);

/*!
*******************************************************************************
\file rng.h

\section rng-rng Генератор случайных чисел

Выходные случайные числа можно использовать для построения ключей и других 
критических объектов.

Генератор является единственным в библиотеке. 

Генератор можно использовать в многопоточных приложениях. 

При создании генератора опрашиваются все доступные источники случайности. 
Данные от источников объединяются и хэшируются с помощью механизма
beltHash (см. crypto/belt.h). Хэш-значение используется как ключ механизма 
brngCTR (см. crypto/brng.h). 

При функционировании генератора выполняются последовательные обращения 
к brngCTR. При подготовке обращений могут использоваться данные 
от доступных источников случайности. Эти данные используются в качестве 
входного буфера функции brngCTRStepR().

Данные от источников используются в функции rngStepR() и не используются 
в функции rngStepR2(). Первую функцию можно применять время от времени 
(например, во время согласования общего ключа перед передачей данных), 
вторую -- регулярно (в процессе передачи данных). 
*******************************************************************************
*/

/*!	\brief Создание генератора

	Создается генератор случайных чисел. При создании используются 
	источники, поддерживаемые функцией rngReadSource(), а также возможно
	дополнительный источник source с состоянием source_state.
	\expect{ERR_BAD_ENTROPY} В совокупности все работоспособные источники 
	выдают не менее 32 октетов случайных данных.
	\expect Источники случайности выдают высокоэнтропийные данные.
	\return Признак успеха.
	\remark При нулевом source дополнительный источник не используется.
*/
err_t rngCreate(
	read_i source,			/*!< [in] дополнительный источник */
	void* source_state		/*!< [in] состояние дополнительного источника */
);

/*!	\brief Корректный генератор?

	Проверяется корректность генератора случайных чисел.
	\return Признак корректности.
*/
bool_t rngIsValid();

/*!	\brief Генерация случайных чисел

	В буфер [count]buf записываются случайные октеты, построенные с помощью 
	генератора случайных чисел. При формировании выходных октетов используются 
	данные от источников случайности. 
	\expect rngСreate() < rngStepR()*.
	\pre Генератор корректен.
	\remark Поддержан интерфейс gen_i (defs.h).
	\remark Состояние state не используется. Оно передается в функцию только 
	для того, чтобы поддержать интерфейс gen_i.
	\remark Запрашивается count октетов от источников. Будет возвращено 
	меньше октетов только если все источники отказали.
*/
void rngStepR(
	void* buf,				/*!< [out] буфер */
	size_t count,			/*!< [in] размер буфера (в октетах) */
	void* state				/*!< [in/out] состояние (игнорируется) */
);

/*!	\brief Облегченная генерация случайных чисел

	В буфер [count]buf записываются случайные октеты, построенные с помощью 
	генератора случайных чисел. При формировании выходных октетов данные от 
	источников случайности не используются.
	\expect rngСreate() < rngStepR2()*.
	\pre Генератор корректен.
	\remark Поддержан интерфейс gen_i (defs.h).
	\remark Состояние state не используется. Оно передается в функцию только 
	для того, чтобы поддержать интерфейс gen_i.
*/
void rngStepR2(
	void* buf,				/*!< [out] буфер */
	size_t count,			/*!< [in] размер буфера (в октетах) */
	void* state				/*!< [in/out] состояние (игнорируется) */
);

/*!	\brief Закрытие генератора

	Генератор случайных чисел закрывается.
	\pre Генератор корректен.
*/
void rngClose();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __BEE2_RNG_H */
